# In the following code, pc means program counter 

require 'ostruct'
require 'set'
require 'erb'

# Check whether the given program counter is listed in debug_info
def sc_sc_has_instr(file_hash, pc)
	file_hash.each do |filename, debug_line_arr|
		debug_line_arr.each do |line_info|
			if Integer(line_info.pc) == Integer('0x' + pc)
				final_tuple = OpenStruct.new
				final_tuple.pc = pc
				final_tuple.filename = line_info.filename
				final_tuple.row = line_info.row
				final_tuple.gray = line_info.gray
				return true, final_tuple
			end	
		end
	end
	# Not found
	final_tuple = OpenStruct.new
	final_tuple.pc = pc
	final_tuple.filename = nil
	final_tuple.row = nil
	final_tuple.gray = nil
	return false, final_tuple 
end

# Check whether a given row in the source file has corresponding assembly
def sc_has_instr(instr_hash, func_set, program_cnter)
	instr_hash.each do |pc, ass_code|
		if !func_set.include?(pc) && Integer('0x' + pc) == Integer(program_cnter)
			return true, pc, ass_code
		end
	end
	return false, nil, nil
end

# Check whether a given assembly instruction has been added to final_hash
def sc_added_final_hash(final_hash, program_cnter)
	final_hash.each do |filename, final_tuple_arr|
		final_tuple_arr.each do |final_tuple|
			if final_tuple.pc != nil && Integer('0x' + final_tuple.pc) == Integer('0x' + program_cnter)
				return true
			end
		end
	end
	return false
end

# Source centric
def xrefsc(exe_name)
	
	# An array to store .debug_line info
	debug_line_arr = Array.new
	# Regex to match valid source file name
	file_re = /(\/.*[\w\-. ]+\.[c|h])/i
	# Regex to capture pc and line number
	line_re = /(^0x.*)(\[\s*)(\d*)(.*,\s*)(\d*)(\s*\])/i
	# Regex to capture pc and assembly instructions
	instr_re = /(^.*)(:\s+)([a-fA-Z0-9]+\s)+(\s+)([a-z]+.*)/i
	# To match the line containing '<.*>' in the output of objdump
	angle_re = /(^\s*)([0-9a-z]+)(\s*)(<)(.+)(>)/i 

	# Parse the output of dwarfdump
	filename = ''
	is_fun = false
	file_hash = Hash.new
	func_set = Set.new
	IO.popen(['dwarfdump' , exe_name]){
		|io| io.each {|line|
		# Find all functions
		if line.downcase.include? 'dw_tag_subprogram'
			is_fun = true
		end
		if is_fun
			if line.downcase.include? 'dw_at_name'
				if func_match = line.match(/(")(.+)(")/i)
					lp, func_name, rp = func_match.captures
					func_set.add(func_name.strip)
				end
				is_fun = false
			end
		end

		# Match line info in .debug_line table
		if line_match = line.match(line_re)
			# A struct to store cp, line number and filename
			line_info = OpenStruct.new
			line_info.pc, lb, line_info.row, comma, line_info.col, rb = line_match.captures
			# A new source file
			if file_match = line.match(file_re)
				filename = file_match.captures
				filename = filename.at(0).strip
				if !file_hash.has_key?(filename)
					file_hash[filename] = Array.new
				end
				line_cnt = 1
			end
			line_info.filename = filename
			# Trim leading and tailing whitespace
			line_info.pc = line_info.pc.strip
			line_info.row = line_info.row.strip
			line_info.col = line_info.col.strip
			# Add line info to an array
			file_hash[filename].push(line_info)
		end
		}
	}

	# Traverse the output of dwarfdump to decide which line should be printed in gray color
	file_hash.each do |filename, debug_line_arr|         
		for i in 0..debug_line_arr.size - 1
			is_found = false
            for j in 0..i - 1
				if debug_line_arr.at(j).row == debug_line_arr.at(i).row
					is_found = true
					break
				end
			end
			if is_found
				debug_line_arr.at(i).gray = true
			else
				debug_line_arr.at(i).gray = false
			end
		end
	end

	# Sort each file according to row number, for inserting source code without assembly
	file_hash.each do |filename, debug_line_arr|
		file_hash[filename] = file_hash[filename].sort_by{|line_info| Integer(line_info.row)}
	end

	# Insert source code without assembly
	file_hash.each do |filename, debug_line_arr|
		file = File.open(filename, 'r')
		total_src_line = file.readlines.size
		total_debug_line = debug_line_arr.size
		src_line_cnt = 1
		# Insert source code
		for i in 0..total_debug_line - 1
			while src_line_cnt < Integer(file_hash[filename].at(i).row)
				single_info = OpenStruct.new
				single_info.pc = '0xffffffff'
				single_info.row = src_line_cnt.to_s
				single_info.col = '0'
				single_info.filename = filename
				single_info.gray = false
				file_hash[filename].push(single_info)
				src_line_cnt += 1
			end
			if src_line_cnt == Integer(file_hash[filename].at(i).row)
				src_line_cnt += 1
			end
		end
		# Insert the remaining source code
		while src_line_cnt <= total_src_line
			single_info = OpenStruct.new
			single_info.pc = '0xffffffff'
			single_info.row = src_line_cnt.to_s
			single_info.col = '0'
			single_info.filename = filename
			single_info.gray = false
			file_hash[filename].push(single_info)
			src_line_cnt += 1
		end	
		file_hash[filename] = file_hash[filename].sort_by{|line_info| Integer(line_info.row)}
	end

	# Traverse the output of dwarfdump to decide which instruction is generated for multiply source lines
	pc_set = Set.new
	file_hash.reverse_each do |filename, debug_line_arr|
		debug_line_arr.reverse_each do |line_info|
			# Only the final source line will have assembly instructions
			if pc_set.include? (line_info.pc)
				line_info.pc = '0xffffffff'
			else
				pc_set.add(line_info.pc)
			end
		end
	end
	
	# Parse the output of objdump
	instr_hash = Hash.new
	text_sec = false
	func_part = false
	IO.popen(['objdump', '-d', exe_name]){
		|io| io.each {|line|
			# Check if it is a function part
			if name_match = line.match(angle_re)
				space1, num, space2, lp, name, rp = name_match.captures
				name = name.strip
				if func_set.include? (name)
					func_part = true
				else
					func_part = false
				end
			end

			# In a function part
			if func_part
				# Match an instruction, use a struct to store instruction location and assembly code
				if instr_match = line.match(instr_re)
					program_cnter, colon, macode, space, ass_code = instr_match.captures
					#Trim leading and tailing whitespace
					program_cnter = program_cnter.strip
					ass_code = ass_code.strip
					# Add an instruction to the hashmap. Key is pc, value is instruction code
					instr_hash[program_cnter] = ass_code
				# Match other info excluding an empty line
				else
					if instr_match = line.match(angle_re)
						space1, num, space2, lp, name, rp = instr_match.captures
						name = name.strip
						# Add to the hashmap
						instr_hash[name] = name
					end
				end
			end
		}
	}

	# Read in all source files, and store them in a hashmap
	# Key is file name, and value is an array, each element is a line.
	file_content_hash = Hash.new
	file_hash.each do |filename, _|
		# Read in each source file
		source_content_arr = Array.new
		File.open(filename, 'r') do |file|
			# Read in each line
			file.each_line do |line|
				# Change to HTML escape
				line = CGI::escapeHTML(line)
				# Replace whitespace with '&nbsp;'
				line_length = line.size
				space_num = 0
				tab_num = 0
				idx = 0
				for i in 0..line_length - 1
					if line[i] == ' '
						space_num += 1
					elsif line[i] == '\t'
						tab_num += 1
					else
						idx = i
						break
					end
				end
				# Truncate non-whitespace part
				line = line[idx, line_length]
				# Add leading whitespace in HTML format
				for i in 1..2 * space_num + 4 * tab_num
					line = '&nbsp;' + line
				end
				source_content_arr.push(line)
			end
			file.close
		end
		file_content_hash[filename] = source_content_arr	
	end

	# The final output is a hashmap. Key is filename, value is index array. 
	# Its elements are tuples: (pc, instruction, file, line_num, flag)
	final_hash = Hash.new
	instr_hash_size = instr_hash.length
	file_hash.each do |filename, debug_line_arr|
		final_hash[filename] = Array.new
		debug_line_arr.each do |line_info|
			puts line_info.pc + ' ' + line_info.row
			# The source has no corresponding assembly instructions
			if line_info.pc == '0xffffffff'
				final_tuple = OpenStruct.new
				final_tuple.pc = nil
				final_tuple.instr = nil
				final_tuple.filename = line_info.filename
				final_tuple.row = line_info.row
				final_tuple.gray = false
				final_hash[filename].push(final_tuple)
			# The source has corresponding assembly instructions
			else
				# Double check. The pc on lhs and rhs have the same value, but different format
				sc_has_instr_res, pc, ass_code = sc_has_instr(instr_hash, func_set, line_info.pc)
				# Has assembly
				if sc_has_instr_res
					final_tuple = OpenStruct.new
					final_tuple.pc = pc
					final_tuple.instr = ass_code
					final_tuple.filename = line_info.filename
					final_tuple.row = line_info.row
					final_tuple.gray = line_info.gray
					final_hash[filename].push(final_tuple)
					# Add all following assembly which has no corresponding source code
					cur_idx = instr_hash.find_index{|pc, ass_code| !func_set.include?(pc) && Integer('0x' + pc) == Integer(line_info.pc)}
					next_idx = cur_idx + 1
					while next_idx < instr_hash_size
						# Meet a function name
						if func_set.include? (instr_hash.keys[next_idx])
							break
						# Meet a program counter
						else
							sc_sc_has_instr_res, _ = sc_sc_has_instr(file_hash, instr_hash.keys[next_idx])
							if sc_sc_has_instr_res
								break
							else
								if !sc_added_final_hash(final_hash, instr_hash.keys[next_idx])
									final_tuple = OpenStruct.new
									final_tuple.pc = instr_hash.keys[next_idx]
									final_tuple.instr = instr_hash.values[next_idx]
									final_tuple.filename = nil
									final_tuple.row = nil
									final_tuple.gray = nil
									final_hash[filename].push(final_tuple)
									next_idx += 1
								else
									next_idx += 1
								end
							end
						end
					end
				# Has no assembly, raise an exception
				else
					raise 'Source line ' + line_info.row + ' has no corresponding assembly code!'  
				end
			end
		end
	end

	# Add source file line content to final_tuple
	final_hash.each do |filename, final_tuple_arr|
		final_tuple_arr.each do |final_tuple|
			if final_tuple.filename != nil
				final_tuple.src = file_content_hash[final_tuple.filename].at(Integer(final_tuple.row) - 1)
			else
				final_tuple.src = nil
			end
		end
	end

	# Add subroutine jump link
=begin
	jump_re = /(^callq|jmp|jo|jno|js|jns|je|jz|jne|jnz|jb|jnae|jc|jnb|jae|jnc|jbe|jna|ja|jnbe|jl|jnge|jge|jnl|jle|jng|jg|jnle|jp|jpe|jnp|jpo|jcxz|jecxz)(\s+)([0-9a-z]+)(\s*)/i
	final_arr.each do |final_tuple|
		if final_tuple.instr != nil
			# A subroutine call
			if call_match = final_tuple.instr.match(jump_re)
				callq, space1, dest, space2 = call_match.captures
				dest = dest.strip
				final_tuple.target = dest
			else
				final_tuple.target = nil
			end
		else
			final_tuple.target = nil
		end
	end
=end
		
	# Output all HTML files, one per source file
	template = File.read('./template_sc.html.erb')
	final_hash.each do |filename, final_tuple_arr|
		html_filename = filename.gsub(/\.cpp|\.c|\.h/i, '.html')
		result = ERB.new(template).result(binding)
		File.open(html_filename, 'w+') do |html_file|
			html_file.write result
			html_file.close
		end	
	end

end

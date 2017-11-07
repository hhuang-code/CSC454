# --
require 'ostruct'
require 'set'
require 'erb'

def has_pc(file_hash, pc)
	file_hash.each do |key, value|
		value.each do |line_info|
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
	final_tuple = OpenStruct.new
	final_tuple.pc = pc
	final_tuple.filename = nil
	final_tuple.row = nil
	final_tuple.gray = nil
	return false, final_tuple 
end

if __FILE__ == $0
	# Check argument
	if ARGV.length != 1
		puts "Please input the name of the executable file."
		exit
	end

	# Name of the executable file
	exe_name = ARGV[0]
	
	# An array to store .debug_line info
	debug_line_arr = Array.new
	# Regex to match valid source file name
	file_re = /((\/.*[\w\-. ]+\.cp*))/i
	# Regex to capture pc and line number
	line_re = /(^0x.*)(\[\s*)(\d*)(.*,\s*)(\d*)(\s*\])/i
	# Regex to capture pc and assembly instructions
	instr_re = /(^.*)(:\s+)([a-fA-Z0-9]+\s)+(\s+)([a-z]+.*)/i
	# To match the line containing '<.*>' in the output of objdump
	angle_re = /(^\s*)([0-9a-z]+)(\s*)(<)(.+)(>)/i 

	# Parse the output of dwarfdump
	filename = ''
	is_fun = false
	line_cnt = 1
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
				file_hash[filename] = Array.new
				line_cnt = 1
			end
			line_info.filename = filename
			# Trim leading and tailing whitespace
			line_info.pc = line_info.pc.strip
			line_info.row = line_info.row.strip
			line_info.col = line_info.col.strip
			while line_cnt < Integer(line_info.row) do
				single_info = OpenStruct.new
				single_info.pc = '0xffffffff'
				single_info.row = line_cnt.to_s
				single_info.col = '0'
				single_info.filename = filename
				file_hash[filename].push(single_info)
				line_cnt += 1
			end
			# Add line info to an array
			file_hash[filename].push(line_info)
			if line_cnt == Integer(line_info.row)
				line_cnt += 1
			end
		end
		}
	}

	# Traverse the output of dwarfdump to decide which line should be printed in gray color
	file_hash.each do |key, value|
		for i in 0..value.size - 1
			is_found = false
			for j in 0..i - 1
				if value.at(j).row == value.at(i).row
					is_found = true
					break
				end
			end
			if is_found
				value.at(i).gray = true
			else
				value.at(i).gray = false
			end
		end
	end

	# Traverse the output of dwarfdump to decide which instruction is generated for multiply source lines
	pc_set = Set.new
	file_hash.reverse_each do |key, value|
		value.reverse_each do |line_info|
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

	# Combine the output of objdump and that of dwarfdump, a hashmap to keep track of the counter to dwarfdump
	# Key is filaname, value is a counter to file_hash, and initially set to 0
	dwarf_cnt_hash = Hash.new
	file_hash.each do |key, value|
		dwarf_cnt_hash[key] = 0
	end

	# The final output index array. Its elements are tuples: (pc, instruction, file, line_num, flag)
	final_arr = Array.new
	instr_hash.each do |key, value|
		if func_set.include? (key)
			final_tuple = OpenStruct.new
			final_tuple.pc = key
			final_tuple.instr = nil
			final_tuple.filename = nil
			final_tuple.row = nil
			final_tuple.gray = nil
			final_arr.push(final_tuple)
		else
			has_src, final_tuple = has_pc(file_hash, key)
			final_tuple.instr = value
			# Has corresponding source code
			if has_src
				while file_hash[final_tuple.filename].at(dwarf_cnt_hash[final_tuple.filename]).pc == '0xffffffff' do
					new_final_tuple = OpenStruct.new
					new_final_tuple.pc = nil
					new_final_tuple.instr = nil
					new_final_tuple.filename = final_tuple.filename
					new_final_tuple.row = file_hash[final_tuple.filename].at(dwarf_cnt_hash[final_tuple.filename]).row
					new_final_tuple.gray = false
					final_arr.push(new_final_tuple)
					dwarf_cnt_hash[final_tuple.filename] += 1
				end
				if Integer(file_hash[final_tuple.filename].at(dwarf_cnt_hash[final_tuple.filename]).pc) == Integer('0x' + final_tuple.pc)
					final_arr.push(final_tuple)
					dwarf_cnt_hash[final_tuple.filename] += 1
				end
			# No corresponding source code
			else
				final_arr.push(final_tuple)
			end
		end
	end

	# Read in all source files, and store them in a hashmap
	# Key is file name, and value is an array, each element is a line.
	file_content_hash = Hash.new
	file_hash.each do |key, value|
		# Read in each source file
		source_content_arr = Array.new
		File.open(key, 'r') do |file|
			# Read in each line
			file.each_line do |line|
				source_content_arr.push(line)
			end
			file.close
		end
		file_content_hash[key] = source_content_arr	
	end

	# Add source file line content to final_tuple
	final_arr.each do |final_tuple|
		if final_tuple.filename != nil
			final_tuple.src = file_content_hash[final_tuple.filename].at(Integer(final_tuple.row) - 1)
		else
			final_tuple.src = nil
		end
	end

	# Add subroutine jump link
	final_arr.each do |final_tuple|
		if final_tuple.instr != nil
			# A subroutine call
			if call_match = final_tuple.instr.match(/(^callq)(\s+)([0-9a-z]+)(\s*)/i)
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
	
	template = File.read('./template.html.erb')
	result = ERB.new(template).result(binding)
	html_filename = 'cross_indexing.html'
	File.open(html_filename, 'w+') do |html_file|
		html_file.write result
		html_file.close
	end

	# ---------------------------------Checking----------------------------------
	final_arr.each do |final_tuple|
		puts final_tuple
	end
	
end

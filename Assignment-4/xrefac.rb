# --
require 'ostruct'
require 'set'
require 'erb'

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
	
	# Print line info for checking
	file_hash.each do |key, value|
		value.each do |line_info|
			puts line_info.pc + ' ' + line_info.row + ' ' + line_info.col + ' ' + line_info.filename 
		end 
	end

	# Print all function names for checking
	func_set.each do |func_name|
		puts func_name
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

	# Print for checking
	instr_hash.each do |key, value|
		puts key + ' ' + value
	end

end

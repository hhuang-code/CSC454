# --
require 'ostruct'
require 'set'

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
	file_re = /([\w\-. ]+\.cp*)/i
	# Regex to capture pc and line number
	line_re = /(^0x.*)(\[\s*)(\d*)(.*,\s*)(\d*)(\s*\])/i
	# Regex to capture pc and assembly instructions
	instr_re = /(^.*)(:\s+)([a-fA-Z0-9]+\s)+(\s+)([a-z]+.*)/i
	# To match '<.*>' in the output of objdump
	angle_re = /(<.*>)/i 

	# Parse the output of dwarfdump
	filename = ''
	file_hash = Hash.new
	IO.popen(['dwarfdump' , exe_name]){
		|io| io.each {|line|
		if line_match = line.match(line_re)
			# A struct to store cp, line number and filename
			line_info = OpenStruct.new
			line_info.pc, lb, line_info.row, comma, line_info.col, rb = line_match.captures
			if file_match = line.match(file_re)
				filename = file_match.captures
				file_hash[filename] = Array.new
			end
			line_info.filename = filename.at(0).strip
			# Trim leading and tailing whitespace
			line_info.pc = line_info.pc.strip
			line_info.row = line_info.row.strip
			line_info.col = line_info.col.strip
			file_hash[filename].push(line_info)
		end
		}
	}

	file_hash.each do |key, value|
		value.each do |line_info|
			puts line_info.pc + ' ' + line_info.row + ' ' + line_info.col + ' ' + line_info.filename 
		end 
	end

	# Parse the output of objdump
	instr_arr = Array.new
	text_sec = false
	IO.popen(['objdump', '-d', exe_name]){
		|io| io.each {|line|
			if text_sec
				if line.downcase.include? 'disassembly'
					text_sec = false
					break;
				else
					# A struct to store instruction location and assembly code
					if instr_match = line.match(instr_re)
						instr_info = OpenStruct.new
						instr_info.pc, colon, macode, space, instr_info.code = instr_match.captures
						# Trim leading and tailing whitespace
						instr_info.pc = instr_info.pc.strip
						instr_info.code = instr_info.code.strip
						instr_arr.push(instr_info)
					else
						if instr_match = line.match(angle_re)
							instr_info = OpenStruct.new
							instr_info.pc = ''
							instr_info.code = ''
							instr_arr.push(instr_info)
						end
					end
				end
			else
				if line.include? '.text'
					text_sec = true
				end
			end
		}
	}

	instr_arr.each do |instr_info|
		puts instr_info.pc + ' ' + instr_info.code
	end
end

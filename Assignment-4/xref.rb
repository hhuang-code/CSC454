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
		# Match line info in .debug_line table
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
			# Add line info to an array
			file_hash[filename].push(line_info)
		end
		}
	}
	
	# Print for checking
	file_hash.each do |key, value|
		value.each do |line_info|
			puts line_info.pc + ' ' + line_info.row + ' ' + line_info.col + ' ' + line_info.filename 
		end 
	end

	# Parse the output of objdump
	instr_hash = Hash.new
	text_sec = false
	IO.popen(['objdump', '-d', exe_name]){
		|io| io.each {|line|
			# In .text section
			if text_sec
				# .text section is over
				if line.downcase.include? 'disassembly'
					text_sec = false
					break;
				# Still in .text section
				else
					# Match an instruction, use a struct to store instruction location and assembly code
					if instr_match = line.match(instr_re)
						instr_info = OpenStruct.new
						instr_info.pc, colon, macode, space, instr_info.code = instr_match.captures
						# Trim leading and tailing whitespace
						instr_info.pc = instr_info.pc.strip
						instr_info.code = instr_info.code.strip
						# Add instruction info to a hashmap, key is pc, value is instruction
						instr_hash[instr_info.pc] = instr_info.code
					# Not match an instruction
					else
						# Match other info excluding an empty line
						if instr_match = line.match(angle_re)
							instr_info = OpenStruct.new
							instr_info.pc = instr_match.captures.at(0)
							instr_info.code = line.strip
							# Add instruction info to a hashmap
							instr_hash[instr_info.pc] = instr_info.code
						end
					end
				end
			# Not in .text section
			else
				# Find .text section
				if line.include? '.text'
					text_sec = true
				end
			end
		}
	}

	# Print for checking
	instr_hash.each do |key, value|
		puts key + ' ' + value
	end

	# Each key in file_hash is a source file name
	template = File.read('./template.html.erb')
	file_hash.each do |key, value|
		# Read in each source file
		key.each do |source_file_name|
			source_content = Array.new
			File.open(source_file_name, 'r') do |source_file|
				# Read in each line
				source_file.each_line do |line|
					source_content.push(line)
				end
				source_file.close
			end
			result = ERB.new(template).result(binding)
			html_file_name = source_file_name.gsub(/\.cpp|\.c|\.h/i, '.html')
			File.open(html_file_name, 'w+') do |html_file|
				html_file.write result
			end
		end
	end	
end

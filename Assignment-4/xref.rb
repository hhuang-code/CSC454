# The entry program

require_relative 'xrefac'
require_relative 'xrefsc'

if __FILE__ == $0
	# Check argument
	if ARGV.length != 2
		puts 'Please input the name of the executable file and running mode.'
		exit
	end

	# Name of the executable file
	exe_name = ARGV[0]
	# Name of running mode: 0 - assembly centric; 1 - source centric
	mode = ARGV[1]

	if mode == '0'
		start_time = Time.now.to_s
		main_loc = xrefac(exe_name)
		end_time = Time.now.to_s
		location = Dir.pwd
		template = File.read('./template_index.html.erb')
		result = ERB.new(template).result(binding)
		if !File.exist?(Dir.pwd + '/HTML')
			Dir.mkdir(Dir.pwd + '/HTML')
		end 
		html_filename = Dir.pwd + '/HTML/' + 'index.html'
		File.open(html_filename, 'w+') do |html_file|
			html_file.write result
			html_file.close
		end
	elsif mode == '1'
		xrefsc(exe_name)
	else
		puts 'Invalid mode.'
		exit	
	end

end

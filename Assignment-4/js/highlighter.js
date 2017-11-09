// Define all keywords in C language
var dict = {'auto': 'c_auto', 'short': 'c_short', 'int': 'c_int', 'long': 'c_long',
'float': 'c_float', 'double': 'c_double', 'char': 'c_char', 'struct': 'c_struct',
'union': 'c_union', 'enum': 'c_enum', 'typedef': 'c_typedef', 'const': 'c_const',
'unsigned': 'c_unsigned', 'signed': 'c_signed', 'extern': 'c_extern', 'register': 'c_register',
'static': 'c_static', 'volatile': 'c_volatile', 'void': 'c_void', 'if': 'c_if',
'else': 'c_else', 'switch': 'c_switch', 'case': 'c_case', 'for': 'c_for',
'do': 'c_do', 'while': 'c_while', 'goto': 'c_goto', 'continue': 'c_continue',
'break': 'c_break', 'default': 'c_default', 'sizeof': 'c_sizeof', 'return': 'c_return',
'inline': 'c_inline', 'restrict': 'c_restrict'};

var str = 'void main(){int a; int b; a = b}';

function compare(a, b){
	return b - a;
}

// Repalce src started at index pos with dest
String.prototype.replaceAt = function(index, src, dest){
	return this.substr(0, index) + dest + this.substr(index + src.length);
}

// Check whether a string contains keywords and highlight them
function check_and_replace(string){
	string = string.trim();
	for(var key in dict){
		var result = [];
		re = new RegExp(key + ' ', "g");
		while((match = re.exec(string)) != null){
			result.push(match.index);
		}
		// Find matches, sort index in descending order
		if(result.length > 0){
			result.sort(compare);
			dest = '<span class = "c_' + key + '">' + key + "</span>";
			for(var i = 0; i < result.length; i++){
				string = string.replaceAt(result[i], key, dest);
			}
		}
	}
	return string;
}

// Get elements by class name; return value is an array.
function get_elem_by_class(parents, classname){
	var class_elems = [];
	var all_elems = document.getElementsByTagName('*');
	for(var i = 0; i < all_elems.length; i++){
		if(all_elems[i].className == classname){
			class_elems.push(all_elems[i]);
		}
	}

	return class_elems;
}

// Highlight all keywords
function highlight(){
	var source = get_elem_by_class('document', 'source');
	var spans = get_elem_by_class(source[0], 'srcline');
	var line_num = spans.length;
	for(var i = 0; i < line_num; i++){
		var line = spans[i].innerHTML;
		spans[i].innerHTML = check_and_replace(line);
		console.log(check_and_replace(line));
	}
}

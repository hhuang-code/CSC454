function compare(a, b){
	return b - a;
}

// Repalce src started at index pos with dest
String.prototype.replaceAt = function(index, src, dest){
	return this.substr(0, index) + dest + this.substr(index + src.length);
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

// Whether text nodes under the given nodes contain target. If contain, surround it with [[ and ]]
function find_all_text(node, target){
	for(node = node.firstChild; node; node = node.nextSibling){
		if(node.nodeType == 3){
			var result = [];
			re = new RegExp(target, 'g');
			while((match = re.exec(node.textContent)) != null){
				result.push(match.index);
			}
			if(result.length > 0){
				tmp = node.textContent;
				result.sort(compare);
				dest = '[[' + target + ']]';
				for(var i = 0; i < result.length; i++){
					tmp = tmp.replaceAt(result[i], target, dest);
				}
				node.textContent = tmp;
			}
		}else{
			find_all_text(node, target);
		}
  	}
}

// Search function
function search(){
	var target = document.getElementById('search_target').value;

	if(target.trim() != ''){
		var source = get_elem_by_class('document', 'source')[0];
		find_all_text(source, target.trim());
	}else{
		alert('Nothing entered!');
	}
}

// Remove all [[ and ]]
function clear_all_text(node){
	for(node = node.firstChild; node; node = node.nextSibling){
		if(node.nodeType == 3){
			var result = [];
			re = new RegExp(/\[\[\S+\]\]/, 'g');
			while((match = re.exec(node.textContent)) != null){
				result.push(match.index);
			}
			if(result.length > 0){
				tmp = node.textContent;
				tmp = tmp.replace(/\[\[/g, '');
				tmp = tmp.replace(/\]\]/g, '');
				node.textContent = tmp;
			}
		}else{
			clear_all_text(node);
		}
	}
}

// Clear function
function clear_found(){
	var source = get_elem_by_class('document', 'source')[0];
	clear_all_text(source);
}

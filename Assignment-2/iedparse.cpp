#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <deque>
#include <map>
#include <algorithm>
#include <exception>

#include "scan.hpp"

using namespace std;

static token input_token, next_token;

bool bracket = true;

// To index enum token
const string token_str[] = {"t_read", "t_write", "t_id", "t_literal", "t_gets",
    "t_add", "t_sub", "t_mul", "t_div", 
    "t_lparen", "t_rparen",
    "t_eq", "t_neq", "t_lte", "t_gt", "t_lt", "t_gte",
    "t_if", "t_fi", "t_do", "t_od", "t_check",
    "t_eoa", "t_eof", "t_eps"};

// To index enum symbol
const string symbol_str[] = {"nt_program", "nt_stmt_list", "nt_stmt", "nt_relation", 
    "nt_expr_tail", "nt_expr", "nt_term_tail", "nt_term", 
    "nt_factor_tail", "nt_factor", "nt_ro_op", "nt_ao_op", "nt_mo_op"};
    
// Copy token_str to token_vec
vector<string> token_vec (token_str, token_str + sizeof (token_str) / sizeof (token_str[0]));

// Copy symbol_str to symbol_vec
vector<string> symbol_vec (symbol_str, symbol_str + sizeof (symbol_str) / sizeof (symbol_str[0]));

// First set and follow set
map<symbol, string> first_set, follow_set; 

const char* names[] = {"read", "write", "id", "literal", "gets",
    "add", "sub", "mul", "div",
    "lparen", "rparen",
    "==", "<>", "<=", ">", "<", ">=",
    "if", "fi", "do", "od", "check",
    "eoa", "eof", "epsilon"};

// Build a first set and follow set
void buildset (){
    ifstream in ("ffp.data");
	string line = "";
	deque<string> dq;
	size_t pos = 0;
	if (in){
		while (getline (in, line)){
			if(line.find ("FIRST") == 0 || line.find ("FOLLOW") == 0 || line.find ("PREDICT") == 0){
				dq.clear();
				while ((pos = line.find ("&")) != string::npos){
					dq.push_back(line.substr (0, pos));
					line.erase (0, pos + 1);
				}
				dq.push_back (line);
				if (dq.front() == "FIRST"){
					dq.pop_front();
					first_set[symbol(find(symbol_vec.begin(), symbol_vec.end(), dq.front()) - symbol_vec.begin())] = dq.back();
				}else if (dq.front() == "FOLLOW"){
					dq.pop_front();
					follow_set[symbol(find(symbol_vec.begin(), symbol_vec.end(), dq.front()) - symbol_vec.begin())] = dq.back();
				}
			}
		}
	}else {
		exit (1);
	}
}

// Print first set and follow set to stdout
void outputset (){
    cout << "-------------------- FIRST SET --------------------" << endl;
    map<symbol, string>::iterator first_iter = first_set.begin();
	for (; first_iter != first_set.end(); first_iter++){
		cout << first_iter->first << " --> " << first_iter->second << endl;
	}
	cout << endl;
    
    cout << "-------------------- FOLLOW SET --------------------" << endl;
	map<symbol, string>::iterator follow_iter = follow_set.begin();
	for (; follow_iter != follow_set.end(); follow_iter++){
		cout << follow_iter->first << " -->" << follow_iter->second << endl;
	}
	cout << endl;
}

// Check whether a token in a first_set specified by a symbol
bool checkfirst (token next_token, symbol cur_symbol, map<symbol, string> first_set) {
    string s = first_set[cur_symbol];
    if (s.find(token_vec[token(next_token)]) != string::npos) {
        return true;
    } else {
        return false;
    }
}

// Check whether a token in a follow set specified by a symbol
bool checkfollow (token next_token, symbol cur_symbol, map<symbol, string> follow_set) {
    string s = follow_set[cur_symbol];
    if (s.find(token_vec[token(next_token)]) != string::npos){
        return true;
    } else {
        return false;
    }
}

// Check whether a symbol goes to epsilon
bool checkeps (symbol cur_symbol) {
	if (cur_symbol == nt_stmt_list || cur_symbol == nt_expr_tail || cur_symbol == nt_term_tail || cur_symbol == nt_factor_tail) {
		return true;
	} else {
		return false;
	}
}

// Check whether a token in the starter set
bool checkstarter (token input_token) {
	if (input_token == t_lparen || input_token == t_if || input_token == t_do) {
		return true;
	} else {
		return false;
	}
}

// Consume an input_token
void match (token expected) {
    if (input_token == expected) {
        /*cout << "matched " << names[input_token];
        if (input_token == t_id || input_token == t_literal) {
            cout << ": " << token_image;
	   }
        cout << endl;*/
        input_token = scan ();
    } else {
    	string emgs = "[Error] Line " + to_string(linecnt) + ": " + token_image + " is unexpected.\n";
        error_msg.push (emgs);
    }
}

// Error checking subroutine
void check_for_error (symbol cur_symbol, map<symbol, string> first_set, map<symbol, string> follow_set) {
	if (! checkfirst (input_token, cur_symbol, first_set) && ! (checkeps (cur_symbol) && checkfollow (input_token, cur_symbol, follow_set))) {
		string emgs = "[Error] Line " + to_string(linecnt) + ": " + token_image + " is unexpected.\n";
        error_msg.push (emgs);
        cout << "cur_symbol: " << symbol_vec[cur_symbol] << endl;
		do {
            input_token = scan ();
        } while (! checkfirst (input_token, cur_symbol, first_set) && ! checkfollow (input_token, cur_symbol, follow_set) && ! checkstarter (input_token) && ! (input_token == t_eof));
	}
}

string program ();
string stmt_list ();
string stmt ();
string relation ();
string expr_tail (string se);
string expr ();
string term_tail (string se);
string term ();
string factor_tail (string se);
string factor ();
string ro_op ();
string add_op ();
string mul_op ();

string program () {
    check_for_error (nt_program, first_set, follow_set);
    string semantic = "(program \n";
    switch (input_token) {
        case t_id:
        case t_read:
        case t_write:
    	case t_if:
    	case t_do:
    	case t_check:
        case t_eof:
            //cout << "predict program --> stmt_list eof" << endl;
            semantic += "[ " + stmt_list () + "]\n";
            match (t_eof);
            break;
        default: break;
    }

    return semantic + ")\n";
}

string stmt_list () {
    check_for_error (nt_stmt_list, first_set, follow_set);
    string semantic = "";
	switch (input_token) {
	    case t_id:
	    case t_read:
	    case t_write:
		case t_if:
		case t_do:
		case t_check:
			//cout << "predict stmt_list --> stmt stmt_list" << endl;
			semantic += stmt ();
			semantic += stmt_list ();
			break;
		case t_fi:
		case t_od:
	    case t_eof:		/*  epsilon production */
	        //cout << "predict stmt_list --> epsilon" << endl;
	        return "";
	        break;          
	    default: break;
	}

    return semantic;
}

string stmt () {
    check_for_error (nt_stmt, first_set, follow_set);
  	string semantic = "(";
	switch (input_token) {
		case t_id:
		    //cout << "predict stmt --> id gets expr" << endl;
		    semantic = semantic + ":= " + " \"" +  token_image + "\" ";
		    match (t_id);
		    match (t_gets);      
		    semantic += relation();
		    break;
		case t_read:
		    //cout << "predict stmt --> read id" << endl;
		    match (t_read);
		    semantic = semantic + "read " + "\"" + token_image + "\"";
		    match (t_id);
		    break;
		case t_write:
		    //cout << "predict stmt --> write expr" << endl;
		    match (t_write);
		    semantic = semantic + "write " + relation();
		    break;
		case t_if:
	  		//cout << "predict stmt --> if relation stmt_list fi" << endl;
	  		match (t_if);
		    semantic += "if\n";
	  		semantic += "(" + relation () + ")\n";
	  		semantic += "[ " + stmt_list () + "]\n";
	  		match(t_fi);
	  		break;
		case t_do:
	  		//cout << "predict stmt --> do stmt_list od" << endl;
	  		match (t_do);
		    semantic += "do\n";
	  		semantic += "[ " + stmt_list () + "]\n";
	  		match (t_od);
			break;
		case t_check:
	  		//cout << "predict stmt --> check relation" << endl;
	  		match (t_check);
		    semantic += "check ";
	  		semantic += relation ();
	  		break;
		default: break;
	}

	return semantic + ")\n";
}

string relation () {
    check_for_error (nt_relation, first_set, follow_set);
    string semantic = "(";
  	switch (input_token) {
			case t_lparen:
			case t_id:
			case t_literal:
				//cout << "predict relation --> expr expr_tail" << endl;
				semantic = expr_tail (expr ());
				break;
			default: break;
  	}
  	
    return semantic;
}

string expr_tail (string se) {
    check_for_error (nt_expr_tail, first_set, follow_set);
    string semantic = "";
  	switch (input_token) {
  		case t_eq:
  		case t_neq:
  		case t_lt:
  		case t_gt:
  		case t_lte:
  		case t_gte:
  			//cout << "predict expr_tail --> ro_op expr" << endl;
  			semantic += ro_op ();
        	semantic += se;
  			semantic += expr ();
  			break;
  		case t_rparen:
  		case t_id:
  		case t_read:
  		case t_write:
  		case t_if:
  		case t_do:
  		case t_check:
  		case t_fi:
  		case t_od:
  		case t_eof:		/* epsilon production */
  			//cout << "predict expr_tail --> epsilon" << endl;
			return se;
  		default: break;
  	}
  	
    return semantic;
}

string expr () {
    check_for_error (nt_expr, first_set, follow_set);
    string semantic = "";
	switch (input_token) {
	    case t_id:
	    case t_literal:
	    case t_lparen:
	        //cout << "predict expr --> term term_tail" << endl;
	        semantic += term_tail (term ());
	        break;
		default: break;
	}
	
  return semantic;
}

string term_tail (string se) {
    check_for_error (nt_term_tail, first_set, follow_set);
    string semantic = "(";
    switch (input_token) {
        case t_add:
        case t_sub:
            //cout << "predict term_tail --> add_op term term_tail" << endl;
            semantic += add_op ();
            semantic += se;
            semantic += term_tail (term ());
            break;
		case t_eq:
		case t_neq:
		case t_lt:
		case t_gt:
		case t_lte:
		case t_gte:
        case t_rparen:
        case t_id:
        case t_read:
        case t_write:
		case t_if:
		case t_do:
		case t_check:
		case t_fi:
		case t_od:
        case t_eof:		/*  epsilon production */
            //cout << "predict term_tail --> epsilon" << endl;
			return se;        
        default: break;
    }
    return semantic + ")";
}

string term () {
    check_for_error (nt_term, first_set, follow_set);
    string semantic = "";
    switch (input_token) {
        case t_id:
        case t_literal:
        case t_lparen:
            //cout << "predict term --> factor factor_tail" << endl;
            semantic += factor_tail (factor ());
            break;
        default: break;
    }
    
    return semantic;
}

string factor_tail (string se) {
    check_for_error (nt_factor_tail, first_set, follow_set);
    string semantic = "(";
    switch (input_token) {
        case t_mul:
        case t_div:
            //cout << "predict factor_tail --> mul_op factor factor_tail" << endl;
            semantic += mul_op ();
            semantic += se;
            semantic += factor_tail (factor ());
            break;
        case t_add:
        case t_sub:
		case t_eq:
		case t_neq:
		case t_lt:
		case t_lte:
		case t_gt:
		case t_gte:
        case t_rparen:
        case t_id:
        case t_read:
        case t_write:
		case t_if:
		case t_do:
		case t_check:
		case t_fi:
		case t_od:
        case t_eof:		/* epsilon production */
            //cout << "predict factor_tail --> epsilon" << endl;
			return se;        
        default: break;
    }
    
    return semantic + ")";
}

string factor () {
    check_for_error (nt_factor, first_set, follow_set);
    string semantic = "";
    switch (input_token) {
        case t_id :
            //cout << "predict factor --> id" << endl;
            semantic = semantic + "(id " + " \"" +  token_image + "\")";
            match (t_id);
            break;
        case t_literal:
            //cout << "predict factor --> literal" << endl;
            semantic = semantic + "(num " + " \"" +  token_image + "\")";
            match (t_literal);
            break;
        case t_lparen:
            //cout << "predict factor --> lparen relation rparen" << endl;
            match (t_lparen);
            semantic += relation ();
            match (t_rparen);
            break;
        default: break;
    }
    
    return semantic + "";
}

string ro_op () {
    check_for_error (nt_ro_op, first_set, follow_set);
	string semantic = "";
	switch (input_token) {
		case t_eq:
			//cout << "predict ro_op --> equal" << endl;
			match (t_eq);
      		semantic = "== ";
			break;
		case t_neq:
			//cout << "predict ro_op --> not equal" << endl;
			match (t_neq);
      		semantic = "<> ";
			break;
		case t_lt:
			//cout << "predict ro_op --> less than" << endl;
			match (t_lt);
      		semantic = "< ";
			break;
		case t_gt:
			//cout << "predict ro_op --> greater than" << endl;
			match (t_gt);
      		semantic = "> ";
			break;
		case t_lte:
			//cout << "predict ro_op --> less than and equal" << endl;
			match (t_lte);
      		semantic = "<= ";
			break;
		case t_gte:
			//cout << "predict ro_op --> greater than and equal" << endl;
			match (t_gte);
      		semantic = ">= ";
			break;
		default: break;
	}
	
	return semantic;
}

string add_op () {
    check_for_error (nt_ao_op, first_set, follow_set);
	string semantic = "";
    switch (input_token) {
        case t_add:
            //cout << "predict add_op --> add" << endl;
            match (t_add);
            semantic = "+ ";
            break;
        case t_sub:
            //cout << "predict add_op --> sub" << endl;
            match (t_sub);
            semantic =  "- ";
            break;
        default: break;
    }
    
    return semantic;
}

string mul_op () {
    check_for_error (nt_mo_op, first_set, follow_set);
	string semantic = "";
    switch (input_token) {
        case t_mul:
            //cout << "predict mul_op --> mul" << endl;
            match (t_mul);
            semantic = "* ";
            break;
        case t_div:
            //cout << "predict mul_op --> div" << endl;
            match (t_div);
            semantic = "/ ";
            break;
        default: break;
    }
    
    return semantic;
}

int main () {

    // Make sure error message queue is empty
    while (!error_msg.empty ()){
        error_msg.pop();
    }

    // Build first and follow set
    buildset ();

    input_token = scan ();
    string AST = program ();

    // Print error messages if parsing fail, otherwise print AST
    if (!error_msg.empty ()) {
        while (!error_msg.empty ()){
            cout << error_msg.front () << endl;
            error_msg.pop ();
        }
    } else {
        cout << AST << endl;
    }

    return 0;
}

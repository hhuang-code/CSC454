#include <iostream>
#include <locale>
#include <set>

#include "scan.hpp"

using namespace std;

char token_image[100];

// Set initial line number to 1
int linecnt = 1;

// Store parsing error message.
queue<string> error_msg;

static char c = ' ';

// A valid operator should be followed by space, alpha, digit and, left parenthesis and underscore. 
bool valid_next (char c) {
    if (isspace (c) || isalpha(c) || isdigit(c) || c == '_' || c == '(') {
        return true;
    } else {
        return false;
    }
}

// Delete invalid character
void delete_invalid_char () {
    if (!valid_next (c)) {
        string emsg = "[Error] Line " + to_string(linecnt) + ": " + string(1, c) + " is unexpected.\n";
        error_msg.push (emsg);
    }
    do {
        cin.get(c);
    } while (!valid_next (c));
}

token scan() {
    // next available char; extra (int) width accommodates EOF
    int i = 0;              /* index into token_image */

    // skip white space
    while (isspace(c)) {
        if (c == '\n'){
            linecnt++;
        }
		cin.get(c);    
    }

    //  Deal with EOF
    if (c == '$') {
        return t_eof;
	}

    if (isalpha(c)) {
        do {
            token_image[i++] = c;
            cin.get(c);
        } while (isalpha(c) || isdigit(c) || c == '_');
        token_image[i] = '\0';

        if (string(token_image) == "read") {
            return t_read;
        } else if (string(token_image) == "write") {
            return t_write;
        } else if (string(token_image) == "do") {
            return t_do;
        } else if (string(token_image) == "od") {
            return t_od;
        } else if (string(token_image) == "if") {
            return t_if;
        } else if (string(token_image) == "fi") {
            return t_fi;
        } else if (string(token_image) == "check") {
            return t_check;
        } else {
			return t_id;
		}
    } else if (isdigit(c)) {
        do {
            token_image[i++] = c;
			cin.get(c);
        } while (isdigit(c));
        token_image[i] = '\0';
        return t_literal;
    } else {
		switch (c) {
        	case ':':
				cin.get(c);
            	if (c != '=') {
                    string emsg = "[Error] Line " + to_string(linecnt) + ": " + string(1, c == ' ' ? ':' : c) + " is unexpected.\n";
                    error_msg.push (emsg);
                    while (!valid_next (c)) {
                        cin.get(c);
                    }
            	} else {
                    cin.get(c);
                    delete_invalid_char ();
                }
                return t_gets;
        	case '+':
                cin.get(c);
                delete_invalid_char ();
            	return t_add;
        	case '-':
                cin.get(c);
				delete_invalid_char ();
            	return t_sub;
        	case '*':
                cin.get(c);
			   	delete_invalid_char ();
            	return t_mul;
        	case '/': 
                cin.get(c);
            	delete_invalid_char ();
            	return t_div;
        	case '(': 
            	cin.get(c);
            	return t_lparen;
        	case ')':
   				cin.get(c);				
            	return t_rparen;
        	case '=':
            	cin.get(c);
            	if (c != '=') {
                	string emsg = "[Error] Line " + to_string(linecnt) + ": " + string(1, c == ' ' ? '=' : c) + " is unexpected.\n";
                    error_msg.push (emsg);
                    while (!valid_next (c)) {
                        cin.get(c);
                    }
            	} else {
                    cin.get(c);
					delete_invalid_char ();
                	return t_eq;
            	}
        	case '>' :
                cin.get(c);
            	if (c != '=') {
                    delete_invalid_char ();
                	return t_gt;
				} else {
                    cin.get(c);
					delete_invalid_char ();
                	return t_gte;
            	}
        	case '<' :
            	cin.get(c);
            	if (c != '>' && c != '=') {
                    delete_invalid_char ();
                	return t_lt;
            	} else if (c == '>') {
					cin.get(c);
                    delete_invalid_char ();
                	return t_neq;
            	} else {
                    cin.get(c);
                    delete_invalid_char ();
                	return t_lte;
            	}
        	default:
            	string emsg = "[Error] Line " + to_string(linecnt) + ": " + string(1, c) + " is unexpected.\n";
                error_msg.push (emsg);
                cin.get(c);
                break;
    	}
	}
}

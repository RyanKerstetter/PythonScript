#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>

#include "util.hpp"
using namespace std;

enum TokenType {
	KEYWORD,
	IDENTIFIER,
	NUMBER,
	STRING,
	OP,
	OPEN_PAR,
	CLOSE_PAR,
	OPEN_BRACE,
	CLOSE_BRACE,
	ASSIGNMENT_OPERATOR,
	DELIMITER, // ,
	MEMBER_ACCESS, // .
	END_OF_LINE,
	END_OF_FILE,
};

string tokenTypeToString(TokenType type) {
	switch (type) {
	case KEYWORD: return "KEYWORD";
	case IDENTIFIER: return "IDENTIFIER";
	case NUMBER: return "NUMBER";
	case STRING: return "STRING";
	case OP: return "OP";
	case OPEN_PAR: return "OPEN_PAR";
	case CLOSE_PAR: return "CLOSE_PAR";
	case OPEN_BRACE: return "OPEN_BRACE";
	case CLOSE_BRACE: return "CLOSE_BRACE";
	case DELIMITER: return "DELIMITER";
	case END_OF_LINE: return "END_OF_LINE";
	case END_OF_FILE: return "END_OF_FILE";
	case ASSIGNMENT_OPERATOR: return "ASSIGNMENT_OPERATOR";
	case MEMBER_ACCESS: return "MEMBER_ACCESS";
	}
	return "UNKNOWN";
}

struct Token {
	TokenType type;
	string value;
};

std::ostream& operator<<(std::ostream& os, const Token& obj) {
	string type = tokenTypeToString(obj.type);
	os << "Token(" << type << ", " << obj.value << ")";
	return os;
}
vector<string> keywords = {
	"if", "else", "while", "for", "return", "fun", "class", "struct",
};

vector<string> operators = {
	"+", "-", "*", "/", "%", "==", "!=", ">", "<", ">=", "<=", "&&", 
	"||", "!", "++", "--","->",
};

vector<string> assignmentOperators = {
	"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
};

vector<Token> tokenize(vector<string> lines) {
	vector<Token> tokens;
	for (string line : lines) {
		for (int i = 0; i < line.size(); i++) {
			char c = line[i];
			if (isWhiteSpace(c)) {
				continue;
			}
			if (c == '.') {
				tokens.push_back({ MEMBER_ACCESS, "." });
				continue;
			}
			if (isAlpha(c)) {
				string word = "";
				while (isAlpha(c)) {
					word += c;
					c = line[++i];
				}
				i--;
				if (find(keywords.begin(), keywords.end(), word) != keywords.end())
					tokens.push_back({ KEYWORD, word });
				else
					tokens.push_back({ IDENTIFIER, word });
				continue;
			}
			if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '!' || c == '>' || c == '<') {
				if (c == '-' && i + 1 < line.size() && isNumeric(line[i + 1])) {}
				else {
					string op = "";
					op += c;
					string op2 = op + line[i + 1];
					if (find(operators.begin(), operators.end(), op2) != operators.end()) {
						tokens.push_back({ OP, op2 });
						i++;
					}
					else if (find(assignmentOperators.begin(), assignmentOperators.end(), op2) != assignmentOperators.end()) {
						tokens.push_back({ ASSIGNMENT_OPERATOR, op2 });
						i++;
					}
					else {
						if (c == '=') tokens.push_back({ ASSIGNMENT_OPERATOR, "=" });
						else tokens.push_back({ OP, op });
					}
					
					continue;
				}
			}
			if (isNumeric(c)) {
				string number = "";
				while (isNumeric(c)) {
					number += c;
					c = line[++i];
				}
				i--;
				tokens.push_back({ NUMBER, number });
				continue;
			}
			if (c == '"') {
				string str = "";
				c = line[++i];
				while (c != '"') {
					str += c;
					c = line[++i];
				}
				tokens.push_back({ STRING, str });
			}
			else if (c == '(') tokens.push_back({ OPEN_PAR, "(" });
			else if (c == ')') tokens.push_back({ CLOSE_PAR, ")" });
			else if (c == '{') tokens.push_back({ OPEN_BRACE, "{" });
			else if (c == '}') tokens.push_back({ CLOSE_BRACE, "}" });
			else if (c == ',') tokens.push_back({ DELIMITER, "," });
		}
		tokens.push_back({ END_OF_LINE, "" });
	}
	tokens.push_back({ END_OF_FILE, "" });
	return tokens;
}





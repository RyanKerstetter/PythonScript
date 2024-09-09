#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include "util.hpp"
#include "tokenizer.hpp"
using namespace std;

/*
	GOAL for variable / struct management
	- Map of variable names
	- Map of struct names
	- Parse all variable declarations to map to DataType int
	- Parse all struct declaration to map to DataType int
*/

enum ASTNodeType {
	BLOCK,
	OPERATOR,
	LITERAL,
	VARIABLE,
	ASSIGNMENT,
	DECLARATION,
	FUNCTION_CALL,
	FUNCTION_DECLARATION,
	STRUCT_DECLARATION,
	IF_STATEMENT,
	WHILE_STATEMENT,
	EXPR_WRAPPER,
	STATEMENT_WRAPPER,
	RETURN_BLOCK,
};

enum DataType {
	NULL_TYPE = -1,
	INT = 0,
	FLOAT = 1,
	BOOL = 2,
	STR = 3,
	LIST = 4,
};

int nextDataType = 5;

struct StructData {
	string name;
	map<string, DataType> fields;
};

struct Data {
	DataType type;
	void* data;
};

string DataToString(Data d) {
	switch (d.type) {
	case INT:
		return to_string(*(int*)d.data);
	case FLOAT:
		return to_string(*(float*)d.data);
	case BOOL:
		return *(bool*)d.data ? "true" : "false";
	case STR:
		return *(string*)d.data;
	case LIST:
		return "List";
	default:
		return "Unknown";
	}
};

struct List {
	Data* data;
	int size;
};

map<string, Data*> variables;
map<DataType, StructData> structs;
map<string, DataType> types = {
	{"int", INT},
	{"float", FLOAT},
	{"bool", BOOL},
	{"str", STR},
	{"list", LIST},
};

void addDataType(string name) {
	types[name] = (DataType)nextDataType;
	nextDataType++;
}

void setDataType(string name, DataType type) {
	types[name] = type;
}

Data* createDataFromType(DataType t) {
	Data* d = new Data();
	d->type = t;
	if (t == INT) d->data = new int(0);
	else if (t == FLOAT) d->data = new float(0.0);
	else if (t == BOOL) d->data = new bool(false);
	else if (t == STR) d->data = new string("");
	else if (t == LIST) d->data = new List();
	else if (t >= nextDataType) {
		cerr << "Error: invalid data type " << t << endl;
		exit(1);
	}
	else {
		StructData s = structs[t];
		map<string, Data*> fields;
		for (auto field : s.fields) {
			DataType type = field.second;
			Data* nullData = new Data{ type, nullptr };
			fields[field.first] = nullData;
		}
		d->data = new map<string, Data*>(fields);
	}
	return d;
}

class MemberList {
public:
	vector<string> members;
	MemberList(vector<string> members) : members(members) {}
	MemberList(string str) {
		members.push_back(str);
	}
	Data* get() {
		if (members.size() == 0) {
			cerr << "Error: invalid member access on empty member list\n";
			exit(1);
		}
		if (variables.find(members[0]) == variables.end()) {
			cerr << "Error: variable " << members[0] << " not declared\n";
			exit(1);
		}
		Data* current = variables[members[0]];
		for (int i = 1; i < members.size(); i++) {
			DataType currentType = current->type;
			if (currentType >= LIST) {
				StructData s = structs[currentType];
				current = ((map<string, Data*>*)current->data)->at(members[i]);
			}
			else {
				cerr << "Error: invalid member access on non struct type\n;";
				cerr << "Member: " << members[0] << " on type: " << currentType << endl;
				cerr << "Cant access member (" << members[i] << ") on type (" << currentType << ")" << endl;
				exit(1);
			}
		}
		return current;
	}

	string getFullName() {
		string name = "";
		for (string member : members) {
			name += member + ".";
		}
		return name;
	}
};

class Node {
public:
	ASTNodeType type;
	Node(ASTNodeType type) : type(type) {}
	virtual void print(int depth) = 0;
};

class Expression: public Node{
public:
	ASTNodeType type;
	Expression(ASTNodeType type) : type(type), Node(type) {}
	virtual Data* evaluate() = 0;
};

class Operator : public Expression {
public:
	string op;
	Expression* left;
	Expression* right;
	Operator(string op, Expression* left, Expression* right) : Expression(OPERATOR), op(op), left(left), right(right) {}
	Data* evaluate() {
		Data* leftData = left->evaluate();
		Data* rightData = right->evaluate();
		if (leftData->type == INT && rightData->type == INT) {
			int* leftInt = (int*)leftData->data;
			int* rightInt = (int*)rightData->data;
			int* result = new int();
			if (op == "+") {
				result = new int(*leftInt + *rightInt);
			}
			else if (op == "-") {
				result = new int(*leftInt - *rightInt);
			}
			else if (op == "*") {
				result = new int(*leftInt * *rightInt);
			}
			else if (op == "/") {
				result = new int(*leftInt / *rightInt);
			}
			else if (op == ">") {
				bool* b = new bool(*leftInt > *rightInt);
				return new Data{ BOOL, b };
			}
			else if (op == "<") {
				bool* b = new bool(*leftInt < *rightInt);
				return new Data{ BOOL, b };
			}
			else if (op == "==") {
				bool* b = new bool(*leftInt == *rightInt);
				return new Data{ BOOL, b };
			}
			else if (op == "!=") {
				bool* b = new bool(*leftInt != *rightInt);
				return new Data{ BOOL, b };
			}
			else {
				cerr << "Error: invalid operator " << op << " for types " << leftData->type << " and " << rightData->type << endl;
				exit(1);
			}
			return new Data{ INT, result };
		}
		if (leftData->type == FLOAT && rightData->type == FLOAT) {
			float* leftFloat = (float*)leftData->data;
			float* rightFloat = (float*)rightData->data;
			float* result = new float();
			if (op == "+") {
				result = new float(*leftFloat + *rightFloat);
			}
			else if (op == "-") {
				result = new float(*leftFloat - *rightFloat);
			}
			else if (op == "*") {
				result = new float(*leftFloat * *rightFloat);
			}
			else if (op == "/") {
				result = new float(*leftFloat / *rightFloat);
			}
			else {
				cerr << "Error: invalid operator " << op << " for types " << leftData->type << " and " << rightData->type << endl;
				exit(1);
			}
			return new Data{ FLOAT, result };
		}
		if (leftData->type == BOOL && rightData->type == BOOL) {
			bool* leftBool = (bool*)leftData->data;
			bool* rightBool = (bool*)rightData->data;
			bool* result = new bool();
			if (op == "&&") {
				result = new bool(*leftBool && *rightBool);
			}
			else if (op == "||") {
				result = new bool(*leftBool || *rightBool);
			}
			else {
				cerr << "Error: invalid operator " << op << " for types " << leftData->type << " and " << rightData->type << endl;
				exit(1);
			}
			return new Data{ BOOL, result };
		}
		cerr << "Error: invalid operator " << op << " for types " << leftData->type << " and " << rightData->type << endl;
		exit(1);
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Operator: " << op << endl;
		left->print(depth + 1);
		right->print(depth + 1);
	}
};

class Literal : public Expression {
public:
	Data* data;
	Literal(Data* data) : Expression(LITERAL), data(data) {}
	Data* evaluate() {
		return data;
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		switch (data->type) {
			case INT:
				cout << "Literal: " << *(int*)data->data << endl;
				break;
			case FLOAT:
				cout << "Literal: " << *(float*)data->data << endl;
				break;
			case BOOL:
				cout << "Literal: " << *(bool*)data->data << endl;
				break;
			case STR:
				cout << "Literal: " << *(string*)data->data << endl;
				break;
		}
	}
};

class Statement : public Node{
public:
	ASTNodeType type;
	Statement(ASTNodeType type) : type(type), Node(type) {}
	virtual void execute() = 0;
};

class StatementWrapper : public Expression {
public:
	Statement* statement;
	StatementWrapper(Statement* statement) : statement(statement), Expression(STATEMENT_WRAPPER) {}
	Data* evaluate() {
		statement->execute();
		return new Data{ NULL_TYPE, nullptr };
	}

	void print(int depth) override {
		statement->print(depth);
	}
};

class Return : public Expression {
public:
	Expression* expression;
	Return(Expression* expression) : expression(expression), Expression(RETURN_BLOCK) {}
	Data* evaluate() {
		return expression->evaluate();
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Return: " << endl;
		expression->print(depth + 1);
	}
};

class Variable : public Expression {
public:
	MemberList members;
	Variable(MemberList members) : Expression(VARIABLE), members(members) {}
	Data* evaluate() {
		Data* data = members.get();
		return data;
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Variable: ";
		for (string member : members.members) {
			cout << member << ".";
		}
		cout << endl;
	}
};

class ExpressionWrapper : public Statement {
public:
	Expression* expression;
	ExpressionWrapper(Expression* expression) : expression(expression), Statement(EXPR_WRAPPER) {}
	void execute() {
		expression->evaluate();
	}

	void print(int depth) override {
		expression->print(depth);
	}
};

class Block : public Statement {
public:
	vector<Statement*> statements;
	Block() : Statement(BLOCK) {}
	void execute() {
		for (Statement* statement : statements) {
			statement->execute();
		}
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Block: " << endl;
		for (Statement* statement : statements) {
			statement->print(depth + 1);
		}
	}
};

class ReturnBlock : public Expression {
public:
	vector<Expression*> expressions;
	ReturnBlock(vector<Expression*> expressions) : expressions(expressions), Expression(RETURN_BLOCK) {}
	ReturnBlock(Block* block) : Expression(RETURN_BLOCK) {
		for (Statement* statement : block->statements) {
			if (statement->type == EXPR_WRAPPER) {
				ExpressionWrapper* wrapper = (ExpressionWrapper*)statement;
				expressions.push_back(wrapper->expression);
			}
			else {
				StatementWrapper* wrapper = new StatementWrapper(statement);
				expressions.push_back(wrapper);
			}
		}
	}

	Data* evaluate() {
		for (Expression* expression : expressions) {
			Data* a = expression->evaluate();
			if (expression->type == RETURN_BLOCK)
				return a;
		}
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Return Block: " << endl;
		for (Expression* expression : expressions) {
			expression->print(depth + 1);
		}
	}
};

class Declaration : public Statement {
public:
	string identifier;
	DataType type;
	Declaration(string identifier, DataType type) : identifier(identifier), type(type), Statement(DECLARATION) {}
	void execute() {
		if (variables.find(identifier) != variables.end()) {
			cerr << "Decleration::Execute Error: variable " << identifier << " already declared" << endl;
			exit(1);
		}
		Data* data = createDataFromType(type);
		variables[identifier] = data;
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Declaration: " << identifier << " Type: " << type << endl;
	}
};

class Assignment : public Statement {
public:
	MemberList identifier;
	Expression* expression;
	Assignment(MemberList identifier, Expression* expression) : identifier(identifier), expression(expression), Statement(ASSIGNMENT) {
	}
	void execute() {
		Data* data = identifier.get();
		Data* expressionData = expression->evaluate();
		if (data->type != expressionData->type) {
			cerr << "Assignment::Execute Error: expected type " << data->type << " but got " << expressionData->type << endl;
			exit(1);
		}
		if (data->data != nullptr)
			delete data->data;
		data->data = expressionData->data;
	}

	void print(int depth) override {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Assignment: " << endl;
		for(int i = 0; i < depth + 1;i++)
			cout << "  ";
		cout << "Identifier: ";
		for (string member : identifier.members) {
			cout << member << ".";
		}
		cout << endl;
		expression->print(depth + 1);
	}
};

class StructDecleration: public Statement {
public:
	string name;
	map<string, DataType> fields;
	StructDecleration(string name, map<string, DataType> fields) : name(name), fields(fields), Statement(STRUCT_DECLARATION) {}
	void execute() {
		StructData data = { name, fields };
	}

	void print(int depth) {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Struct Decleration: " << name << endl;
		for (pair<string, DataType> field : fields) {
			for (int i = 0; i < depth + 1; i++)
				cout << "  ";
			cout << "Field: " << field.first << " Type: " << field.second << endl;
		}
	}
};

class ParameterList {
public:
	vector<pair<string, DataType>> params;
	ParameterList(vector<pair<string, DataType>> params) : params(params) {};
	ParameterList() {}
	void print() {
		cout << "Parameter List:" << endl;
		for (pair<string, DataType> param : params) {
			cout << "Param: " << param.first << " Type: " << param.second << endl;
		}
	}
};

class Callable {
public:
	virtual Data* call(vector<Data*> params) = 0;
};

class Function : public Callable{
public:
	ReturnBlock* block;
	ParameterList list;
	DataType returnType;
	Function(Block* block, ParameterList list, DataType returnType) : list(list), returnType(returnType) {
		this->block = new ReturnBlock(block);
	}
	Function() {}
	Data* call(vector<Data*> params) {
		for (int i = 0; i < params.size(); i++) {
			variables[list.params[i].first] = params[i];
		}
		return block->evaluate();
	}
};

map<string, Callable*> functions;

void addFunctionName(string name) {
	functions[name] = nullptr;
}

void addFunction(string name, Function* function) {
	functions[name] = function;
}

class FunctionDecleration : public Statement {
public:
	string name;
	Block* block;
	ParameterList list;
	DataType returnType;
	FunctionDecleration(string name, Block* block, ParameterList list, DataType returnType) : Statement(FUNCTION_DECLARATION), name(name), block(block), list(list), returnType(returnType) {}
	void execute() {
		Function* function = new Function( block,list, returnType );
		functions[name] = function;
	}

	void print(int depth) {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Function Decleration: " << name << " Return Type: " << returnType << endl;
		block->print(depth + 1);

	}
};

class FunctionCall : public Expression {
public:
	string functionName;
	vector<Expression*> params;
	FunctionCall(string functionName, vector<Expression*> params) : Expression(FUNCTION_CALL),functionName(functionName), params(params) {};
	Data* evaluate() {
		vector<Data*> paramData;
		for (Expression* param : params) {
			paramData.push_back(param->evaluate());
		}
		Callable* function = functions[functionName];
		return function->call(paramData);
	}

	void print(int depth) {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "Function Call: " << functionName << " Params:" << endl;
		for (Expression* param : params) {
			param->print(depth + 1);
		}
	}
};

class IfStatement : public Statement {
public:
	Expression* condition;
	Block* ifBlock;
	Block* elseBlock;
	IfStatement(Expression* condition, Block* ifBlock, Block* elseBlock) : Statement(IF_STATEMENT), condition(condition), ifBlock(ifBlock), elseBlock(elseBlock) {}
	void execute() {
		Data* conditionData = condition->evaluate();
		string conditionStr = DataToString(*conditionData);
		if (conditionData->type != BOOL) {
			cerr << "Error: expected bool in if statement condition but got " << conditionData->type << endl;
			exit(1);
		}
		if (*(bool*)conditionData->data) {
			ifBlock->execute();
		}
		else {
			if (elseBlock != nullptr)
				elseBlock->execute();
		}
	}

	void print(int depth) {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "If Statement: " << endl;
		condition->print(depth + 1);
		ifBlock->print(depth + 1);
		if (elseBlock != nullptr)
			elseBlock->print(depth + 1);
	}
};

class WhileStatement : public Statement {
public:
	Expression* condition;
	Block* block;
	WhileStatement(Expression* condition, Block* block) : Statement(WHILE_STATEMENT), condition(condition), block(block) {}
	void execute() override {
		Data* conditionData = condition->evaluate();
		if (conditionData->type != BOOL) {
			cerr << "Error: expected bool in while statement condition but got " << conditionData->type << endl;
			exit(1);
		}
		while (*(bool*)conditionData->data) {
			block->execute();
			conditionData = condition->evaluate();
		}
	}

	void print(int depth) {
		for (int i = 0; i < depth; i++)
			cout << "  ";
		cout << "While Statement: " << endl;
		condition->print(depth + 1);
		block->print(depth + 1);
	}
};

vector<vector<Token>> splitOnTokenType(vector<Token> tokens, TokenType type) {
	vector<vector<Token>> acc;
	vector<Token> current;
	for (Token token : tokens) {
		if (token.type == type) {
			acc.push_back(current);
			current.clear();
		}
		else {
			current.push_back(token);
		}
	}
	acc.push_back(current);
	return acc;
};

// Expects tokens[i].type == OPEN_PAR
ParameterList parseParameterList(vector<Token> tokens, int& i) {
	if (tokens[i].type != OPEN_PAR) {
		cerr << "Error: expected open parenthesis when parsing parameterList" << endl;
		exit(1);
	}

	vector<Token> acc;
	for (i++; i < tokens.size(); i++) {
		if (tokens[i].type == CLOSE_PAR) {
			break;
		}
		acc.push_back(tokens[i]);
	}
	vector<pair<string, DataType>> params;
	vector<vector<Token>> split = splitOnTokenType(acc, DELIMITER);
	for (vector<Token> param : split) {
		if (param.size() == 0)
			continue;
		if (param.size() != 2) {
			cerr << "Error: invalid parameter declaration size should be two but is:" << param.size() << endl;
			exit(1);
		}
		if (param[0].type != IDENTIFIER || param[1].type != IDENTIFIER) {
			cerr << "Error: invalid parameter declaration" << endl;
			exit(1);
		}
		cout << "Param: " << param[0].value << " Type: " << param[1].value << endl;
		params.push_back({ param[1].value, types[param[0].value] });
	}
	ParameterList list(params);
	list.print();
	return list;
}

// Expects tokens[i].type == IDENTIFIER
MemberList parseMemberList(vector<Token> tokens, int& i) {
	if (tokens[i].type != IDENTIFIER) {
		cerr << "Error: expected identifier when parsing member list\n";
		cerr << "Given token:" << tokens[i] << endl;
		exit(1);
	}
	vector<string> members;
	for (i; i < tokens.size(); i++) {
		Token token = tokens[i];
		if (token.type == IDENTIFIER) {
			members.push_back(token.value);
			if (i + 1 >= tokens.size())
				return MemberList(members);
			Token next = tokens[++i];
			if (next.type != MEMBER_ACCESS)
				break;
		}
	}
	i--;
	return MemberList(members);
}

Block* parse(vector<Token> tokens) {
	Block* block = new Block();
	vector<Statement*> statements;
	int lineNumber = 0;
	for (int i = 0; i < tokens.size(); i++) {
		Token token = tokens[i];
		if (token.type == END_OF_LINE) {
			lineNumber++;
			continue;
		}
		if (token.type == KEYWORD) {
			string keyword = token.value;
			if (keyword == "fun") {
				Token next = tokens[++i];
				if (next.type != IDENTIFIER) {
					cerr << "Error: expected identifier after function decleration on line:" << lineNumber << endl;
					exit(1);
				}
				string functionName = next.value;

			}
		}
	}
}

Block* parseBlock(vector<Token> tokens, int& i);

Expression* parseExpression(vector<Token> tokens, int& i);
// Expected first char is OPEN_PAR and parses until matching LAST_PAR
vector<Expression*> parseExpressionList(vector<Token> tokens, int& i){
	if (tokens[i].type != OPEN_PAR) {
		cerr << "Error: expected open parenthesis when parsing expression list" << endl;
		exit(1);
	}
	i++;
	vector<vector<Token>> split;
	vector<Token> current;
	int depth = 1;
	while (depth > 0) {
		if (tokens[i].type == OPEN_PAR) {
			depth++;
		}
		if (tokens[i].type == CLOSE_PAR) {
			depth--;
			if (depth == 0)
				break;
		}
		if (depth == 1 && tokens[i].type == DELIMITER) {
			split.push_back(current);
			current.clear();
			i++;
			continue;
		}
		current.push_back(tokens[i]);
		i++;
	}
	split.push_back(current);
	cout << "Parsing Expression List:" << endl;
	cout << "Split size: " << split.size() << endl;
	for (vector<Token> s : split) {
		for (Token t : s) {
			cout << t << endl;
		}
		cout << "----" << endl;
	}
	vector<Expression*> expressions;
	for (vector<Token> expression : split) {
		int k = 0;
		expressions.push_back(parseExpression(expression, k));
	}
	return expressions;
}

class OperatorHandeler {
private:
	Expression* left = nullptr;
	Expression* right = nullptr;
	string op = "";
public:
	OperatorHandeler() {};
	void addExpression(Expression* expr) {
		cout << "Adding expression: ";
		expr->print(0);
		if (left == nullptr) {
			left = expr;
		}
		else if (right == nullptr && op != "") {
			right = expr;
			Operator* oper = new Operator(op, left, right);
			left = oper;
			right = nullptr;
			op = "";
		}
		else {
			cerr << "Error: invalid operator handeler state\n";
			cerr << "Expression added without operator or with two expressions" << endl;
			exit(1);
		}
	}

	void addOperator(string op) {
		cout << "Setting operator: " << op << endl;
		if (this->op != "") {
			cerr << "Error: invalid operator handeler state\n";
			cerr << "Operator added without expression or with two operators" << endl;
			exit(1);
		}
		this->op = op;
	}

	Expression* getExpression() {
		return left;
	}

	void printState() {
		cout << "Operator Handeler State: " << endl;
		cout << "Left: ";
		if (left != nullptr)
			left->print(0);
		cout << "Right: ";
		if (right != nullptr)
			right->print(0);
		cout << "Operator: " << op << endl;
	}
};

Expression* parseExpression(vector<Token> tokens, int& i) {
	cout << "Parsing Expression" << endl;
	OperatorHandeler handeler;
	for (; i < tokens.size(); i++) {
		Token token = tokens[i];
		std::cout << "parseExpression::Token = " << token << endl;
		if (token.type == END_OF_LINE) {
			return handeler.getExpression();
		}
		if (token.type == IDENTIFIER) {
			cout << "parseExpression::Parsing identifier" << endl;
			if (tokens.size() > i + 1 && tokens[i+1].type == OPEN_PAR) {
				i++;
				vector<Expression*> params = parseExpressionList(tokens, i);
				string functionName = token.value;
				FunctionCall* call = new FunctionCall(functionName, params);
				handeler.addExpression(call);
			}
			else {
				MemberList member = parseMemberList(tokens, i);
				Variable* variable = new Variable(member);
				handeler.addExpression(variable);
			}
		}
		if (token.type == NUMBER) {
			Data* data = new Data();
			if (token.value.find(".") != string::npos) {
				data->type = FLOAT;
				float* f = new float(stof(token.value));
				data->data = f;
			}
			else {
				int* j = new int(stoi(token.value));
				data->data = j;
			}
			Literal* literal = new Literal(data);
			handeler.addExpression(literal);
		}
		if (token.type == OPEN_PAR) {
			int depth = 1;
			i++;
			vector<Token> acc;
			while (depth > 0) {
				if (tokens[i].type == OPEN_PAR) {
					depth++;
				}
				if (tokens[i].type == CLOSE_PAR) {
					depth--;
					if (depth == 0)
						break;
				}
				cout << "Adding token: " << tokens[i] << endl;
				acc.push_back(tokens[i]);
				i++;
			}
			int k = 0;
			Expression* expr = parseExpression(acc,k);
			handeler.addExpression(expr);
			cout << "Done parsing expression" << endl;
		}
		if (token.type == OP) {
			cout << token << endl;
			handeler.addOperator(token.value);
		}
		if (token.type == OPEN_BRACE) {
			i--;
			return handeler.getExpression();
		}
	}
	return handeler.getExpression();
}

void StructPass(vector<Token> tokens) {
	int lineNumber = 0;
	cout << "Starting Struct Pass" << endl;
	vector<pair<string, int>> dataBlocks;
	for (int i = 0; i < tokens.size(); i++) {
		Token token = tokens[i];
		if (token.type == END_OF_LINE) {
			lineNumber++;
			continue;
		}
		if (token.type == KEYWORD) {
			string keyword = token.value;
			cout << "Keyword: " << keyword << endl;
			if (keyword == "struct") {
				Token next = tokens[++i];
				if (next.type != IDENTIFIER) {
					cerr << "Error: expected identifier after struct decleration on line:" << lineNumber << endl;
					exit(1);
				}
				string structName = next.value;
				next = tokens[++i];
				if (next.type != OPEN_BRACE) {
					cerr << "Error: expected open brace after struct name on line:" << lineNumber << endl;
					exit(1);
				}
				dataBlocks.push_back({ structName, i });
				addDataType(structName);
			}
			else {
				continue;
			}
		}
	}
	cout << "Struct Pass Complete" << endl;
	cout << "Parsing Struct Data" << endl;

	vector<pair<string, Block*>> blocks;
	for (pair<string, int> dataBlock : dataBlocks) {
		string structName = dataBlock.first;
		int index = dataBlock.second;
		Block* block = parseBlock(tokens, index);
		blocks.push_back({ structName, block });
	}

	for (pair<string, Block*> dataBlock : blocks) {
		string structName = dataBlock.first;
		Block* block = dataBlock.second;
		vector<pair<string, DataType>> fields;
		for (Statement* statement : block->statements) {
			if (statement->type != DECLARATION) {
				cerr << "Error: expected decleration in struct block" << endl;
				exit(1);
			}
			Declaration* decleration = (Declaration*)statement;
			fields.push_back({ decleration->identifier, decleration->type });
		}
		map<string, DataType> fieldsMap;
		for (pair<string, DataType> field : fields) {
			fieldsMap[field.first] = field.second;
		}
		StructData data = { structName, fieldsMap };
		structs[types[structName]] = data;
	}
	cout << "Struct Data Parsed" << endl;
}

vector<FunctionDecleration*> FunctionPass(vector<Token> tokens) {
	cout << "Parsing functions:" << endl;
	vector<FunctionDecleration*> functions;
	int lineNumber = 0;
	for (int i = 0; i < tokens.size(); i++) {
		Token token = tokens[i];
		if (token.type == END_OF_LINE) {
			lineNumber++;
			continue;
		}
		if (token.type == KEYWORD) {
			cout << "Keyword: " << token.value << endl;
			string keyword = token.value;
			if (keyword == "fun") {
				Token next = tokens[++i];
				if (next.type != IDENTIFIER) {
					cerr << "Error: expected identifier after function decleration on line:" << lineNumber << endl;
					exit(1);
				}
				string functionName = next.value;
				Token nextToken = tokens[++i];
				if (nextToken.type != OPEN_PAR) {
					cerr << "Error: expected open parenthesis after function name on line:" << lineNumber << endl;
					exit(1);
				}
				ParameterList list = parseParameterList(tokens, i);
				nextToken = tokens[++i];
				if (nextToken.type != OP && nextToken.value != "->") {
					cerr << "Error: expected -> after parameter list on line:" << lineNumber << endl;
					exit(1);
				}
				nextToken = tokens[++i];
				if (nextToken.type != IDENTIFIER) {
					cerr << "Error: expected return type after -> on line:" << lineNumber << endl;
					exit(1);
				}
				string returnType = nextToken.value;
				DataType type = types[returnType];
				nextToken = tokens[++i];
				if (nextToken.type != OPEN_BRACE) {
					cerr << "Error: expected open brace after return type on line:" << lineNumber << endl;
					exit(1);
				}
				Block* block = parseBlock(tokens, i);
				FunctionDecleration* function = new FunctionDecleration(functionName, block, list, type);
				functions.push_back(function);
			}
		}
	}
	cout << "Function Pass Complete" << endl;
	return functions;
}

vector<Statement*> parseStatement(vector<Token> t, int& i) {
	vector<Statement*> statements;
	for (;i < t.size(); i++) {
		Token first = t[i];
		cout << "parseStatement::Token: " << first << endl;
		if (t.size() > i + 1)
			cout << "parseStatement::Next Token: " << t[i + 1] << endl;
		if (first.type == IDENTIFIER) {
			cout << "parseStatment::Parsing identifier" << endl;
			bool isType = types.find(first.value) != types.end();
			if (isType) {
				DataType type = types[first.value];
				Token next = t[++i];
				if (next.type != IDENTIFIER) {
					cerr << "Error: expected identifier after type on line:" << i << endl;
					exit(1);
				}
				string identifier = next.value;
				next = t[++i];
				Declaration* decleration = new Declaration(identifier, type);
				statements.push_back(decleration);
				if (next.type == END_OF_LINE) {
					cout << "parseStatement::Returning decleration statement" << endl;
					return statements;
				}
				else if (next.type == ASSIGNMENT_OPERATOR) {
					cout << "parseStatement::Parsing assignment statement" << endl;
					string op = next.value;
					Expression* expression = parseExpression(t, ++i);
					MemberList member = MemberList(identifier);
					Assignment* assignment = new Assignment(member, expression);
					statements.push_back(assignment);
					cout << "parseStatemenet::Returning assignment statement" << endl;
				}
				else {
					cerr << "Error: expected assignment operator or end of line after decleration\n";
					cerr << "Instead got: " << next << endl;
					exit(1);
				}
				return statements;
			}
			else {
				MemberList member = parseMemberList(t, i);
				// If there is an identifier by itself it must be a function call or assignment
				Token next = t[++i];
				if (next.type == ASSIGNMENT_OPERATOR) {
					Expression* expression = parseExpression(t, ++i);
					Assignment* assignment = new Assignment(member, expression);
					statements.push_back(assignment);
				}
				else if (next.type == OPEN_PAR) {
					cout << "we must be parsing an expr list\n";
					/*vector<Expression*> params;
					while (t[i].type != CLOSE_PAR) {
						Expression* param = parseExpression(t, i);
						params.push_back(param);
					}*/
					vector<Expression*> params = parseExpressionList(t, i);
					FunctionCall* call = new FunctionCall(first.value, params);
					ExpressionWrapper* wrapper = new ExpressionWrapper(call);
					statements.push_back(wrapper);
				}
				else {
					cerr << "Error: expected assignment operator or open parenthesis after identifier\n";
					cerr << "Instead got: " << next << endl;

					exit(1);
				}

			}
		}
		if (first.type == KEYWORD) {
			if (first.value == "return") {
				Expression* expression = parseExpression(t, ++i);
				Return* returnStatement = new Return(expression);
				ExpressionWrapper* wrapper = new ExpressionWrapper(returnStatement);
				statements.push_back(wrapper);
				return statements;
			}
			if (first.value == "if") {
				Token next = t[++i];
				Expression* condition = parseExpression(t,i);
				next = t[++i];
				if (next.type != OPEN_BRACE) {
					cerr << "Error: expected open brace after if condition on line:" << i << endl;
					exit(1);
				}
				Block* ifBlock = parseBlock(t, i);
				Block* elseBlock = nullptr;
				next = t[++i];
				if (next.type == KEYWORD && next.value == "else") {
					next = t[++i];
					if (next.type != OPEN_BRACE) {
						cerr << "Error: expected open brace after else" << endl;
						exit(1);
					}
					elseBlock = parseBlock(t, i);
				}
				IfStatement* ifStatement = new IfStatement(condition, ifBlock, elseBlock);
				statements.push_back(ifStatement);
				return statements;
			}
			if (first.value == "while") {
				Token next = t[++i];
				Expression* condition = parseExpression(t, i);
				next = t[++i];
				if (next.type != OPEN_BRACE) {
					cerr << "Error: expected open brace after while condition on line:" << i << endl;
					exit(1);
				}
				Block* block = parseBlock(t, i);
				WhileStatement* whileStatement = new WhileStatement(condition, block);
				statements.push_back(whileStatement);
				return statements;
			}
		}
	}
	return statements;
}
// Expects tokens[i].type == openBrace
Block* parseBlock(vector<Token> tokens, int& i) {
	cout << "Parsing block" << endl;
	if (tokens[i].type != OPEN_BRACE) {
		cerr << "Error: expected open brace when parsing block" << endl;
		exit(1);
	}

	vector<Token> acc;
	int depth = 1;
	for (i++; i < tokens.size(); i++) {
		if (tokens[i].type == OPEN_BRACE) {
			depth++;
		}
		if (tokens[i].type == CLOSE_BRACE) {
			depth--;
			if (depth == 0) {
				break;
			}
		}
		acc.push_back(tokens[i]);
	}

	vector<Statement*> statements;
	int j = 0;
	while (j < acc.size()) {
		vector<Statement*> statement = parseStatement(acc, j);
		statements.insert(statements.end(), statement.begin(), statement.end());
	}
	Block* block = new Block();
	block->statements = statements;
	return block;
}

void PrintStructData() {
	cout << "Printing Struct Data" << endl;
	for (pair<string, DataType> s : types) {
		cout << "Struct: " << s.first << endl;
		StructData data = structs[s.second];
		for (pair<string, DataType> field : data.fields) {
			cout << "Field: " << field.first << " Type: " << field.second << endl;
		}
	}
}

class Print : public Callable {
public:
	Data* call(vector<Data*> params) {
		string str = "";
		for (Data* param : params) {
			str += DataToString(*param);
			str += " ";
		}
		cout << str;
		return new Data{ NULL_TYPE,nullptr };
	}
};

class Println : public Callable {
public:
	Data* call(vector<Data*> params) {
		string str = "";
		for (Data* param : params) {
			str += DataToString(*param);
			str += " ";
		}
		cout << str;
		cout << endl;
		return new Data{ NULL_TYPE,nullptr };
	}
};

void AddDefaultFunctions() {
	functions["print"] = new Print();
	functions["println"] = new Println();
}

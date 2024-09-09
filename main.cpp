#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include "util.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"
using namespace std;


int main() {
	vector<string> lines = readLinesFromFile("expressions.pys");
	vector<Token> tokens = tokenize(lines);
	for (auto token : tokens) {
		cout << token << endl;
	}
	StructPass(tokens);
	PrintStructData();
	vector<FunctionDecleration*> funcs = FunctionPass(tokens);
	AddDefaultFunctions();

	cout << "Printing functions" << endl;
	for (auto func : funcs) {
		func->print(0);
		func->execute();
	}
	if (functions.find("main") == functions.end()) {
		cerr << "Error: no main function found" << endl;
		return 1;
	}
	cout << "Running main function" << endl;
	Data* result = functions["main"]->call({});
	cout << "Result: " << DataToString(*result);
}

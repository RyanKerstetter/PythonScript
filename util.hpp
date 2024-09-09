#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
using namespace std;


vector<string> readLinesFromFile(string filename) {
	vector<string> lines;
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error: could not open file " << filename << endl;
		return lines;
	}
	string line;
	while (getline(file, line)) {
		lines.push_back(line);
	}
	file.close();
	return lines;
}

bool isWhiteSpace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isNumeric(char c) {
	return c >= '0' && c <= '9' || c == '.' || c == '-';
}


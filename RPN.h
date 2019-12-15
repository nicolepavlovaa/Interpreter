#ifndef __RPN_H
#define __RPN_H
#include <iostream>
#include <algorithm> 
#include <stack>
#include <string>
#include <list>
#include <stdexcept>
using namespace std;

class RPN
{
	stack<int> results;
	stack<char> ops;

	bool isDigit(char c) const { return '0' <= c && c <= '9'; }
	int digitValue(char c) const { return c - '0'; }
	int apply(int larg, int rarg, char op);
	void popAndApply(char op);
	int priority(char op) const;

public:
	string stringToRPN(string input);
	int calculateRPN(string RPNstring);
};
#endif
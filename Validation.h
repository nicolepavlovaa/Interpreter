#ifndef __VALIDATION_H
#define __VALIDATION_H
#include "RPN.h"
#include "Container.h"
class ValidationTree
{
public:
	bool valid = false;
public:
	
	ValidationTree(string line, Container& memory);
	bool validateNum(string num);
	bool validateVar(string var);
	bool validateFun(string fun);
	bool validateTerm(string term);
	bool validateFactor(string factor);
	bool validateExpr(string expr);
	bool validateLine(string line, Container& memory);

	void findAndReplaceAll(string & data, string toSearch, string replaceStr, Container& momory);
	int getVariableValue(string& key, Container& memory);
	bool isOp(char a);
	void divideAndReplace(string body, list<string>& funContainer, list<string>& argContainer);
	int CalculateBodyofFunction(string body, Container& memory);
	int CalculateFunction(string key,string argument, Container& memory);
};

#endif
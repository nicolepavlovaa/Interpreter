#include "RPN.h"

int RPN::apply(int larg, int rarg, char op)
{
	if (op == '+') return larg + rarg;
	if (op == '-') return larg - rarg;
	if (op == '*') return larg * rarg;
	if (op == '/' && rarg != 0) return larg / rarg;
	if (op == '%') return larg % rarg;
	if (op == '/' && rarg == 0) throw invalid_argument("Attempted to divide by Zero.");
}

void RPN::popAndApply(char op)
{
	double rarg = results.top();
	results.pop();
	double larg = results.top();
	results.pop();
	try
	{
		results.push(apply(larg, rarg, op));
	}
	catch (const std::invalid_argument &err)
	{
		throw std::invalid_argument(err.what());
	}
}
int RPN::priority(char op) const {
	switch (op) {
	case '+':
	case '-': return 1;
	case '*':
	case '/': return 2;
	case '%': return 2;
	default: return 0;
	}
}
string removeSpaces(string str)
{
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}
string RPN::stringToRPN(string expr)
{
	expr = removeSpaces(expr);
	string rpn;
	for (char c : expr) 
	{
		if (isDigit(c))
			rpn.push_back(c);
		else if (c == '(')
			ops.push(c);
		else if (c != ')') 
		{
			// избутваме операциите с >= приоритет
			while (!ops.empty() && priority(ops.top()) >= priority(c)) 
			{
				rpn.push_back(' ');
				rpn.push_back(ops.top());
				ops.pop();
			}
			ops.push(c);
			rpn.push_back(' ');
		}
		else
		{
			// c == ')'
			while (ops.top() != '(')
			{
				rpn.push_back(' ');
				rpn.push_back(ops.top());
				ops.pop();
			}
			if (ops.top() == '(') ops.pop();
		}
	}
	while (!ops.empty())
	{
		rpn.push_back(' ');
		rpn.push_back(ops.top());
		ops.pop();
	}
	return rpn;
}

int RPN::calculateRPN(string rpn)
{
	bool flag=false;
	string currNum = "";
	for (int i = 0; i < rpn.length(); i++)
	{
		if (rpn[i] == '/' || rpn[i] == '*' || rpn[i] == '+' || rpn[i] == '-' || rpn[i] == '%')
		{
			flag = true;
		}
	}
	for (int i = 0; i < rpn.length(); i++)
	{
		while (isDigit(rpn[i]))
		{
			currNum += rpn[i];
			i++;
		}
		if (currNum != "")
		{
			results.push(stoi(currNum));
			currNum.clear();
		}
		if (!isDigit(rpn[i]) && rpn[i] != ' ' && flag)
		{
			try
			{
				popAndApply(rpn[i]);
			}
			catch (const std::invalid_argument &err)
			{
				throw std::invalid_argument(err.what());
			}
		}
	}
	return results.top();
}
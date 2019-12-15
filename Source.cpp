#include "Validation.h"

bool ValidationTree::isOp(char a)
{
	return (a == '+' || a == '-' || a == '*' ||
		a == '/' || a == '%');
}

void ValidationTree::divideAndReplace(string body, list<string>& funContainer, list<string>& argContainer)
{
	int counter = 0;
	int pos = -1;
	string op = "";

	for (int i = 0; i < body.length(); i++)
	{
		if (body[i] == '[') counter++;
		if (body[i] == ']') counter--;
		if (isOp(body[i]) && counter == 0)
		{
			pos = i;
			op = string(" ") + body[i] + string(" ");
			break;
		}
	}
	regex fn("[A-Z]+\\[.+\\]");
	string token1 = "";
	string token2 = "";
	if (pos != -1)
	{
		token1 = body.substr(0, pos - 1);
		token2 = body.substr(pos + 2);
	}

	if (pos == -1 && !(regex_match(body, fn))) return;
	if (pos == -1 && (regex_match(body, fn)))
	{
		unsigned first = body.find('[');
		unsigned second = body.rfind(']');
		string arg = body.substr(first + 1, second - first - 1);
		string fun = body.substr(0, body.find('['));

		funContainer.push_back(fun);
		argContainer.push_back(arg);
		if (!(regex_match(arg, fn))) return;
		else divideAndReplace(arg, funContainer, argContainer);
	}
	else if (regex_match(token1, fn) && pos != -1)
	{
		unsigned first = token1.find('[');
		unsigned second = token1.rfind(']');
		string arg = token1.substr(first + 1, second - first - 1);
		string fun = token1.substr(0, token1.find('['));

		funContainer.push_back(fun);
		argContainer.push_back(arg);
		divideAndReplace(arg, funContainer, argContainer);
	}
	divideAndReplace(token2, funContainer, argContainer);

}
int ValidationTree::CalculateBodyofFunction(string body, Container& memory)
{
	list<string> varContainer;

	regex var_reg("[a-z]+");
	sregex_iterator iter(body.begin(), body.end(), var_reg);
	sregex_iterator end;

	while (iter != end)
	{
		for (unsigned i = 0; i < iter->size(); ++i)
		{
			varContainer.push_back((*iter)[i]);
		}
		++iter;
	}
	while (!varContainer.empty())
	{
		string currVar = varContainer.front();
		string replacement = "";
		//exception
		try
		{
			replacement = to_string(getVariableValue(currVar, memory));
		}
		catch (const std::invalid_argument &err)
		{
			throw std::invalid_argument(err.what());
		}
		//
		findAndReplaceAll(body, varContainer.front(), replacement, memory);
		varContainer.pop_front();
	}
	//functions part
	list<string> funContainer;
	list<string> argContainer;

	divideAndReplace(body, funContainer, argContainer);
	while (!funContainer.empty())
	{
		string currFun = funContainer.front();
		string currArg = argContainer.front();
		string currWhole = currFun + "[" + currArg + "]";
		//exception
		string replacement = "";
		try
		{
			replacement = to_string(CalculateFunction(currFun, currArg, memory));
		}
		catch (const std::invalid_argument &err)
		{
			throw std::invalid_argument(err.what());
		}
		//
		findAndReplaceAll(body, currWhole, replacement, memory);
		funContainer.pop_front();
		argContainer.pop_front();
	}
	int result = 0;
	try
	{
		result = RPN().calculateRPN(RPN().stringToRPN(body));
	}
	catch (const std::invalid_argument &err)
	{
		throw std::invalid_argument(err.what());
	}
	return result;
	
}

int ValidationTree::CalculateFunction(string key,string argument, Container & memory)
{
	string funBody = "";
	string varToReplace = "";
	try
	{
		funBody = memory.functions.at(key).second;
		varToReplace = memory.functions.at(key).first;
	}
	catch (const std::out_of_range &arr)
	{
		throw std::invalid_argument("Undefined function.");
	}

	findAndReplaceAll(funBody, varToReplace, argument, memory);
	int result = 0;
	try
	{
		result = CalculateBodyofFunction(funBody, memory);
	}
	catch (const std::invalid_argument &err)
	{
		throw std::invalid_argument(err.what());
	}
	return result;
}

ValidationTree::ValidationTree(string line, Container& memory)
{
	try
	{
		valid=validateLine(line, memory);
	}
	catch (const std::invalid_argument &read)
	{
		throw std::invalid_argument(read.what());
	}
}

bool ValidationTree::validateVar(string var)
{
	//cout << "var: " << var << endl;
	regex var_reg("^[a-z]+");
	return regex_match(var, var_reg);
}
bool ValidationTree::validateNum(string num)
{
	//cout << "num: " << num << endl;
	regex num_reg("([1-9]+[0-9]*|0)");
	return regex_match(num, num_reg);
}
bool ValidationTree::validateFactor(string factor)
{
	//cout << "factor: " << factor << endl;
	regex factor_reg1("^\\(.+\\)"); //factor->(Expr)
	regex factor_reg2("^(.+\\[.+\\])"); //factor->Fun[Expr]
	//3
	if (regex_match(factor, factor_reg1))
	{
		string token = factor.substr(1, factor.rfind(')') - 1);
		if (validateExpr(token)) return true;
	}
	//4
	else if (regex_match(factor, factor_reg2))
	{
		string token1 = factor.substr(0, factor.find('['));
		unsigned first = factor.find('[');
		unsigned second = factor.rfind(']');
		string token2 = factor.substr(first + 1, second - first - 1);
		if (validateFun(token1) && validateExpr(token2)) return true;
	}
	//1,2
	else if (validateVar(factor)) //factor->var
	{
		return true; 
	}
	else if (validateNum(factor)) //factor-> num
	{
		return true;
	}
	return false;
}
bool ValidationTree::validateTerm(string term)
{
	//cout << "term: " << term << endl;
	regex term1_reg("^(.+)");//term->factor
	regex term2_reg("^((.+) \\* (.+))"); //term->term * factor
	regex term3_reg("^((.+) \/ (.+))"); //term->term / factor
	regex term4_reg("^((.+) % (.+))"); //term->term % factor

	//utils

	int multPos = -1;
	int divPos = -1;
	int percPos = -1;
	int counter = 0;
	int counter2 = 0;

	for (int i = 0; i < term.length(); i++)
	{
		if (term[i] == '[') counter++;
		if (term[i] == ']') counter--;
		if (term[i] == '(') counter2++;
		if (term[i] == ')') counter2--;
		if (term[i] == '*' && counter == 0 && counter2 == 0) multPos = i;
		if (term[i] == '/' && counter == 0 && counter2 == 0) divPos = i;
		if (term[i] == '%' && counter == 0 && counter2 == 0) percPos = i;
	}
	//2
	if (regex_match(term, term2_reg) && multPos>divPos && multPos>percPos)
	{
		string token1 = term.substr(0, multPos - 1);
		string token2 = term.substr(multPos + 2);

		if (validateTerm(token1) && validateFactor(token2)) return true;
	}
	//3
	else if (regex_match(term, term3_reg) && divPos>multPos && divPos>percPos)
	{
		string token1 = term.substr(0, divPos - 1);
		string token2 = term.substr(divPos + 2);

		if (validateTerm(token1) && validateFactor(token2)) return true;
	}
	//4
	else if (regex_match(term, term4_reg) && percPos>multPos && percPos>divPos)
	{
		string token1 = term.substr(0, percPos - 1);
		string token2 = term.substr(percPos + 2);

		if (validateTerm(token1) && validateFactor(token2)) return true;
	}
	//1
	else if (regex_match(term, term1_reg))
	{
		if (validateFactor(term)) return true;
	}
	return false;
}
bool ValidationTree::validateExpr(string expr)
{
	//cout << "expr: " << expr << endl;
	regex expr1_reg("^(.+)"); //expr->term
	regex expr2_reg("^(.+ - .+)"); //expr-> expr - term
	regex expr3_reg("^(.+ \\+ .+)"); //expr-> expr + term   

	int minPos = -1;
	int plusPos = -1;
	int counter = 0;
	int counter2 = 0;
	
	for (int i = 0; i < expr.length(); i++)
	{
		if (expr[i] == '[') counter++;
		if (expr[i] == ']') counter--;
		if (expr[i] == '(') counter2++;
		if (expr[i] == ')') counter2--;
		if (expr[i] == '-' && counter == 0 && counter2 == 0) minPos = i;
		if (expr[i] == '+' && counter == 0 && counter2 == 0) plusPos = i;
	}
	//2
	if (regex_match(expr, expr2_reg) && minPos>plusPos)
	{
		string token1 = expr.substr(0, minPos - 1);
		string token2 = expr.substr(minPos + 2);
		
		if (validateExpr(token1) && validateTerm(token2)) return true;
	}
	//3
	else if (regex_match(expr, expr3_reg) && plusPos>minPos)
	{
		string token1 = expr.substr(0, plusPos -1);
		string token2 = expr.substr(plusPos + 2);
		
		if (validateExpr(token1) && validateTerm(token2)) return true;
	}
	//1
	else if (regex_match(expr, expr1_reg))
	{
		if (validateTerm(expr)) return true;
	}
	return false;
}
bool ValidationTree::validateFun(string fun)
{
	regex fun_reg("^([A-Z]+)");
	return regex_match(fun, fun_reg);
}
bool ValidationTree::validateLine(string line, Container& memory)
{
	regex line1_reg("^(.+ = .+)"); //line-> var = expr
	regex line2_reg("^(.+\\[.+\\] = .+)"); //line-> fun[var] = expr
	regex line3_reg("^(read .+)"); //line-> read var
	regex line4_reg("^(print .+)"); //line-> print expr
	//cout << "line: " << line << endl;
	//1
	if (regex_match(line, line1_reg))
	{
		string delimiter = " = ";
		string token1 = line.substr(0, line.find(delimiter));
		string token2 = line.substr(line.find(delimiter) + 3);
		if (validateVar(token1) && validateExpr(token2))
		{
			//cout << token2 << endl;
			int n = 0;
			try
			{
				n = CalculateBodyofFunction(token2, memory);
			}
			catch (const std::invalid_argument &err)
			{
				throw std::invalid_argument(err.what());
			}
			memory.variables.emplace(token1, n);
			return true;
		}
	}
	//2
	if (regex_match(line, line2_reg))
	{
		string delimiter = " = ";
		string token1 = line.substr(0, line.find(delimiter));
		string token2 = line.substr(line.find(delimiter) + 3);
		unsigned first = line.find('[');
		unsigned second = line.find(']');
		string var = line.substr(first + 1, second - first - 1);
		string fun = line.substr(0, line.find('['));
		if (validateFun(fun) && validateVar(var) && validateExpr(token2))
		{
			memory.functions.emplace(fun, make_pair(var, token2));
			return true;
		}
	}
	//3
	if (regex_match(line, line3_reg))
	{
		string token = line.substr(line.rfind(' ') + 1);
		if (validateVar(token))
		{
			int value;
			cin >> value;
			memory.variables.emplace(token, value);
			return true;
		}
		//throw exception
		else throw std::invalid_argument("Invalid argument.");
	}
	//4
	if (regex_match(line, line4_reg))
	{
		string token = line.substr(line.rfind(' ') + 1);
		if (validateExpr(token))
		{
			int n = 0;
			try
			{
				n = CalculateBodyofFunction(token, memory);
			}
			catch (const std::invalid_argument &err)
			{
				throw std::invalid_argument(err.what());
			}
			cout << n << endl;
			return true;
		}
	}
	return false;
}

void ValidationTree::findAndReplaceAll(string & data, string toSearch, string replaceStr, Container& memory)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}

int ValidationTree::getVariableValue(string & key, Container & memory)
{
	int result;
	map<string, int>::iterator it;
	it = memory.variables.find(key);
	if (it != memory.variables.end()) result = memory.variables.find(key)->second;
	//exception
	else throw std::invalid_argument("Undefined variable.");
	//
	return result;
}
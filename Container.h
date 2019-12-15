#ifndef __CONTAINER_H
#define __CONTAINER_H
#include <map>
#include <regex>
#include "RPN.h"

class Container
{
public:
	map<string, int> variables;
	map<string, pair<string,string>> functions;
};
#endif
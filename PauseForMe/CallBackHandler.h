#pragma once
#pragma warning(disable: 4267)

#include <string>

using namespace std;

class CallBackHandler
{
public:
	CallBackHandler();
	~CallBackHandler();
	void acceptMessage(string origin, string message);
	string getCommand();
	string getOrigin();
	void commandExecuted();
private:
	string command;
	string origin;
	int executed;
};

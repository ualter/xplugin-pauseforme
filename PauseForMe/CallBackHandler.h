#pragma once
#pragma warning(disable: 4267)

#include <string>

using namespace std;

class CallBackHandler
{
public:
	CallBackHandler();
	~CallBackHandler();
	void acceptMessage(string message);
	string getCommand();
	void commandExecuted();
private:
	string command;
	int executed;
};

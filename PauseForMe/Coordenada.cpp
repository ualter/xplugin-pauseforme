#include <sstream>
#include "Coordenada.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std;

Coordenada::Coordenada(int dec) {
	decimals = dec;
}

int Coordenada::getIntPart(void) {
	return intPart;
}
int Coordenada::getDecPart(void) {
	return decPart;
}
double Coordenada::getValor(void) {
	return valor;
}
string Coordenada::getValorStr(void) {
	string result = valorStr;
	if (valorStr.find(".")) {
		result = valorStr.substr(0,valorStr.find(".")+(decimals+1));
	} else {
		result = valorStr;
	}
	return result;
}
void Coordenada::setValor(double vlr) {
	valor = vlr;
	calculate();
}
void Coordenada::setValor(string strVlr) {
	valor = atof(strVlr.c_str());
	calculate();
}

std::string convertDoubleToString(double number)
{
    std::ostringstream ostr;
    ostr << std::fixed << std::setprecision(6) << number;
    std::string str = ostr.str();
    return str;
}

void Coordenada::calculate(void) {
    string strDouble = convertDoubleToString(valor);
	valorStr = strDouble;
	string strDoubleIntpart, strDoubleDecpart;
	if (strDouble.find(".")) {
		strDoubleIntpart = strDouble.substr(0, strDouble.find("."));
		strDoubleDecpart = strDouble.substr(strDouble.find(".") + 1);
		intPart = atoi(strDoubleIntpart.c_str());
		if (strDoubleDecpart.length() > 4) {
			decPart = atoi(strDoubleDecpart.substr(0, decimals).c_str());
		}
		else {
			decPart = atoi(strDoubleDecpart.c_str());
		}
	}
	if (intPart < 0) intPart *= -1;
	if (decPart < 0) decPart *= -1;
}
int Coordenada::compare(Coordenada otherCoordenada, int acceptableDifference) {
	int result = 0;
	if (intPart == otherCoordenada.getIntPart()) {
		if (decPart == otherCoordenada.getDecPart()) {
			result = 1;
		}
		else {
			int diffDecPart = decPart - otherCoordenada.getDecPart();
			if (diffDecPart < 0) diffDecPart *= -1;
			if (diffDecPart <= acceptableDifference) {
				result = 1;
			}
		}
	}
	return result;
}

Coordenada::~Coordenada()
{
}

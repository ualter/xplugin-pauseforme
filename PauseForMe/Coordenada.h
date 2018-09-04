#pragma once
using namespace std;
class Coordenada
{
public:
	int getIntPart(void);
	int getDecPart(void);
	double getValor(void);
	string getValorStr(void);
	void setValor(double vlr);
	void setValor(string strVlr);
	int compare(Coordenada otherCoordenada, int acceptableDifference);
	Coordenada(int decimals);
	~Coordenada();

private:
	string valorStr;
	double valor;
	int intPart;
	int decPart;
	int decimals;
	void calculate(void);

};
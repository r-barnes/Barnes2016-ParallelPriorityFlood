#include <iostream>
#include <fstream>
using namespace std;
int main(){
  
  	ifstream fin("flow.hgt");
	ifstream fin2("CacheAwareAccumulation.hgt");
	ofstream saida("Erro.hgt");
	for(int i=0;i<40000;i++) {
		for(int j=0;j<40000;j++) {
			short int in1;
			fin.read(reinterpret_cast<char *>(&in1),2);
			short int in2;
			fin2.read(reinterpret_cast<char *>(&in2),2);
                        in2 = in2 -in1;
			saida.write(reinterpret_cast<char *>(&in2),2);
		}
	}
  
  
  
  
}

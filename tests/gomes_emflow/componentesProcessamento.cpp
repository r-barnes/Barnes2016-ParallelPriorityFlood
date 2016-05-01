/** EMFlow 1.0. Date of release: July 25th, 2013. 
 *
 * 
 * Copyright (C) 2013 Thiago L. Gomes, Salles V. G. Magalhães, Marcus V. A. Andrade, W. Randolph Franklin, 
 * and Guilherme C. Pena.
 * 
 * This program is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * You should have received a copy of the GNU General Public License along 
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * EMFlow - Compute the drainage network on digital elevation models
 * stored in external memory. This algorithm has been 
 * specially designed to minimize I/O operations.
 * 
 * For further information, consider the following reference:
 * 
 * [1] Thiago L. Gomes, Salles V. G. Magalhães, Marcus V. A. Andrade, W. Randolph Franklin, and Guilherme C. Pena. 2012. 
 *  Computing the drainage network on huge grid terrains. In Proceedings of the 1st ACM SIGSPATIAL International 
 *  Workshop on Analytics for Big Geospatial Data (BigSpatial '12). ACM, New York, NY, USA, 53-60. 
 *  DOI=10.1145/2447481.2447488 http://doi.acm.org/10.1145/2447481.2447488
 * 
 */
 
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cassert>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stack>
using namespace std;

typedef unsigned short int indexType;

class ComponentesProcessamento {
	public:

	~ComponentesProcessamento();
	ComponentesProcessamento(int nrows, int componentesWidth);

	void marcaComponentesConexos(int &numBlocosComponentesCompletamenteProcessados);
		
	int getComponentesWidth();
	int getNrowsComponentes();

	unsigned short int **componentes;
	vector<pair<int,int> > tamanhoComponentes;  //Inicializar!

	void marcaPontoFinalizado(int i,int j);
	bool isCompletamenteProcessado(int i, int j);

	int getNumeroComponentes();
	private:

	int larguraBlocoMemoria;
	int nrows;

	int nrowsComponentes;
	unsigned short int **numPontosFaltam;
	int componentesWidth;

	void marcaComponente(indexType  i, indexType  j, unsigned short int cc,int &) ;
};

bool ComponentesProcessamento::isCompletamenteProcessado(int i, int j) {
	return (numPontosFaltam[i][j]==0);
}

int ComponentesProcessamento::getNumeroComponentes() {
	return tamanhoComponentes.size();
}

ComponentesProcessamento::ComponentesProcessamento(int nrows, int componentesWidth) {
	this->componentesWidth = componentesWidth;
	this->nrows = nrows;

	nrowsComponentes = nrows/getComponentesWidth();


	

	numPontosFaltam =  new unsigned short int *[nrowsComponentes];
	for(int i =0;i<nrowsComponentes;i++) {
		numPontosFaltam[i] = new unsigned short int[nrowsComponentes];
		for(int j=0;j<nrowsComponentes;j++)
			numPontosFaltam[i][j] = getComponentesWidth()*getComponentesWidth();
	}

	componentes =  new unsigned short int *[nrowsComponentes];
	for(int i =0;i<nrowsComponentes;i++) {
		componentes[i] = new unsigned short int[nrowsComponentes];
		for(int j=0;j<nrowsComponentes;j++)
			componentes[i][j] = 1;
	}

	tamanhoComponentes.push_back( pair<int,int>(nrowsComponentes*nrowsComponentes,1));

}

void ComponentesProcessamento::marcaPontoFinalizado(int i,int j) {
	numPontosFaltam[i/componentesWidth][j/componentesWidth]--;
}

ComponentesProcessamento::~ComponentesProcessamento() {
	for(int i=0;i<nrowsComponentes;i++) {
		delete []numPontosFaltam[i];
	}
	//delete numSets;

	

}


int ComponentesProcessamento::getNrowsComponentes() {
	return nrowsComponentes;
}


int ComponentesProcessamento::getComponentesWidth() {
	return componentesWidth;
}


void ComponentesProcessamento::marcaComponente(indexType i, indexType j, unsigned short int cc,int &ctElems) {
	stack<pair<indexType,indexType> > st;
	st.push(pair<indexType,indexType>(i,j));

	while(!st.empty()) {
		pair<indexType,indexType> p = st.top(); st.pop();
		i = p.first; j = p.second;

		if (i<0 || j<0 || i>=nrowsComponentes || j>= nrowsComponentes )
			continue;

		if (componentes[i][j]!=0) continue;

		if (isCompletamenteProcessado(i,j)) //o tile esta COMPLETAMENTE setado... entao temos garantia que ele nao pode conectar ninguem!
			continue;
	
		componentes[i][j] = cc;
		ctElems++;


		st.push(pair<int,int>(i-1,j-1));
		st.push(pair<int,int>(i-1,j));
		st.push(pair<int,int>(i-1,j+1));

		st.push(pair<int,int>(i,j-1));
		st.push(pair<int,int>(i,j+1));

		st.push(pair<int,int>(i+1,j-1));
		st.push(pair<int,int>(i+1,j));
		st.push(pair<int,int>(i+1,j+1));
	}
}


void gravaNumSets(int **numSets,int **numCelulasBloco,int numLinhas,int id,int ccs) {
	stringstream st;
	st << "sets/sets_" << id << ".pgm";
	ofstream fout(st.str().c_str());


	  
	  if (fout) { 
	    fout << "P2" << endl;
	    fout << numLinhas << ' ' << numLinhas << endl;
	    fout << ccs << endl;
	    for (int i=0; i<numLinhas; i++) {
	      for (int j=0; j<numLinhas; j++) fout << (numSets[i][j]!=numCelulasBloco[i][j]) << ' ';
	      fout << '\n';
	    }
	    fout << flush;
	  } else {
		cerr << "Error writing flow.pgm file" << endl;
		exit(1);
	  }
	fout.close();
}




int id =0;

void ComponentesProcessamento::marcaComponentesConexos(int &numBlocosComponentesCompletamenteProcessados) {
	tamanhoComponentes.resize(0);

	//gravaNumSets(numPontosFinalizados,numCelulasBlocoComponentes,nrowsComponentes,id++,1);
	//gravaBlocosMemoria(posTiles,nrowsTiles,id);
	//cerr << "Marcando componentes" << endl;
	for(int i=0;i<nrowsComponentes;i++)
		for(int j=0;j<nrowsComponentes;j++) 
			componentes[i][j] = 0;


	numBlocosComponentesCompletamenteProcessados =0;
	for(int i=0;i<nrowsComponentes;i++)
		for(int j=0;j<nrowsComponentes;j++) {
			if( isCompletamenteProcessado(i,j) )
				numBlocosComponentesCompletamenteProcessados++;
		}
	//cerr << "Numero de blocos completamente processados: " << numBlocosComponentesCompletamenteProcessados << endl;
	//cerr << "Max sets: " << maxSt << endl;


	int cc =0;
	
	
	for(int i=0;i<nrowsComponentes;i++)
		for(int j=0;j<nrowsComponentes;j++) 
			if(componentes[i][j]==0 &&  !isCompletamenteProcessado(i,j) ) {
				int ctElems = 0;
				//cerr << "Marcadno " << i << " , " <<  j << endl;
				marcaComponente(i,j,++cc,ctElems);
				//cerr << "Marcado " << endl;
				tamanhoComponentes.push_back(pair<int,int>(ctElems,cc));
			}
	//cerr << "componentes marcados " << endl;

	sort( tamanhoComponentes.begin(), tamanhoComponentes.end());
	
}


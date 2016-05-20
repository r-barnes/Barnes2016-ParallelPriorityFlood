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
#include <list>
using namespace std;


class AgendadorProcessamento {
	public:
	AgendadorProcessamento(ComponentesProcessamento &comp, tiledMatrix<short int> &matrizMemoriaExterna, int nrows);
	void agendarProcessamento(queue<Point> filaProcCorrente[65536],int elevacaoAtingida,int quantidadeVezesMaisLentoMedia );
	int getNumBlocosComponentesCompletamenteProcessados();
	
	private:

	tiledMatrix<short int> &matrizMemoriaExterna;

	void  insereIlhaFilaProcFuturo(vector<pair<int,Point> > &ilha);
	void copiaIlhaFilaProcessamento(queue<Point> filaProcCorrente[65536]);
	ComponentesProcessamento &componentes;
	int numBlocosComponentesCompletamenteProcessados;
	int quantidadeVezesMaisLentoMinimaAtivarReagendamento;
	//int numeroBlocosComponentesCompletamenteProcessados;
	int nrows;
	int componentesWidth;

	queue<pair<int,int> > filaComponentesProcessar; //first == tamanho do componente, second == id do componente
	//vector<bool> componentesJaProcessados; //Componentes ja processados (o componente 0 nÃ£o existe... conta a partir do 1)

	list<vector<pair<int,Point> > > ilhasProcessarFuturo;
	int numIlhas;

	int getNumPontosNaMemoria(const vector<pair<int,Point> > &vetor);
	void moveIlhaFilaProcessamento(queue<Point> filaProcCorrente[65536], std::list< vector<pair<int,Point> > >::iterator itIlhaProcessarAgora);
};



int AgendadorProcessamento::getNumBlocosComponentesCompletamenteProcessados() {
	return numBlocosComponentesCompletamenteProcessados;
}

AgendadorProcessamento::AgendadorProcessamento(ComponentesProcessamento &comp, tiledMatrix<short int> &matrixMemExt, int nrows): componentes(comp), matrizMemoriaExterna(matrixMemExt) {
	this->numIlhas = 0;
	this->nrows = nrows;
	this->componentesWidth = componentes.getComponentesWidth();
	quantidadeVezesMaisLentoMinimaAtivarReagendamento = 10;
}

/*
Gerencia da fila de processamento futuro...
*/

int AgendadorProcessamento::getNumPontosNaMemoria(const vector<pair<int,Point> > &vetor) {
	int ct =0;
	int n = vetor.size();
	for(int i=0;i<n;i++)
		if ( matrizMemoriaExterna.isTileInMemory( vetor[i].second.y, vetor[i].second.x ) ) {
			ct++;
		}
	return ct;
}

void  AgendadorProcessamento::insereIlhaFilaProcFuturo(vector<pair<int,Point> > &ilha) {
	ilhasProcessarFuturo.push_front(ilha);	
	numIlhas++; //contador para evitar o uso do metodo size ( O(N) )
}

void AgendadorProcessamento::moveIlhaFilaProcessamento(queue<Point> filaProcCorrente[65536], std::list< vector<pair<int,Point> > >::iterator itIlhaProcessarAgora) {
		vector<pair<int,Point> > &ilhaProcessarAgora = *itIlhaProcessarAgora;
		int sz = ilhaProcessarAgora.size();
		assert(sz>0);
		cerr << "Pegando componente com tamanho: " << sz << "\n";
		for(int i=0;i<sz;i++) {
			filaProcCorrente[ ilhaProcessarAgora[i].first + 32768].push(ilhaProcessarAgora[i].second);
		}
		ilhasProcessarFuturo.erase(itIlhaProcessarAgora);
		numIlhas--; //contador para evitar o uso do metodo size ( O(N) )
}

void AgendadorProcessamento::copiaIlhaFilaProcessamento(queue<Point> filaProcCorrente[65536]) {
		if (ilhasProcessarFuturo.empty()) {
			cerr << "Acabaram-se os pontos a serem processados " <<endl;
			return;
		}

		std::list< vector<pair<int,Point> > >::iterator itIlhaProcessarAgora =  ilhasProcessarFuturo.begin();
		int pontosBegin = ilhasProcessarFuturo.begin()->size();
		int pontosMemBegin = getNumPontosNaMemoria(*(ilhasProcessarFuturo.begin()));
		double percentualMemMelhor = pontosMemBegin*1.0/pontosBegin;

		int numMaxIlhasPegar = max(1,numIlhas/10);
		int numIlhasPeguei = 0;
				
		//Log
		unsigned int numTotalPontos = 0;
		unsigned int numTotalPontosMemoria = 0;
		ofstream log("logComponentes.txt",ios::app);
		log << "\n\n---------------------------------------------\n-----------------------------------\n\n";		
		log << "size: " << numIlhas << "\n";
		std::list< vector<pair<int,Point> > >::iterator iterator;

		for (iterator = ilhasProcessarFuturo.begin(); iterator != ilhasProcessarFuturo.end(); ) {
    			int numPontos = iterator->size();
			int numPontosMemoria = getNumPontosNaMemoria(*iterator);
			numTotalPontos += numPontos;
			numTotalPontosMemoria += numPontosMemoria;
			double percentualMem = numPontosMemoria*1.0/numPontos ;

			if (percentualMem > percentualMemMelhor) {
				percentualMemMelhor  = percentualMem;
				itIlhaProcessarAgora = iterator;
			}	
		
			if (numIlhasPeguei >= numMaxIlhasPegar)
				break;
			else if (numPontos == numPontosMemoria) { //Ilha com 100% da borda na memoria... vamos pegÃ¡-la!
				moveIlhaFilaProcessamento(filaProcCorrente,iterator++);
				numIlhasPeguei++;
			} else {
				++iterator;
			}		
			
		}
		log << "\n";
		log << "Total Pontos: " << numTotalPontos << "\n";
		log << "Total Pontos Memoria: " << numTotalPontosMemoria << "\n";
		log << "Percentual total na memoria: " << numTotalPontosMemoria*1.0/numTotalPontos << "\n";
		log << "Processarei ilha com percentual: " << percentualMemMelhor << "\n";
		log << "Peguei " << numIlhasPeguei << " ilhas" << "\n";


		log.close();
		//End log

		
		
		if (numIlhasPeguei==0) //Se nao peguei nenhuma ilha com 100% de pontos na memoria...
			moveIlhaFilaProcessamento(filaProcCorrente,itIlhaProcessarAgora);
		
		

}


/*




*/

void AgendadorProcessamento::agendarProcessamento(queue<Point> filaProcCorrente[65536],int elevacaoAtingida,int quantidadeVezesMaisLentoMedia ) {
	//Se acabei de processar um componente inteiro, vou pegar outro(s) para processar...
	if (elevacaoAtingida > 32767) {
		cerr << "####\n" ;
		cerr << "\nPegarei outro componente...\n";
		copiaIlhaFilaProcessamento(filaProcCorrente);	
	} else if (quantidadeVezesMaisLentoMedia > this->quantidadeVezesMaisLentoMinimaAtivarReagendamento ) { //Se o componente atual estÃ¡ com processamnto lento entao vamos tentar dividi-lo...
		componentes.marcaComponentesConexos(this->numBlocosComponentesCompletamenteProcessados);
		int numeroComponentes = componentes.getNumeroComponentes();

		//Se hÃ¡ mais de um componente, vamos tentar dividÃ­-lo, escolher um subconjunto dos componentes para processar agora e colocar os outros na (frente da) fila de proc. futuro	
		if (numeroComponentes > 1) {	//##### Talvez fazer isso apenas qdo o numero de componentes aumentar  #####
			//Vamos pegar um  dos componentes e deixar na fila corrente....as celulas dos outros componentes vao para o futuro...  ##### no futuro, tentar usar estrategia melhor para escolher qual componente deixar....
			
			int numeroCelulasFilasCorrente = 0;
			vector<vector<pair<int,Point> > > ilhasGeradas;
			vector<int> mapaPosicaoIlhas(numeroComponentes+1,-1);
			vector<int> tamanhoComponente(numeroComponentes+1,-1);
			for(unsigned int i=0;i<componentes.tamanhoComponentes.size();i++)
				tamanhoComponente[componentes.tamanhoComponentes[i].second] = componentes.tamanhoComponentes[i].first;

			//Vamos separar as ilhas geradass e guarda-las no vetor "ilhasGeradas"...
			for(int i=-32768;i<=32767;i++) {
				while( !filaProcCorrente[i+32768].empty() ) {
					Point p = filaProcCorrente[i+32768].front(); filaProcCorrente[i+32768].pop();
					int componenteP = componentes.componentes[p.y/componentesWidth][p.x/componentesWidth];			
					if ( mapaPosicaoIlhas[componenteP] ==-1) { 
						ilhasGeradas.push_back( vector<pair<int,Point> >() );
						mapaPosicaoIlhas[componenteP] = ilhasGeradas.size() -1;
					}						
					ilhasGeradas[ mapaPosicaoIlhas[componenteP]  ].push_back( pair<int,Point>(i,p) );
								
				}	
			}


			for(int i=componentes.tamanhoComponentes.size()-1;i>=0;i--) {
				int compSz = componentes.tamanhoComponentes[i].first;
				int compId = componentes.tamanhoComponentes[i].second;
				int posicaoComponente = mapaPosicaoIlhas[compId];
				if (posicaoComponente!=-1)
					insereIlhaFilaProcFuturo(ilhasGeradas[posicaoComponente]);
					
			}
			
			copiaIlhaFilaProcessamento(filaProcCorrente);

			//cerr << "Celulas nas filas de proc. corrente (MB): " << (numeroCelulasFilasCorrente*sizeof(Point))/(1024.0*1024.0) << "\n";	
			for(int i=0;i< numeroComponentes ;i++) {
				int idComponente = componentes.tamanhoComponentes[i].second;
				int tamanhoComponente = componentes.tamanhoComponentes[i].first;
				cerr << "(" << idComponente << ", " << tamanhoComponente << ") ";
			} cerr << "\n";

		}
		
	}


}


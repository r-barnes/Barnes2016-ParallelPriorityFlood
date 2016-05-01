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
#include <algorithm>
#include <queue>
#include <cmath>
#include <fstream>
#include "tiledMatrix.cpp"
#include "componentesProcessamento.cpp"
#include "flow.cpp"
#include <stdio.h>
#include <time.h>

using namespace std;

// http://en.wikipedia.org/wiki/ANSI_escape_code
const string redtty("\033[1;31m");   // tell tty to switch to red
const string greentty("\033[1;32m");   // tell tty to switch to bright green
const string bluetty("\033[1;34m");   // tell tty to switch to bright blue
const string magentatty("\033[1;35m");   // tell tty to switch to bright magenta
const string yellowbgtty("\033[1;43m");   // tell tty to switch to bright yellow background
const string underlinetty("\033[4m");   // tell tty to switch to underline
const string deftty("\033[0m");      // tell tty to switch back to default color

// Exec a string then print its time, e.g. TIME(init());
#define TIME(arg) { arg;  } ; ptime(#arg); 

time_t tStart;

void ptime(const char *const msg) {
  float t= ((float)clock())/CLOCKS_PER_SEC;
  float tr = difftime(time(NULL),tStart);
  cerr << magentatty << "Cumulative CPU time thru " << msg << "= " << t << "   |  Real time= " << tr << deftty << endl;





//  cout  << "Cumulative CPU time thru " << msg << "= " << t << "   |  Real time= " << tr  << endl;
}

// Create a new array of size var of type.  Report error.
#define NEWA(var,type,size) { try  { if(0==(var=new type [(size)])) throw;} catch (...)  { cerr << "NEWA failed on " #var "=new " #type "[" #size "=" << (size) << "]" << endl; exit(1); }}


int tilesWidth =  400;
int tamMemoria = 1600;
const int TILE_CONECTED_COMPONENTS= 10;



struct timeval tstart, end;
void zeraRelogio() {    
    gettimeofday(&tstart, NULL);

}

double getSecsDesdeZerado() {
    long mtime, seconds, useconds;  
    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - tstart.tv_sec;
    useconds = end.tv_usec - tstart.tv_usec;



    return ((seconds) * 1000 + useconds/1000.0)/1000.0;
}


typedef  pair<int,int> pii;




#include "agendadorProcessamento.cpp"


/*bool operator<(const Point &a, const Point &b) {
  return a.z > b.z;
}*/




/*
#include <stack>
#define front top
*/


//Uses Young-He's algorithm to flood a terrain...
//Ref: http://www.google.com.br/url?sa=t&source=web&cd=3&ved=0CDIQFjAC&url=http%3A%2F%2Fwww.sciencedirect.com%2Fscience%2Farticle%2Fpii%2FS0098300405001937&ei=6FD2Tc-8JYSGhQfKxtjgBg&usg=AFQjCNF4EmCGPPtO6jCH1AMXyCzZbQcrNQ&sig2=oG8MIndi0fXNdixh_nXXvw

//Allocates matrix: dirs(nrows][nrows]



struct StatusProcessamento {
	unsigned int numeroCelulasProcessadas;
	int elevacaoAtingida;
};

StatusProcessamento flood(tiledMatrix<short int>  &elevs,tiledMatrix<unsigned char> &dirs,const int nrows, queue<Point> queues[65536], int elevStart,int elevEnd, int maxCelulasProcessar, ComponentesProcessamento &componentes ) {

	int numCelulasProcessdas = 0;
	int seaLevel = elevStart;
	for(;seaLevel<=elevEnd;seaLevel++) {
		while (!queues[seaLevel+32768].empty()) {
			if (numCelulasProcessdas >= maxCelulasProcessar) {
				StatusProcessamento status;
				status.numeroCelulasProcessadas = numCelulasProcessdas;
				status.elevacaoAtingida = seaLevel;
				return status;
			}
	
			Point p = queues[seaLevel+32768].front(); queues[seaLevel+32768].pop();

			componentes.marcaPontoFinalizado(p.y,p.x);
			numCelulasProcessdas++;
			//seaLevel = p.z;

			if (p.y > 0 ) {
				if (p.x >0) {
					if (dirs.get(p.y-1,p.x-1)==0) {		
						dirs.set(p.y-1,p.x-1,16);		

						if ( elevs.get( p.y-1,p.x-1 ) <= seaLevel )
							 elevs.set( p.y-1,p.x-1 ,  seaLevel);
						queues[ 32768 +elevs.get( p.y-1,p.x-1 )].push( Point(p.y-1,p.x-1 ) );	
					}
				}

				if (dirs.get(p.y-1,p.x)==0)  {	
					dirs.set(p.y-1,p.x,32);		
	
					if ( elevs.get( p.y-1,p.x ) <= seaLevel )
						 elevs.set( p.y-1,p.x,  seaLevel);
					queues[ 32768 +elevs.get( p.y-1,p.x )].push( Point(p.y-1,p.x ) );	
				}

				if (p.x < nrows-1) {
					if (dirs.get(p.y-1,p.x+1)==0) {
						dirs.set(p.y-1,p.x+1,64);

						if ( elevs.get( p.y-1,p.x+1 ) <= seaLevel )
							 elevs.set( p.y-1,p.x+1 ,  seaLevel);
						queues[ 32768 +elevs.get( p.y-1,p.x+1 )].push( Point(p.y-1,p.x+1) );
					}	
				}
			} 
			if (p.x >0) {
				if (dirs.get(p.y,p.x-1)==0) {
					dirs.set(p.y,p.x-1,8);

					if ( elevs.get( p.y,p.x-1 ) <= seaLevel )
						elevs.set( p.y,p.x-1 ,  seaLevel);
					queues[ 32768 +elevs.get(p.y,p.x-1 )  ].push( Point(p.y,p.x-1 ) );	
				}
			}
			if (p.x < nrows-1) {
				if (dirs.get(p.y,p.x+1)==0) { 
					dirs.set(p.y,p.x+1, 128);

					if ( elevs.get( p.y,p.x+1 ) <= seaLevel )
						elevs.set( p.y,p.x+1 ,  seaLevel);
					queues[ 32768 +elevs.get( p.y,p.x+1 ) ].push( Point(p.y,p.x+1 ) );	
				}
			}
	
			if (p.y < nrows-1 ) {
				if (p.x >0) {
					if (dirs.get(p.y+1,p.x-1)==0)  {
						dirs.set(p.y+1,p.x-1,4);

						if ( elevs.get( p.y+1,p.x-1 ) <= seaLevel )
							 elevs.set( p.y+1,p.x-1 ,  seaLevel);
						queues[ 32768 +elevs.get( p.y+1,p.x-1 ) ].push( Point(p.y+1,p.x-1 ) );	
					}
				}

				if (dirs.get(p.y+1,p.x)==0)  {
					dirs.set(p.y+1,p.x,2);

					if ( elevs.get( p.y+1,p.x ) <= seaLevel )
						 elevs.set( p.y+1,p.x,  seaLevel);
					queues[ 32768 +elevs.get( p.y+1,p.x ) ].push( Point(p.y+1,p.x ) );	
				}

				if (p.x < nrows-1) {
					if (dirs.get(p.y+1,p.x+1)==0) {
						dirs.set(p.y+1,p.x+1,1);

						if ( elevs.get( p.y+1,p.x+1 ) <= seaLevel )
							 elevs.set( p.y+1,p.x+1 ,  seaLevel);
						queues[ 32768 + elevs.get( p.y+1,p.x+1 ) ].push( Point(p.y+1,p.x+1 ) );	
					}
				}
			} 
		}

	}

	StatusProcessamento status;
	status.numeroCelulasProcessadas = numCelulasProcessdas;
	status.elevacaoAtingida = seaLevel;
	return status;
}

void gravaComponentes(int **componentes,int numLinhas,int id,int ccs) {
	stringstream st;
	st << "componentes/componente_" << id << ".pgm";
	ofstream fout(st.str().c_str());



	  
	  if (fout) { 
	    fout << "P2" << endl;
	    fout << numLinhas << ' ' << numLinhas << endl;
	    fout << ccs << endl;
	    for (int i=0; i<numLinhas; i++) {
	      for (int j=0; j<numLinhas; j++) fout << componentes[i][j] << ' ';
	      fout << '\n';
	    }
	    fout << flush;
	  } else {
		cerr << "Error writing flow.pgm file" << endl;
		exit(1);
	  }
	fout.close();
}



queue<Point> filaProcCorrente[65536];
void flood(tiledMatrix<short int>  &elevs,tiledMatrix<unsigned char> &dirs,const int nrows) {

	dirs.set(0);

	
	
	for (int i=0;i<nrows;i++) {
		filaProcCorrente[elevs.get(i,0) + 32768].push(Point(i,0)); //filaProcCorrente[elevs(i][0] + 32768].push(Point(i,0));
		filaProcCorrente[elevs.get(i,nrows-1)+ 32768].push(Point(i,nrows-1)); //filaProcCorrente[elevs(i][nrows-1]+ 32768].push(Point(i,nrows-1));

		dirs.set(i,0,128); //dirs(i,0] = 128;
		dirs.set(i,nrows-1, 8); //dirs(i,nrows-1] = 8;
	}
	for (int i=1;i<nrows-1;i++) {
		filaProcCorrente[elevs.get(0,i)+ 32768].push(Point(0,i)); //filaProcCorrente[elevs(0,i]+ 32768].push(Point(0,i));
		filaProcCorrente[elevs.get(nrows-1,i)+ 32768].push(Point(nrows-1,i)); //filaProcCorrente[elevs(nrows-1,i]+ 32768].push(Point(nrows-1,i));

		dirs.set(0,i,2); 
		dirs.set(nrows-1,i, 32);
	}	

	int ctS=0;	
	
	StatusProcessamento statusProc;
	int elevCorrente = -32768;

	
	
	ComponentesProcessamento componentes(nrows,TILE_CONECTED_COMPONENTS);
	AgendadorProcessamento agendador(componentes,elevs, nrows);

	int nrowsComponentes = componentes.getNrowsComponentes();
	int componentesWidth = componentes.getComponentesWidth();
	int totalBlocosComponentes = nrowsComponentes*nrowsComponentes;

	
	unsigned int  numCelulasProc = 0;
	int id =0;
	int iteracao = 0;
	unsigned int nrows2 = ((unsigned int)nrows)*((unsigned int)nrows);

	unsigned int numCelulasPorVez = max(nrows2/1000,(unsigned int)10000);

	double tempoTotal10Iteracoes =0;
	double tempoMedio10Iteracoes;
	double tempoTotalExtra =0;

	int quantidadeIdealBlocosProcessar = (dirs.getNumTilesMemoria()*(dirs.getTilesWidth()/componentes.getComponentesWidth())*(dirs.getTilesWidth()/componentes.getComponentesWidth()))/2;
	cerr << "NrowsComponentes : " << nrowsComponentes << endl;
	cerr << "QuantidadeIdealBlocosProcessar: " << quantidadeIdealBlocosProcessar << endl;
	cerr << "Largura bloco componentes: " << componentesWidth << endl;
	cerr << "Numero de celulas processadas por vez: " <<numCelulasPorVez <<endl;
	
	vector<bool> componentesProcessados(1,false);
	vector<bool> componentesProcessar(1,true);
	int componenteCorrente = 1;

	//Tem varias coisas para arrumar aqui! a elevacao nao precisa ser reinicializada sempre para -32768 , etc...
	while(true) {
		iteracao++;
		elevCorrente = -32768;
		zeraRelogio();

		cerr << "\nIniciando flood...: " << iteracao <<endl;
		
		
		statusProc = flood(elevs,dirs,nrows, filaProcCorrente, elevCorrente,32767, numCelulasPorVez,componentes);
		numCelulasProc += statusProc.numeroCelulasProcessadas;

		double tempoFlood = getSecsDesdeZerado();
		double quantidadeVezesMaisLentoMedia = 	tempoFlood/tempoMedio10Iteracoes ;
		zeraRelogio();

		
		if(iteracao<=10) {
			tempoTotal10Iteracoes += tempoFlood;
			tempoMedio10Iteracoes = tempoTotal10Iteracoes/iteracao;
		
			cerr << "Tempo total: " << tempoTotal10Iteracoes << endl;
		} else {
			agendador.agendarProcessamento(filaProcCorrente,statusProc.elevacaoAtingida,quantidadeVezesMaisLentoMedia);
		}
		tempoTotalExtra += getSecsDesdeZerado();	

		cerr << "Tempo medio: " << tempoMedio10Iteracoes << '\n';
		cerr << "Quantas vezes mais lento que media? :::::::::::: " << quantidadeVezesMaisLentoMedia << "\n";
		cerr << "Elev: " << statusProc.elevacaoAtingida << "\n";
		cerr << "Tempo: " << tempoFlood << "\n";	
		cerr << "Tempo total extra: " << tempoTotalExtra << "\n";	
		cerr << "Celulas processadas: " << statusProc.numeroCelulasProcessadas << "\n";
		cerr << "\% terreno processado: " << numCelulasProc*100.0/(nrows2) << "\n";
		cerr << "Numero total de celulas processadas: " << numCelulasProc << "\n";
		cerr << "Celulas/s: " << (statusProc.numeroCelulasProcessadas*1.0)/tempoFlood << "\n" ;		

		if (agendador.getNumBlocosComponentesCompletamenteProcessados() == totalBlocosComponentes || numCelulasProc == nrows2) { //ja processei todas celulas
			cerr << "Processamento concluÃ­do..." << endl;
			break;
		}

			
	}

	
}






int numTilesMemoria = (tamMemoria*(long long)1024*1024)/(6*tilesWidth*tilesWidth);//(2450*(500*500)/(tilesWidth*tilesWidth));

int tilesWidthElevs = tilesWidth;
int numTilesMemoriaElevs = numTilesMemoria*1.5;

int tilesWidthDirs = tilesWidth;
int numTilesMemoriaDirs = numTilesMemoria*1.5;

int tilesWidthFlowIndegree = tilesWidth;
int numTilesMemoriaFlowIndegree = numTilesMemoria;//1225*6;

//Uses a "topological sorting" to compute flow accumulation
//Allocates matrix: flow[nrows][nrows]
void computeFlow(tiledMatrix<unsigned char> &dirs,tiledMatrix<int> &flow,const int nrows) {

	tiledMatrix<unsigned char> inputDegree( nrows, nrows,tilesWidthFlowIndegree, numTilesMemoriaFlowIndegree,"tiles/tile_inputDegree_"); //Matrix that represents the input degree of each point in the terrain
                      //the input degree of a point c represents the number of points in the 
                      //terrain that flows directly to c
  	
	inputDegree.set(0);


	int directionToDyDx[129][2]; //Represents the deltaY and deltaX of each direction code...
		         	     //For example, directionToDyDx[64][0] == 1 (deltaY) and directionToDyDx[64][1] == -1 (deltaX)
	directionToDyDx[1][0] = -1;
	directionToDyDx[1][1] = -1;
	directionToDyDx[2][0] = -1;
	directionToDyDx[2][1] = 0;
	directionToDyDx[4][0] = -1;
	directionToDyDx[4][1] = +1;

	directionToDyDx[128][0] = 0;
	directionToDyDx[128][1] = -1;
	directionToDyDx[8][0] = 0;
	directionToDyDx[8][1] = 1;


	directionToDyDx[64][0] = 1;
	directionToDyDx[64][1] = -1;
	directionToDyDx[32][0] = 1;
	directionToDyDx[32][1] = 0;
	directionToDyDx[16][0] = 1;
	directionToDyDx[16][1] = +1;

	#define PROCESSED_POINT_FLAG 255


	flow.set(1);
	for(int i=0;i<nrows;i++)
		for(int j=0;j<nrows;j++) {	
			//flow[i][j] = 1;		
			unsigned int ptDir = (unsigned int) dirs.get(i,j);
			if (ptDir!=0) {			
				if (	i+directionToDyDx[ptDir][0] <0  || j+directionToDyDx[ptDir][1] <0 || i+directionToDyDx[ptDir][0] >=nrows  || j+directionToDyDx[ptDir][1] >=nrows ) 
					continue;	
				inputDegree.set( i+directionToDyDx[ptDir][0] , j+directionToDyDx[ptDir][1], inputDegree.get( i+directionToDyDx[ptDir][0] , j+directionToDyDx[ptDir][1] ) +1 ); //Increases the input degree of the point that receives the flow from point (i,j)
			}
		}



	for(int i=0;i<nrows;i++)
		for(int j=0;j<nrows;j++) {	
				/*if (inputDegree[i][j]==PROCESSED_POINT_FLAG) //If the point has already been processed.... 
					continue;*/
	
				pii point(i,j);
				//Distributes the flow from point (i,j) to the neighbor of (i,j) that will receive its flow				
				while(inputDegree.get( point.first , point.second )==0) {
					inputDegree.set( point.first , point.second ,  PROCESSED_POINT_FLAG); //Mark this point as processed	
					int dirPt = dirs.get(point.first,point.second);	
					pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
					if (neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows ) 
						break;

					
					flow.set( neighbor.first , neighbor.second , flow.get(point.first,point.second) + flow.get( neighbor.first , neighbor.second) );
					inputDegree.set( neighbor.first , neighbor.second, inputDegree.get( neighbor.first , neighbor.second ) -1 );
					point = neighbor; //Repeat the process with the neighbor (if it has input degree equal to 0).
				}			
		}


	
}






//Allocates matrix: elevs[nrows][nrows] 
void init(const int argc, const char **argv,int &nrows) {
	if (argc!=3) {
		cerr << "Error, use: hydrog nrows input" << endl;
		exit(1);
	}

	tStart = time(NULL);
	nrows = atoi(argv[1]);

}

void writeFlowDirs(tiledMatrix<unsigned char> &dirs,const int nrows) {  
	int dir2[256];
	for(int i=0;i<8;i++)
		dir2[1<<i] = i;
	

  	for (int i=0; i<nrows; i++) {
	     for (int j=0; j<nrows; j++) {		
		unsigned char dir = dirs.get(i,j);	
		//cout << dir2[dir] ;	
		cout.write(reinterpret_cast<char *>(&(dir)),1);
	     }
	}
	cout << flush;
}

void writeFlowPgm(tiledMatrix<int> &flow,const int nrows) { 
  	ofstream fcomp("flow.hgt");

  	    for (int i=0; i<nrows; i++) {
	      for (int j=0; j<nrows; j++) {
			/*unsigned short tmp = (sqrt(float(flow[i][j])));
			tmp = (tmp&255)<<8 | ((tmp>>8)&255);
			fcomp.write(reinterpret_cast<char *>(&tmp),2);*/
			int flw = flow.get(i,j);
			short int tmp = flw>30000? 30000:flw;
			fcomp.write(reinterpret_cast<char *>(&tmp),2);

	      }
	    }
	fcomp.close();
}




int main(const int argc, const char **argv) {

        
	int nrows; //Number of rows in the terrain
	
	TIME(init(argc,argv,nrows));



	string arquivoEntrada = string(argv[2]);

	//Utiliza alocacao dinamica para economizar memoria (podemos desalocar a matriz antes do fim da funcao!)
	tiledMatrix<short int> *elevs2 = new tiledMatrix<short int>( nrows, nrows,tilesWidthElevs, numTilesMemoriaElevs,"tiles/tile_elevs_");
	
	tiledMatrix<short int> &elevs = *elevs2;	
	
	cerr << "Num. tiles na memoria: " << numTilesMemoria << endl;

	ifstream fin(arquivoEntrada.c_str());
	assert(fin.good());
	for(int i=0;i<nrows;i++) {
		for(int j=0;j<nrows;j++) {
			short int in;
			fin.read(reinterpret_cast<char *>(&in),2);
			elevs.set(i,j,  in);
		}
	}

	tiledMatrix<unsigned char> dirs( nrows, nrows,tilesWidthDirs, numTilesMemoriaDirs,"tiles/tile_dirs_" );//unsigned char **dirs; //Flow direction matrix
		             // The direction of a cell c may be 1,2,4,8,16,32,64 or 128
		             // 1 means c points to the upper left cell, 2 -> upper cell, 
		             // 4 -> upper right , ...
		             // 1   2   4
		             // 8   c  16
		             // 32 64 128

	
  
	
	//Flood depressions and Compute directions
	 TIME(flood(elevs,dirs,nrows));

	//elevs.gravaStatVezes("elevsStats.pgm");
	//dirs.gravaStatVezes("dirsStatsFlood.pgm");
	
	delete elevs2; //Desaloca matriz de elevacao... nao precisamos dela mais!
    time_t start,end;
    double dif;
    
    if(nrows <= 10000) { // o terreno cabe na memoria 
        tiledMatrix<int> *flow2 = new tiledMatrix<int>(nrows, nrows,tilesWidthFlowIndegree, numTilesMemoriaFlowIndegree,"tiles/tile_flow_" );
        tiledMatrix<int> &flow = *flow2;
	  

        time (&start);
  
	
	    TIME(computeFlow(dirs,flow,nrows));
	    TIME(writeFlowPgm(flow,nrows));
        time (&end);
        dif = difftime (end,start);
	    cout << "\nFluxo acumulado----------------------\n";
	    cout<< dif << endl;
	
	    delete flow2;
     }else {
	     time (&start);
	     TIME(CacheAwareAccumulation(dirs,nrows));
         time (&end);
         dif = difftime (end,start);
         cout << "\nFluxo acumulado----------------------\n";
	     cout<< dif << endl;
	//dirs.gravaStatVezes("dirsStatsFlow.pgm");
	//flow.gravaStatVezes("flowStats.pgm");
     }
	


	return 0;
	
}



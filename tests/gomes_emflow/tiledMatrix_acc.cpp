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
using namespace std;

//#define DBG(X) X
#define DBG(X) 

template <class T>
class tiledMatrix_acc {
	public:
		void gravaStatVezes(string arquivo);

		T get(int i, int j);
		void set(int i,int j, T valor);
		void set(T valor);

		T& operator() (unsigned int i, unsigned int j);        
  		T  operator() (unsigned int i, unsigned int j) const; 

		tiledMatrix_acc(int nrows,int ncolumns,int tilesWidth,int nTilesMemoria,string arquivoBase,string arquivoMatriz="");

		~tiledMatrix_acc();
	
		tiledMatrix_acc();
		tiledMatrix_acc(const tiledMatrix_acc &x);		
	
		void loadTile2(int i,int j);
		void loadTile(int i,int j);
		void removeTileDaMemoria(int i,int j);
                T Valor_Inicial;
		T **matrizes;
		int tilesWidth;
		int nrowsTiles,ncolumnsTiles;
		int numTilesMemoria;
		int nrows,ncolumns;

		unsigned long long timeStamp;

		unsigned long long *timeStamps;
		pair<int,int> *tilesCarregados;
		int **posTiles;
		//fstream arquivoAssociado;

		int **numVezesTileCarregadoDisco;

		unsigned long tempoTotalCarregando;

		int tilesAbertos;
		int tilesCarregadosMemoria;
		int totalFalhasTiles;
		int tilesGravadosMemoria;
};

template <class T>
tiledMatrix_acc<T>::tiledMatrix_acc() {

}

template <class T>
tiledMatrix_acc<T>::tiledMatrix_acc(const tiledMatrix_acc &x) {

}

template <class T>
tiledMatrix_acc<T>::~tiledMatrix_acc() {
	cerr << "------------------------------------------------------------" <<endl;
	cerr << "Sizeof(T): " << sizeof(T) << endl;
	cerr << "Tamanho da matriz (MB): " << (nrows*((long long)ncolumns)*sizeof(T))/(1024*1024) << endl;
	cerr << "Total falhas: " << totalFalhasTiles << endl;
	cerr << "Total Tiles carregados: "<< tilesCarregadosMemoria << endl;
	cerr << "Tempo total carregando tiles (s): " << tempoTotalCarregando/1000000 << endl;
	cerr << "Tempo total carregando tiles (u): " << tempoTotalCarregando << endl;
	cerr << "Tempo de carga para cada 1M celulas: " << (tempoTotalCarregando)/( ((double)tilesCarregadosMemoria+totalFalhasTiles)*tilesWidth*tilesWidth) << endl;
	cerr << "Espaco ocupado por cada celula: " << sizeof(T) << '\n';
	cerr << "Quantos MB de celula carregados por segundo: " <<  sizeof(T)/((tempoTotalCarregando)/( ((double)tilesCarregadosMemoria+totalFalhasTiles)*tilesWidth*tilesWidth))   << " MB/s" << endl;
	cerr << "Numero de vezes o terreno foi recarregado: " << tilesCarregadosMemoria*1.0/(nrowsTiles*ncolumnsTiles) << endl ;
	cerr << "------------------------------------------------------------" <<endl;

	delete timeStamps;
	delete tilesCarregados;
	for(int i=0;i<nrowsTiles;i++) {
		delete []posTiles[i];
	}
	//assert(arquivoAssociado.is_open());
	//arquivoAssociado.close();
	for(int i=0;i<numTilesMemoria;i++) {
		delete []matrizes[i];
	}

	/*delete numVezesTileCarregadoDisco;
	for(int i=0;i<nrowsTiles;i++) {
		delete []numVezesTileCarregadoDisco[i];
	}*/
}



template <class T>
tiledMatrix_acc<T>::tiledMatrix_acc(int nrows,int ncolumns,int tilesWidth,int nTilesMemoria,string arquivoBase,string arquivoMatriz) {
	tempoTotalCarregando=0;

	tilesAbertos = 0;
	tilesCarregadosMemoria =0;
	totalFalhasTiles =0;
	tilesGravadosMemoria = 0;

	assert(tilesWidth>0);
	assert(nrows > 0 && ncolumns >0);
	
	if(nrows%tilesWidth != 0){
	    int aux =  nrows/tilesWidth;
	    nrows = aux*tilesWidth + tilesWidth;
	}
	if(ncolumns%tilesWidth != 0){
	    int aux =  ncolumns/tilesWidth;
	    ncolumns = aux*tilesWidth + tilesWidth;
	}

	nrowsTiles = nrows/tilesWidth;
	ncolumnsTiles = ncolumns/tilesWidth;
	this->tilesWidth = tilesWidth;
	this->numTilesMemoria = nTilesMemoria;
	this->nrows = nrows;
	this->ncolumns = ncolumns;
       	posTiles =  new int *[nrowsTiles];
	for(int i =0;i<nrowsTiles;i++) {
		posTiles[i] = new int[ncolumnsTiles];
		for(int j=0;j<ncolumnsTiles;j++)
			posTiles[i][j] = -1;
	}

	/*numVezesTileCarregadoDisco =  new int *[nrowsTiles];
	for(int i =0;i<nrowsTiles;i++) {
		numVezesTileCarregadoDisco[i] = new int[ncolumnsTiles];
		for(int j=0;j<ncolumnsTiles;j++)
			numVezesTileCarregadoDisco[i][j] = 0;
	}*/

	
	matrizes = new T *[numTilesMemoria];
	for(int k=0;k<numTilesMemoria;k++) {
		matrizes[k] = new T[tilesWidth*tilesWidth];
	}

	

	timeStamp =1;
	tilesCarregados = new pair<int,int>[numTilesMemoria];
	timeStamps = new unsigned long long [numTilesMemoria];
	for(int i=0;i<numTilesMemoria;i++) {
		timeStamps[i]= 0;
	}
        return;
	//cerr <<" usando arquivo " <<arquivoBase.c_str() <<endl;
	/*arquivoAssociado.open(arquivoBase.c_str(),ios::out | ios::binary);
	
	

	unsigned long long tamanhoArquivo = ((unsigned long long)nrows)*ncolumns*sizeof(T);

	if (arquivoMatriz.size()==0) {
		char c=0;
		arquivoAssociado.seekp(tamanhoArquivo-1,ios::beg);
		arquivoAssociado.write( reinterpret_cast<char *>( &c ),1);

		arquivoAssociado.close();
		arquivoAssociado.open(arquivoBase.c_str(),ios::in | ios::out | ios::binary);
		assert(arquivoAssociado.is_open());
			
		return;
	} 

	
	ifstream fin(arquivoMatriz.c_str(),ios::binary);
	assert(fin.good());


	arquivoAssociado.seekp(0,ios::beg);
	//Suponha que tilesWidth*ncolumns*sizeof(T) caibam na memoria....
	T *matrizTemp = new T [tilesWidth*ncolumns];
	for(int i=0;i<nrowsTiles;i++) { //Le uma "linha" e grava os arquivos em disco..
		fin.read(reinterpret_cast<char *>(matrizTemp),tilesWidth*ncolumns*sizeof(T));
	

		//Para cada tile (i,t) (linha i, coluna t)
		for(int t=0;t<ncolumnsTiles;t++) {
			for(int y=0;y<tilesWidth;y++) {
				for(int x=0;x<tilesWidth;x++) {
					arquivoAssociado.write( reinterpret_cast<char *>( matrizTemp[y*ncolumns + x] ),sizeof(T));
				}
			}
		}		
	}
	delete matrizTemp;

	arquivoAssociado.close();
	arquivoAssociado.open(arquivoBase.c_str(),ios::in | ios::out | ios::binary);
	assert(arquivoAssociado.is_open());
	*/
}


template <class T>
void tiledMatrix_acc<T>::set(T valor) {
        Valor_Inicial = valor;
	
}

template <class T>
T tiledMatrix_acc<T>::get(int i, int j) {
	int posTile = posTiles[i/tilesWidth][j/tilesWidth];
	if (posTile == -1) {
		loadTile(i/tilesWidth,j/tilesWidth);
		posTile = posTiles[i/tilesWidth][j/tilesWidth];
	}
	timeStamps[posTile] = timeStamp++; //Marca o timestamp dele como um novo timestamp
	return matrizes[posTile][ (i%tilesWidth)*tilesWidth + (j%tilesWidth)   ];
}

template <class T>
void tiledMatrix_acc<T>::set(int i,int j, T valor) {

	int posTile = posTiles[i/tilesWidth][j/tilesWidth];
	if (posTile == -1) {
		DBG(cerr << "carregando " <<endl;)
		loadTile(i/tilesWidth,j/tilesWidth);
		posTile = posTiles[i/tilesWidth][j/tilesWidth];
		DBG(cerr << "pos " << posTile << endl;)
	}
	timeStamps[posTile] = timeStamp++; //Marca o timestamp dele como um novo timestamp
	matrizes[posTile][ (i%tilesWidth)*tilesWidth + (j%tilesWidth)   ] = valor;
}


template <class T>
 inline T& tiledMatrix_acc<T>::operator() (unsigned int i, unsigned int j)
 {
	int posTile = posTiles[i/tilesWidth][j/tilesWidth];
	if (posTile == -1) {
		DBG(cerr << "carregando " <<endl;)
		loadTile(i/tilesWidth,j/tilesWidth);
		posTile = posTiles[i/tilesWidth][j/tilesWidth];
		DBG(cerr << "pos " << posTile << endl;)
	}
	timeStamps[posTile] = timeStamp++; //Marca o timestamp dele como um novo timestamp
	return matrizes[posTile][ (i%tilesWidth)*tilesWidth + (j%tilesWidth)   ];
 }
 




template <class T>
void tiledMatrix_acc<T>::gravaStatVezes(string arquivo) {
	/*ofstream saida(arquivo.c_str(),ios::out);
	saida<< "P2" <<endl;
	saida<< "# feep.pgm" <<endl;
	saida << nrowsTiles << " " << nrowsTiles << endl;
	int mx = -1;
	for(int i=0;i<nrowsTiles;i++)
		for(int j=0;j<ncolumnsTiles;j++)
			mx = max( numVezesTileCarregadoDisco[i][j],mx);
	saida << mx <<endl;
	for(int i=0;i<nrowsTiles;i++) {
		for(int j=0;j<ncolumnsTiles;j++)
			saida <<  numVezesTileCarregadoDisco[i][j] << ' ';
		saida << '\n';
	}
	saida <<flush;	
	saida.close();*/
}


template <class T>
void tiledMatrix_acc<T>::loadTile(int i,int j) {
    struct timeval start, end;

    long mtime, seconds, useconds;    

	//cerr << "aqui " <<endl;
    //gettimeofday(&start, NULL);
	//cerr << "ali " <<endl;
   
	loadTile2(i,j);

    //gettimeofday(&end, NULL);

    //seconds  = end.tv_sec  - start.tv_sec;
    //useconds = end.tv_usec - start.tv_usec;

    //mtime = ((seconds) * 1000000 + useconds) ;

	//cerr << mtime << endl;

    //tempoTotalCarregando += mtime;
}

//Private
template <class T>
void tiledMatrix_acc<T>::loadTile2(int i,int j) {
	//numVezesTileCarregadoDisco[i][j]++;

	tilesCarregadosMemoria++;
	tilesAbertos++;

	int id = 0;
	//Pegue o tile carregado (ou vago) menos recentemente utilizado...
	for(int k=0;k<numTilesMemoria;k++) {		
		DBG(cerr << k << " ---> " << tilesCarregados[k].first << " , " <<  tilesCarregados[k].second << endl;)
		if (timeStamps[k] < timeStamps[id]) 
			id = k;
	}

	DBG(cerr << "Carregando tile " << i << " , " << j << " no id " << id << endl;)
	//Se hÃ¡ algo carregado na posicao id... precisamos grava-lo!
	if (timeStamps[id]!=0) {
		DBG(cerr << "Removendo da memoria " <<endl;)
		DBG(cerr << tilesCarregados[id].first << " , " << tilesCarregados[id].second << endl;)
		removeTileDaMemoria(tilesCarregados[id].first,tilesCarregados[id].second); //Grava			
	} 
	
	timeStamps[id] = timeStamp++; //Marca o timestamp dele como um novo timestamp
	DBG(cerr << id << " :: " << i << endl;)
	tilesCarregados[id].first = i; //Agora o tile (i,j) estara carregado na posicao id
	tilesCarregados[id].second = j;
	posTiles[i][j] = id;
	
	//Carrega o tile (i,j) na posicao id...
	
	for(int q = 0;q < tilesWidth*tilesWidth ; q++)
	    matrizes[id][q] = Valor_Inicial;
	
	
	//unsigned long long tilePos = ((unsigned long long )i)*nrowsTiles+j;
	DBG(cerr << "lendo do disco.. para a matriz com id: " << id << endl;)

	
	//unsigned long long baseLinhaTile = ((unsigned long long )tilePos)*tilesWidth*tilesWidth;

	//arquivoAssociado.seekg(baseLinhaTile*sizeof(T),ios::beg);		
	//arquivoAssociado.read(reinterpret_cast<char *>(matrizes[id]) ,tilesWidth*tilesWidth*sizeof(T));		
	
	

	DBG(cerr << "fim " <<endl;)

	//cerr << "Total falhas: " << totalFalhasTiles << endl;
	//cerr << "Total Tiles carregados: "<< tilesCarregadosMemoria << endl;
	//cerr << "Tile " << i << " ,  " <<  j << " carregado"  << endl << endl;

}

template <class T>
void tiledMatrix_acc<T>::removeTileDaMemoria(int i,int j) {
	tilesGravadosMemoria++;
	totalFalhasTiles++;
	
	//cerr << "Tile " << i << " ,  " <<  j << " removido da memoria"  << endl << endl;

	int id = posTiles[i][j];	
		
	posTiles[tilesCarregados[id].first][tilesCarregados[id].second] =  -1; //Fala que ele nao estah mais carregado...
}


/*

int main(int argc, char **argv) {
	tiledMatrix_acc<int> matriz(1000,1000,100,10,"base_");
	matriz(10,10) = 5;
	cout << matriz(0,9) << " " << matriz(10,10);

	//cout << flush;
}


*/

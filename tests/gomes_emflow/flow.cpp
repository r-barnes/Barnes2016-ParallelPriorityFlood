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
#include "tiledMatrix_acc.cpp"

void writeFlowPgm(tiledMatrix_acc<int> &flow,const int nrows) ;
using namespace std;

int tilesWidth_Acc =  10000; // um tamanho que caiba na memoria junto com as bordas
int tamMemoria_Acc = 1600; // memoria disponivel 
int numTilesMemoriaFlow_Acc = 1; // deixar sempre 1, pois o algoritmo so usa 1 bloco por vez
int numTilesMemoria_Acc = 1; // deixar sempre 1, pois o algoritmo so usa 1 bloco por vez
int tilesWidthFlowIndegree_Acc = tilesWidth_Acc;
int numTilesMemoriaFlowIndegree_Acc = numTilesMemoria_Acc;//1225*6;


// Create a new array of size var of type.  Report error.
#define NEWA(var,type,size) { try  { if(0==(var=new type [(size)])) throw;} catch (...)  { cerr << "NEWA failed on " #var "=new " #type "[" #size "=" << (size) << "]" << endl; exit(1); }}


typedef  pair<int,int> pii;

class Point {
public:
  unsigned short int x,y;
 // short int z;
  Point() : x(0),y(0) {}
  Point(const short int y1, const short int x1) : 
    x(x1),y(y1) {}
};

class Borda { // guarda as bordas compartilhadas 
public:
  unsigned int nrows,ncolumns,tamanho_bloco,nlinhas,ncolunas;
  int **linha;
  int **coluna;
 // short int z;
   Borda(const int x1, const int y1, const int tamanho_bloco1){ 
               nrows = x1;
               ncolumns = y1;
               tamanho_bloco = tamanho_bloco1;
               nlinhas = nrows/tamanho_bloco + 1;
               ncolunas = ncolumns/tamanho_bloco + 1;
	       if(nrows%tamanho_bloco != 0)
		   nlinhas++;
               if(ncolumns%tamanho_bloco != 0)
		   ncolunas++;
	       
               linha =  new int *[nlinhas];
               
	           for(unsigned int i =0;i<nlinhas;i++) {
		           linha[i] = new int[ncolumns];
               }
               coluna =  new int *[ncolunas];
               
	           for(unsigned int i =0;i<ncolunas;i++) {
		           coluna[i] = new int[nrows];
               }
               
               
               
    }
        
    void set(unsigned int x, unsigned int y, int valor){
          if(x%tamanho_bloco== 0 )
               linha[x/tamanho_bloco][y] = valor;
          else if (x == nrows -1 ){
                linha[x/tamanho_bloco + 1][y] = valor;
          }
          else if(y%tamanho_bloco== 0 )
               coluna[y/tamanho_bloco][x] = valor;
          else if (y == ncolumns -1 ){
                coluna[y/tamanho_bloco + 1][x] = valor;
          } 
         
     }
     
     
     void set_Somar(unsigned int x, unsigned int y, int valor){
          if(x%tamanho_bloco== 0 )
               linha[x/tamanho_bloco][y] = linha[x/tamanho_bloco][y] + valor;
          else if (x == nrows -1 ){
                linha[x/tamanho_bloco + 1][y] = linha[x/tamanho_bloco + 1][y] + valor;
          }
          else if(y%tamanho_bloco== 0 )
               coluna[y/tamanho_bloco][x] = coluna[y/tamanho_bloco][x] + valor;
          else if (y == ncolumns -1 ){
                coluna[y/tamanho_bloco + 1][x] = coluna[y/tamanho_bloco + 1][x] + valor;
          } 
         
     }
          
     
    void set_All(int valor){
         
                for(unsigned int i =0;i<nlinhas;i++) {
                    for(unsigned int j = 0;j< ncolumns;j++)
		                linha[i][j] = valor;
               }
              
               
	           for(unsigned int i =0;i<ncolunas;i++) {
		            for(unsigned int j = 0;j< nrows;j++)
		                coluna[i][j] = valor;
               }
               
         
     }
        
     
      int get(unsigned int x, unsigned int y){
          if(x%tamanho_bloco== 0 )
               return linha[x/tamanho_bloco][y] ;
          else if (x == nrows -1 ){
                return linha[x/tamanho_bloco + 1][y] ;
          }
          else if(y%tamanho_bloco== 0 )
               return coluna[y/tamanho_bloco][x];
          else if (y == ncolumns -1 ){
                return coluna[y/tamanho_bloco + 1][x];
          } 
         
     }
    
    
};


void CacheAwareAccumulation(tiledMatrix<unsigned char> &dirs,const int nrows) {
    tiledMatrix_acc<int> flow( nrows, nrows,tilesWidthFlowIndegree_Acc,numTilesMemoriaFlow_Acc,"tiles/tile_flowAcc_" );;
    tiledMatrix_acc<char> inputDegree( nrows, nrows,tilesWidthFlowIndegree_Acc, numTilesMemoriaFlowIndegree_Acc,"tiles/tile_inputDegreeAcc_"); //Matrix that represents the input degree of each point in the terrain
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

   
   flow.set(1);//mudar para fazer quando tiver processando o bloco
	
   Borda Borda_flow(nrows, nrows,tilesWidthFlowIndegree_Acc);
   Borda_flow.set_All(1);
   
   Borda Borda_linha_dest(nrows, nrows,tilesWidthFlowIndegree_Acc);
   Borda Borda_coluna_dest(nrows, nrows,tilesWidthFlowIndegree_Acc); 
   
   Borda_linha_dest.set_All(-1);
   Borda_coluna_dest.set_All(-1);
   
   Borda Borda_inputDegree(nrows, nrows,tilesWidthFlowIndegree_Acc);
   Borda_inputDegree.set_All(0);                     
  
 // fase 1 
    for(int i=0;i<flow.nrowsTiles;i++) 
		for(int j=0;j<flow.ncolumnsTiles;j++) {
              //marca fase 1 -----------------------------------------------------------------
                          
              for(int p=1;p<flow.tilesWidth;p++)
		         for(int q=1;q <flow.tilesWidth;q++){
                           pii point(i*(flow.tilesWidth)+p,j*(flow.tilesWidth)+q);
                           if(point.first >= nrows - 1 || point.second >= nrows -1 ){
                               continue;
                           }
			               //flow[i][j] = 1;		
			            unsigned int ptDir = (unsigned int) dirs.get(point.first,point.second);
			            if (ptDir!=0) {			
				        if (point.first+directionToDyDx[ptDir][0] <0  || point.second+directionToDyDx[ptDir][1] <0 || point.first+directionToDyDx[ptDir][0] >=nrows -1  ||point.second+directionToDyDx[ptDir][1] >= nrows -1 || (point.first+directionToDyDx[ptDir][0])%flow.tilesWidth == 0 || (point.second+directionToDyDx[ptDir][1])%flow.tilesWidth == 0) 
					          continue;	
		                 //cout << point.first+directionToDyDx[ptDir][0] << "  " << point.second+directionToDyDx[ptDir][1]<< endl;
				        inputDegree( point.first+directionToDyDx[ptDir][0] , point.second+directionToDyDx[ptDir][1] )++; //Increases the input degree of the point that receives the flow from point (i,j)
			             
                    }
		      }
         //calcula fase 1 ------------------------------------------------------------------------------------   
                  
              for(int p=0;p <= flow.tilesWidth;p++)
		           for(int q=0;q <= flow.tilesWidth;q++){
                           pii point(i*(flow.tilesWidth)+p,j*(flow.tilesWidth)+q);
                           if (point.first >=nrows  || point.second >=nrows ) 
				   continue;
                           if(point.first%flow.tilesWidth==0 || point.first == nrows - 1 || point.second%flow.tilesWidth==0 || point.second == nrows - 1){
                               int dirPt = dirs.get(point.first,point.second);
			        if(dirPt!=0){
                               pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
			        if(neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1){
				     Borda_linha_dest.set(point.first,point.second,neighbor.first);
                                    Borda_coluna_dest.set(point.first,point.second,neighbor.second);
				}else if( neighbor.first/flow.tilesWidth == i && neighbor.second/flow.tilesWidth == j){ // faz destino somente para quem esta no bloco
                                   //cout << neighbor.first/flow.tilesWidth << i << neighbor.second/flow.tilesWidth << j<< endl;                  
                                   while(!(neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows || neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1)) {
                                         dirPt = dirs.get(neighbor.first,neighbor.second);
                                         pii neighbor1( neighbor.first+directionToDyDx[dirPt][0], neighbor.second+directionToDyDx[dirPt][1] );
                                         neighbor = neighbor1;       
                                   }
                                   Borda_linha_dest.set(point.first,point.second,neighbor.first);
                                   Borda_coluna_dest.set(point.first,point.second,neighbor.second);
                                   
                                }
				}
                               continue;
                           }
				           //Distributes the flow from point (i,j) to the neighbor of (i,j) that will receive its flow				
				           while(inputDegree( point.first , point.second )==0) {
                                   inputDegree( point.first , point.second ) = PROCESSED_POINT_FLAG; //Mark this point as processed	
					               int dirPt = dirs.get(point.first,point.second);	
					               pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
					               if (neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows ) 
						              break;
                                   if(neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1){
                                          Borda_flow.set_Somar(neighbor.first,neighbor.second,flow(point.first,point.second));
                                          break;
                                    }else{ 	
					                      flow( neighbor.first , neighbor.second )+= flow(point.first,point.second);
                                    }
					               inputDegree( neighbor.first , neighbor.second )--;
					               point = neighbor;
					               
					                //Repeat the process with the neighbor (if it has input degree equal to 0).
				                }	
                           
                           
                   } 
		      
    
    
    }
   
	    
 //	fase 2	
 
	// marcar vizinhos 
	
    
   for(unsigned int i = 0;i < Borda_inputDegree.nlinhas;i++)
          for(unsigned int j = 0;j < Borda_inputDegree.ncolumns;j++)
                if(Borda_linha_dest.linha[i][j] < 0 || 	Borda_linha_dest.linha[i][j] >= nrows || Borda_coluna_dest.linha[i][j] < 0 || 	Borda_coluna_dest.linha[i][j] >= nrows )
                     continue;
                else
                     Borda_inputDegree.set_Somar(Borda_linha_dest.linha[i][j],Borda_coluna_dest.linha[i][j],1);
   
   for(unsigned int i = 0;i < Borda_inputDegree.ncolunas;i++)
          for(unsigned int j = 0;j < Borda_inputDegree.nrows - 1;j++){
                if(j%Borda_inputDegree.tamanho_bloco == 0)
                     continue;
                if(Borda_linha_dest.coluna[i][j] < 0 || Borda_linha_dest.coluna[i][j] >= nrows || Borda_coluna_dest.coluna[i][j] < 0 || 	Borda_coluna_dest.coluna[i][j] >= nrows )
                     continue;
                else
                     Borda_inputDegree.set_Somar(Borda_linha_dest.coluna[i][j],Borda_coluna_dest.coluna[i][j],1);                
          }
		
	// calcular
     for(unsigned int i = 0;i < Borda_inputDegree.nlinhas;i++)
          for(unsigned int j = 0;j < Borda_inputDegree.ncolumns;j++){
                pii point((i*Borda_inputDegree.tamanho_bloco<nrows)?(i*Borda_inputDegree.tamanho_bloco):(nrows -1),j);
                while(Borda_inputDegree.get(point.first,point.second)==0) {
                                   Borda_inputDegree.set( point.first , point.second, 255); //Mark this point as processed	
					               pii neighbor( Borda_linha_dest.get(point.first , point.second), Borda_coluna_dest.get(point.first , point.second) );
					               if (neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows ) 
						              break;

					               Borda_flow.set_Somar( neighbor.first , neighbor.second,Borda_flow.get(point.first,point.second));
					               
                                   Borda_inputDegree.set_Somar( neighbor.first , neighbor.second,-1);
                                   point = neighbor;
					               
					                //Repeat the process with the neighbor (if it has input degree equal to 0).
				                }
           }
	
	
	 for(unsigned int i = 0;i < Borda_inputDegree.ncolunas;i++)
          for(unsigned int j = 0;j < Borda_inputDegree.nrows -1;j++){
                if(j%Borda_inputDegree.tamanho_bloco == 0)
                     continue;
                pii point(j,(i*Borda_inputDegree.tamanho_bloco<nrows)?(i*Borda_inputDegree.tamanho_bloco):(nrows -1));
                while(Borda_inputDegree.get(point.first,point.second)==0) {
                                   Borda_inputDegree.set( point.first , point.second, 255); //Mark this point as processed	
					               pii neighbor( Borda_linha_dest.get(point.first , point.second), Borda_coluna_dest.get(point.first , point.second) );
					               if (neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows ) 
						              break;

					               Borda_flow.set_Somar( neighbor.first , neighbor.second,Borda_flow.get(point.first,point.second));
					               
                                   Borda_inputDegree.set_Somar( neighbor.first , neighbor.second,-1);
                                   point = neighbor;
					               
					                //Repeat the process with the neighbor (if it has input degree equal to 0).
				                }
           }
		
		
		
		
// fase 3
       
          
    ofstream fcomp2("flow.hgt");
	unsigned long long tamanhoArquivo = ((unsigned long long)nrows)*nrows*2;
	short int c=0;
	fcomp2.seekp(tamanhoArquivo-2,ios::beg);
	fcomp2.write( reinterpret_cast<char *>( &c ),2);

	fcomp2.close();
	fcomp2.open("flow.hgt",ios::in | ios::out | ios::binary);
	assert(fcomp2.is_open());
	
	
	
		
	for(int i=0;i<flow.nrowsTiles;i++) 
		for(int j=0;j<flow.ncolumnsTiles;j++) {	
             
               
		//  setar o interior com 1 e 0
		for(int p=1;p<flow.tilesWidth;p++)
		               for(int q=1;q <flow.tilesWidth;q++){
                           pii point(i*(flow.tilesWidth)+p,j*(flow.tilesWidth)+q);
                           if(point.first >= nrows - 1 || point.second >=nrows -1 ){
                               continue;
                           }
			               flow(point.first,point.second) = 1;	
                           inputDegree(point.first,point.second) = 0;	             
                       }
      
   	        
		//--------------------------------------------
		// arrumar o interior pela borda-----------------------------
	     for(int p=0;p<=flow.tilesWidth;p++){
                 if((i*flow.tilesWidth+p)>= nrows)
                     break;
                 pii point((i*flow.tilesWidth+p),j*flow.tilesWidth); 
                 int dirPt = dirs.get(point.first,point.second);
                 pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
                 if( neighbor.first/flow.tilesWidth == i && neighbor.second/flow.tilesWidth == j){ // faz destino somente para quem esta no bloco
                      if(!(neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows || neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1)) {
                            flow(neighbor.first,neighbor.second) =   flow(neighbor.first,neighbor.second) + Borda_flow.get(point.first,point.second);  
                      }
                                                         
                 }
                 
                 
         }
     
         
		 for(int p=0;p<=flow.tilesWidth;p++){
                 if((i*flow.tilesWidth+p)>= nrows)
                     break;
                 pii point((i*flow.tilesWidth+p),((j*flow.tilesWidth + flow.tilesWidth)<nrows)?(j*flow.tilesWidth + flow.tilesWidth):(nrows -1) ); 
                 int dirPt = dirs.get(point.first,point.second);
                 pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
                 if( neighbor.first/flow.tilesWidth == i && neighbor.second/flow.tilesWidth == j){ // faz destino somente para quem esta no bloco
                      if(!(neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows || neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1)) {
                            flow(neighbor.first,neighbor.second) =   flow(neighbor.first,neighbor.second) + Borda_flow.get(point.first,point.second);  
                      }
                                                         
                 }
                 
                 
         }
        	
		 for(int p=1;p<flow.tilesWidth;p++){
                 if((j*flow.tilesWidth+p)>= nrows - 1)
                     break;
                 pii point(i*flow.tilesWidth,(j*flow.tilesWidth+p)); 
                 int dirPt = dirs.get(point.first,point.second);
                 pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
                 if( neighbor.first/flow.tilesWidth == i && neighbor.second/flow.tilesWidth == j){ // faz destino somente para quem esta no bloco
                      if(!(neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows || neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1)) {
                            flow(neighbor.first,neighbor.second) =   flow(neighbor.first,neighbor.second) + Borda_flow.get(point.first,point.second);  
                      }
                                                         
                 }
                 
                 
         }	
		 for(int p=1;p<flow.tilesWidth;p++){
                 if((j*flow.tilesWidth+p)>= nrows - 1 )
                     break;
                 pii point(((i*flow.tilesWidth + flow.tilesWidth)<nrows)?(i*flow.tilesWidth + flow.tilesWidth):(nrows -1),(j*flow.tilesWidth+p) ); 
                 int dirPt = dirs.get(point.first,point.second);
                 pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
                 if( neighbor.first/flow.tilesWidth == i && neighbor.second/flow.tilesWidth == j){ // faz destino somente para quem esta no bloco
                      if(!(neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows || neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1)) {
                            flow(neighbor.first,neighbor.second) =   flow(neighbor.first,neighbor.second) + Borda_flow.get(point.first,point.second);  
                      }
                                                         
                 }
                 
                 
         }
       		
		//----------------------------------------------------------
		
		 
		//marca fase 3 -----------------------------------------------------------------
                            
              for(int p=1;p<flow.tilesWidth;p++)
		               for(int q=1;q <flow.tilesWidth;q++){
                           pii point(i*(flow.tilesWidth)+p,j*(flow.tilesWidth)+q);
                           if(point.first >= nrows - 1 || point.second >= nrows ){
                               continue;
                           }
			               //flow[i][j] = 1;		
			            unsigned int ptDir = (unsigned int) dirs.get(point.first,point.second);
			            if (ptDir!=0) {			
				        if (point.first+directionToDyDx[ptDir][0] <0  || point.second+directionToDyDx[ptDir][1] <0 || point.first+directionToDyDx[ptDir][0] >=nrows -1  ||point.second+directionToDyDx[ptDir][1] >= nrows -1 || (point.first+directionToDyDx[ptDir][0])%flow.tilesWidth == 0 || (point.second+directionToDyDx[ptDir][1])%flow.tilesWidth == 0) 
					          continue;	
		                 //cout << point.first+directionToDyDx[ptDir][0] << "  " << point.second+directionToDyDx[ptDir][1]<< endl;
				        inputDegree( point.first+directionToDyDx[ptDir][0] , point.second+directionToDyDx[ptDir][1] )++; //Increases the input degree of the point that receives the flow from point (i,j)
			             
                    }
		      }
         //calcula fase 3 ------------------------------------------------------------------------------------   
                
              for(int p=0;p <= flow.tilesWidth;p++)
		           for(int q=0;q <= flow.tilesWidth;q++){
                           pii point(i*(flow.tilesWidth)+p,j*(flow.tilesWidth)+q);
                           if (point.first >=nrows  || point.second >=nrows ) 
						              continue;
                           if(point.first%flow.tilesWidth==0 || point.first == nrows - 1 || point.second%flow.tilesWidth==0 || point.second == nrows - 1){
                               continue;
                           }
				           //Distributes the flow from point (i,j) to the neighbor of (i,j) that will receive its flow				
				           while(inputDegree( point.first , point.second )==0) {
                                   inputDegree( point.first , point.second ) = PROCESSED_POINT_FLAG; //Mark this point as processed	
					               int dirPt = dirs.get(point.first,point.second);	
					               pii neighbor( point.first+directionToDyDx[dirPt][0], point.second+directionToDyDx[dirPt][1] );
					               if (neighbor.first <0  || neighbor.second <0 || neighbor.first >=nrows  || neighbor.second >=nrows ) 
						              break;
                                   if(neighbor.first%flow.tilesWidth==0 || neighbor.first == nrows - 1 || neighbor.second%flow.tilesWidth==0 || neighbor.second == nrows - 1){
                                          break;
                                    }else{ 	
					                      flow( neighbor.first , neighbor.second )+= flow(point.first,point.second);
                                    }
					               inputDegree( neighbor.first , neighbor.second )--;
					               point = neighbor;
					               
					                //Repeat the process with the neighbor (if it has input degree equal to 0).
				                }	
                           
                           
                   }
                   
                   
                   
            for(int p=0;p < flow.tilesWidth;p++){
	                  if(i*flow.tilesWidth +p >= nrows)
			      break;
	                   unsigned long long posi = (((unsigned long long )nrows)*(i*flow.tilesWidth +p) + j*flow.tilesWidth)*2;
			   fcomp2.seekp(posi,ios::beg);
	                  for(int q=0;q < flow.tilesWidth;q++){  
			      if(j*flow.tilesWidth + q >= nrows)
				  continue;
                             int flw ;
			       pii point(i*flow.tilesWidth +p, j*flow.tilesWidth +q);
			       if(point.first%flow.tilesWidth==0 || point.first == nrows - 1 || point.second%flow.tilesWidth==0 || point.second == nrows - 1)   
                                      flw =  Borda_flow.get(point.first,point.second);
                              else
                                       flw = flow(point.first,point.second);
			       short int tmp = flw>30000? 30000:flw;
			       fcomp2.write(reinterpret_cast<char *>(&tmp),2);    
                   
			   }
		}      
                   
                   
                   
                   
				
       }
      
      fcomp2.close();
      
          	
}




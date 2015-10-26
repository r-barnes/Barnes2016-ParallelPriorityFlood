#ifndef __distpdf_zhou2015pf_hp__
#define __distpdf_zhou2015pf_hp__

#include "Array2D.hpp"
#include "common.hpp"
#include <queue>
#include <vector>
#include <iostream> //TODO

template<class elev_t, class label_t>
label_t GetNewLabelZhou(int x, int y, label_t &current_label, uint8_t edge, const Array2D<elev_t> &dem, const Array2D<label_t> &labels){
  if(labels(x,y)!=0)
    return labels(x,y);

  if((edge & GRID_TOP) && y==0)
    return 1;

  if((edge & GRID_BOTTOM) && y==dem.viewHeight()-1)
    return 1;

  if((edge & GRID_LEFT) && x==0)
    return 1;

  if((edge & GRID_RIGHT) && x==dem.viewWidth()-1)
    return 1;

  for(int n=1;n<=8;n++){
    int nx = x+dx[n];
    int ny = y+dy[n];
    if(!dem.in_grid(nx,ny))
      continue;
    if(labels(nx,ny)!=0 && dem(nx,ny)<=dem(x,y))
      return labels(nx,ny);
  }

  return current_label++;
}

template<class elev_t, class label_t>
void WatershedsMeet(
  label_t my_label, label_t n_label, elev_t my_elev, elev_t n_elev,
  std::vector<std::map<label_t, elev_t> > &my_graph
){
  if(n_label==0)
    return;
  if(my_label==n_label)
    return;

  auto elev_over = std::max(my_elev,n_elev); //TODO: I think this should always be the neighbour.
  //If count()==0, then we haven't seen this watershed before.
  //Otherwise, only make a note of the spill-over elevation if it is
  //lower than what we've seen before.
  if(my_graph.at(my_label).count(n_label)==0 || elev_over<my_graph.at(my_label)[n_label]){
    my_graph.at(my_label)[n_label] = elev_over;
    my_graph.at(n_label)[my_label] = elev_over;
  }
}

template<class elev_t, class label_t>
void ProcessTraceQue_onepass(Array2D<elev_t> &dem, Array2D<label_t> &labels, std::queue<GridCellZ<elev_t> > &traceQueue, GridCellZ_pq<elev_t> &priorityQueue, std::vector<std::map<label_t, elev_t> > &my_graph){
  while (!traceQueue.empty()){
    GridCellZ<elev_t> c = traceQueue.front();
    traceQueue.pop();

    bool bInPQ = false;
    for(int n=1;n<=8;n++){
      int nx = c.x+dx[n];
      int ny = c.y+dy[n];

      if(!dem.in_grid(nx,ny))
        continue;

      WatershedsMeet(labels(c.x,c.y),labels(nx,ny),dem(c.x,c.y),dem(nx,ny),my_graph);

      if(labels(nx,ny)!=0)
        continue;    
      
      //The neighbour is unprocessed and higher than the central cell
      if(c.z<dem(nx,ny)){
        traceQueue.emplace(nx,ny,dem(nx,ny));
        labels(nx,ny) = labels(c.x,c.y);
        continue;
      }

      //Decide  whether (nx, ny) is a true border cell
      if (!bInPQ) {
        bool isBoundary = true;
        for(int nn=1;nn<=8;nn++){
          int nnx = nx+dx[n];
          int nny = ny+dy[n];
          if(!dem.in_grid(nnx,nny))
            continue;
          if (labels(nnx,nny)!=0 && dem(nnx,nny)<dem(nx,ny)){
            isBoundary = false;
            break;
          }
        }
        if(isBoundary){
          priorityQueue.push(c);
          bInPQ = true;
        }
      }
    }
  }
}

template<class elev_t, class label_t>
void ProcessPit_onepass(Array2D<elev_t> &dem, Array2D<label_t> &labels, std::queue<GridCellZ<elev_t> > &depressionQue, std::queue<GridCellZ<elev_t> > &traceQueue, GridCellZ_pq<elev_t> &priorityQueue, std::vector<std::map<label_t, elev_t> > &my_graph){
  while (!depressionQue.empty()){
    GridCellZ<elev_t> c = depressionQue.front();
    depressionQue.pop();

    for(int n=1;n<=8;n++){
      int nx = c.x+dx[n];
      int ny = c.y+dy[n];

      if(!dem.in_grid(nx,ny))
        continue;

      WatershedsMeet(labels(c.x,c.y),labels(nx,ny),dem(c.x,c.y),dem(nx,ny),my_graph);

      if(labels(nx,ny)!=0)
        continue;    

      labels(nx,ny) = labels(c.x,c.y);

      if (dem(nx,ny) > c.z) { //Slope cell
        traceQueue.emplace(nx,ny,dem(nx,ny));
      } else {                //Depression cell
        dem(nx,ny) = c.z;
        depressionQue.emplace(nx,ny,c.z);
      }
    }
  }
}

template<class elev_t, class label_t>
void Zhou2015Labels(
  Array2D<elev_t>                         &dem,
  Array2D<label_t>                        &labels,
  std::vector<std::map<label_t, elev_t> > &my_graph,
  uint8_t edge
){
  std::queue<GridCellZ<elev_t> > traceQueue;
  std::queue<GridCellZ<elev_t> > depressionQue;

  label_t current_label = 2;

  labels.init(0);

  GridCellZ_pq<elev_t> priorityQueue;

  for(int x=0;x<dem.viewWidth();x++){
    const int height = dem.viewHeight()-1;
    priorityQueue.emplace(x,0,     dem(x,0     ));
    priorityQueue.emplace(x,height,dem(x,height));
  }

  for(int y=1;y<dem.viewHeight()-1;y++){
    const int width = dem.viewWidth()-1;
    priorityQueue.emplace(0,    y,dem(0,    y));
    priorityQueue.emplace(width,y,dem(width,y));
  }

  while (!priorityQueue.empty()){
    GridCellZ<elev_t> c = priorityQueue.top();
    priorityQueue.pop();

    auto my_label = labels(c.x,c.y) = GetNewLabelZhou(c.x,c.y,current_label,edge,dem,labels);

    for(int n=1;n<=8;n++){
      int nx = c.x+dx[n];
      int ny = c.y+dy[n];

      if (!dem.in_grid(nx,ny))
        continue;

      WatershedsMeet(my_label,labels(nx,ny),dem(c.x,c.y),dem(nx,ny),my_graph);

      if(labels(nx,ny)!=0)
        continue;

      labels(nx,ny) = labels(c.x,c.y);

      if(dem(nx,ny)<=c.z){ //Depression cell
        dem(nx,ny) = c.z;
        depressionQue.emplace(nx,ny,c.z);
        ProcessPit_onepass(dem,labels,depressionQue,traceQueue,priorityQueue,my_graph);
      } else {          //Slope cell
        traceQueue.emplace(nx,ny,dem(nx,ny));
      }     
      ProcessTraceQue_onepass(dem,labels,traceQueue,priorityQueue,my_graph);
    }
  }

  std::cout<<current_label<<" labels used of "<<(2*dem.viewWidth()+2*dem.viewHeight())<<std::endl;
}

#endif
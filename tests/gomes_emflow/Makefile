CPP   =g++
FLAGS =-O3 -Wall

EMFlow: agendadorProcessamento.cpp lz4.h componentesProcessamento.cpp flow.cpp tiledMatrix_acc.cpp diffn.cpp hydrogEmFillFlow.cpp lz4.c tiledMatrix.cpp
	$(CPP) $(FLAGS) -o EMFlow hydrogEmFillFlow.cpp lz4.c

clean:
	rm -f EMFlow

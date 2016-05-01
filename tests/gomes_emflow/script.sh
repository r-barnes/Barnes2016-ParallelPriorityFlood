#terrenos em: "../terrains/"
#criar pasta tiles para armazenar os tiles temporarios
#criar pasta tempos para armazenar os tempos

mkdir tiles
mkdir tempos

#./hydrogEm_T_versaoEstavel200 30000 ../terrains/terrainR2_30000.hgt ;
#mv flow.hgt flow_R2_30000.hgt;

#./hydrogEm_T_versaoEstavel200 30000 ../terrains/terrainR3_30000.hgt ;
#mv flow.hgt flow_R3_30000.hgt;

#Compilando
#g++ -O3 -o hydrogEm_300_2GB tiledMatrix.cpp hydrogEmFillFlow.cpp
#chmod 777 hydrogEm_400_2GB



arquivoResumoTempos='tempos/resumoComponentes.txt'
arquivoDiffTempos='tempos/diff.txt'

for  regiao in R3
do
	for metodo in EMflow_CAA  
	do
		for j in 50000 
		do
			rm tiles/*
			sleep 30

			  arquivo="tempo'_'$metodo'_'$regiao'_'$j.txt" 
			  terrenoEntrada='../terrains/terrain'$regiao'_'$j'.hgt'
			  
		          
			  echo $arquivo
			  echo $terrenoEntrada
		          echo $arquivoResumoTempos
			  
			  (time -p  ./$metodo $j $terrenoEntrada ) 2> tempos/$arquivo >> $arquivoResumoTempos  ;
			  echo $terrenoEntrada >> $arquivoDiffTempos;
                          diff "flow.hgt" "CacheAwareAccumulation.hgt" >> $arquivoDiffTempos;
			  
		done;
	done;
done;




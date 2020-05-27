#!/bin/bash

for i in {1..5}
do
echo "<== Execução ${i} ==>" >> resultados.txt
./Tarefa2 >> resultados.txt & sleep 1
kill Tarefa2
done

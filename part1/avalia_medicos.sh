#!/bin/bash

if [ -f lista_negra_medicos.txt ]; then rm lista_negra_medicos.txt; fi

awk -F';' '{if($6>6 && $5<5) print}' medicos.txt > lista_negra_medicos.txt
echo "Médicos que não cumprem os requisitos foram adicionados com sucesso."; echo

echo "-- \"lista negra\" de médicos inscritos na plataforma --"
cat lista_negra_medicos.txt
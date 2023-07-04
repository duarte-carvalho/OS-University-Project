#!/bin/bash

if [ $# -eq 2 ]; then
    echo -n "Número de pacientes inscritos na plataforma ($1): "
    awk -F';' '{if(tolower($3) == tolower('\"$1\"')) print}' pacientes.txt | wc -l
    
    echo -n "Número de Médicos inscritos na plataforma com saldo superior a $2: "
    awk -F';' '{if($NF>'$2') print}' medicos.txt | wc -l
else echo "Utilização: $0 <Localidade> <saldo>"; fi
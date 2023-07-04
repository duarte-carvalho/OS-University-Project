#!/bin/bash

function checkArguments {
    if [[ $1 =~ ^[a-zA-Z\ ]+$ && $3 =~ ^[a-zA-Z\ ]+$ && $2 =~ ^[0-9]+$ ]]; then
        if [[ $4 =~ ^[a-zA-Z0-9]+'@'+[a-zA-Z]+'.'+[a-zA-Z]+$ ]]; then
            return 0
        else echo "Email inválido."; fi
    else echo "Nome, nº de cédula ou especialidade inválidos."; fi

    return 1
}

if [ ! -f medicos.txt ]; then touch medicos.txt; fi

if [ $# -eq 4 ]; then
    if checkArguments "$1" "$2" "$3" "$4"; then
        if [ ! -z "$(awk -F';' '{if($0 ~ /^'"$1;"$2";$3;$4"'/) print}' medicos.txt)" ]; then   
            echo "O médico $1 já se encontra registado."; echo
        else
            echo "$1;$2;$3;$4;0;0;0" >> medicos.txt; 
            echo "Médico registado com sucesso."; echo
        fi
        echo "-- Lista de médicos registados -- "; cat medicos.txt
    fi
else
    echo "Utilização: $0 <nome> <número cédula profissional> <especialidade médica> <e-mail>"
fi
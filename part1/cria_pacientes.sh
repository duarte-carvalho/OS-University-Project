#!/bin/bash

if [ -f pacientes.txt ]; then rm pacientes.txt; fi
touch pacientes.txt

awk -F[:,,,] '{OFS=";";if($1 ~ /^a[0-9]+$/) print $1,$5,";;"$1"@iscte-iul.pt",100}' \
/etc/passwd | head -10 | sed 's/^a//' | awk -F[\ ] '{if(NF>1)print $1,$NF;else print $1}' >> pacientes.txt

echo "Pacientes criados com sucesso!"
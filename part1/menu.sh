#!/bin/bash

while true; do
    echo -e "\e[34m\e[1m-- Cliniq IUL --\e[0m"
    echo -e "1. Cria pacientes\n2. Cria médicos\n3. Stats\n4. Avalia médicos\n0. Sair"
    read user_input

    case $user_input in
        1) ./cria_pacientes.sh; echo;;
        2) 
        read -p "Nome: " nome; read -p "Nr. Cedula Profissional: " nr
        read -p "Especialidade: " especialidade; read -p "Email: " email
        ./cria_medico.sh "$nome" "$nr" "$especialidade" "$email"; echo;;
        3)
        read -p "Localidade: " localidade; read -p "Saldo: " saldo
        ./stats.sh $localidade $saldo; echo;;
        4) ./avalia_medicos.sh; echo;;
        0) break;;
        *) echo "Erro. Insira apenas o número da opção"; echo;;
    esac
done
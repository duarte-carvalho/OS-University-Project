#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "consulta.h"

#define MAX_CONSULTAS 10

int NORMAL = 0;
int COVID19 = 0;
int URGENTE = 0;
int CONSULTAS_PERDIDAS = 0;

Consulta lista_consultas[MAX_CONSULTAS];

int verifica_listaConsultas() {
    for(int i = 0; i < MAX_CONSULTAS; i++) 
        if(lista_consultas[i].tipo == -1)
            return i+1;
    return 0;
}

void incrementa_consulta(int tipo) {
    if(tipo == 1) NORMAL += 1;
    if(tipo == 2) COVID19 += 1;
    if(tipo == 3) URGENTE += 1;
}

void trata_sinal(int signal) {
    Consulta consulta, limpar_consulta;
    FILE *fp = fopen("PedidoConsulta.txt", "r");

    if(!(fp == NULL)) { 
        while(fread(&consulta, sizeof(Consulta), 1, fp)); fclose(fp);
        printf("Chegou novo pedido de consulta do tipo %d, %s e PID %d\n", consulta.tipo, consulta.descricao, consulta.pid_consulta);

        if(verifica_listaConsultas()) {
            int pos = verifica_listaConsultas()-1; lista_consultas[pos] = consulta; 
            printf("Consulta agendada para a sala %d\n", pos);
            incrementa_consulta(consulta.tipo);

            int pid = fork();

            if(pid == 0) {
                kill(consulta.pid_consulta, SIGHUP); sleep(10);
                printf("Consulta terminada na sala %d\n", pos);
                kill(consulta.pid_consulta, SIGTERM);
                exit(0);
            } 
            
            wait(NULL);
            lista_consultas[pos] = limpar_consulta; lista_consultas[pos].tipo = -1;
        } else {
            printf("Lista de consultas cheia\n");
            kill(consulta.pid_consulta, SIGUSR2);
            CONSULTAS_PERDIDAS += 1;
        }
    }
}

void trata_sinal_ctrlc(int signal) {
    remove("SrvConsultas.pid");
    Stats consultas; 
    FILE *read_values;

    if((read_values = fopen("StatsConsultas.dat", "rb")) != NULL) {
        fread(&consultas, sizeof(Stats), 1, read_values);

        consultas.normal += NORMAL; consultas.covid19 += COVID19; 
        consultas.urgente += URGENTE; consultas.consultas_perdidas += CONSULTAS_PERDIDAS;
    } else {
        consultas.normal = NORMAL; consultas.covid19 = COVID19; 
        consultas.urgente = URGENTE; consultas.consultas_perdidas = CONSULTAS_PERDIDAS;
    }

    FILE *write_values = fopen("StatsConsultas.dat", "wb");
    fwrite(&consultas, sizeof(Stats), 1, write_values);
    fclose(write_values);

    exit(0);
}

int main() {
    for(int i = 0; i < 10; i++) lista_consultas[i].tipo = -1; 

    FILE *fp = fopen("SrvConsultas.pid", "w");
    fprintf(fp, "%d", getpid()); fclose(fp);

    signal(SIGUSR1, trata_sinal); 
    signal(SIGINT, trata_sinal_ctrlc); 

    while(1) pause();
}
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "consulta.h"

int verifica_sinal = 0;
int verifica_ficheiro = 0;

void trata_sinal_SIGHUP(int signal) {
    printf("Consulta iniciada para o processo %d\n", getpid());
    remove("PedidoConsulta.txt");
    verifica_sinal = 1;
}

void trata_sinal_SIGTERM(int signal) {
    if(verifica_sinal) {
        printf("Consulta concluída para o processo %d\n", getpid());
        exit(0);
    } else printf("Erro. A consulta ainda não foi iniciada.\n");
}

void trata_sinal_SIGUSR2(int signal) {
    if(!verifica_sinal) {
        printf("Consulta não é possível para o processo %d\n", getpid());
        remove("PedidoConsulta.txt");
        exit(0);
    }
}

void trata_sinal_SIGINT(int signal) {
    if(!verifica_ficheiro || verifica_sinal) {
        printf("Paciente cancelou o pedido.\n");
        exit(0);
    }

    printf("Paciente cancelou o pedido.\n");
    remove("PedidoConsulta.txt");
    exit(0);
}

int main() {
    int tipo;
    char descricao[100], tmp[10];

    signal(SIGINT, trata_sinal_SIGINT);

    while(1) {
        printf("Preencha os seguintes campos.\n");
        printf("Tipo: "); fgets(tmp, sizeof(tmp), stdin); tipo = atoi(tmp);
        printf("Descrição: "); fgets(descricao, sizeof(descricao), stdin);
        descricao[strlen(descricao)-1] = '\0';

        if(tipo == 1 || tipo == 2 || tipo == 3) break;
        printf("Tipo inválido. Tente novamente.\n\n");
    }
    
    Consulta consulta; consulta.tipo = tipo; strcpy(consulta.descricao, descricao); 
    consulta.pid_consulta = getpid();

    // (C8) verifica se o ficheiro já existe antes de criar um novo para evitar escrever
    // por cima de outro pedido
    if(access("PedidoConsulta.txt", F_OK) == 0) {
        while(1) {
            printf("A registar consulta. Porfavor aguarde..(ctrl-c para sair)\n"); sleep(10);

            if(access("PedidoConsulta.txt", F_OK) != 0)
                break;
        }
    }

    FILE *criar_consulta = fopen("PedidoConsulta.txt", "w"); 
    if(!(criar_consulta == NULL)) {
        fwrite(&consulta, sizeof(Consulta), 1, criar_consulta);
        fclose(criar_consulta);

        verifica_ficheiro = 1;
    } else {
        perror("Erro ao abrir o ficheiro criar_consulta.");
        exit(1);
    }

    FILE *ler_pid = fopen("SrvConsultas.pid", "r"); int pid;
    fscanf(ler_pid, "%d", &pid); fclose(ler_pid);

    kill(pid, SIGUSR1);

    printf("A consulta irá começar brevemente. (ctrl-c para cancelar)\n");

    signal(SIGHUP, trata_sinal_SIGHUP);
    signal(SIGTERM, trata_sinal_SIGTERM);
    signal(SIGUSR2, trata_sinal_SIGUSR2);

    while(1) pause();
}

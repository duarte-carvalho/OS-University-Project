#include "servidor.h"

#include <errno.h>

int CONSULTA_INICIADA = 0;

void trata_sinal(int signal) {
    if(CONSULTA_INICIADA) {
        printf("\nPaciente cancelou o pedido\n");

        int id = msgget(IPC_KEY, 0);
        exit_on_error(id, "O cliente não se conseguiu ligar ao servidor.");

        mensagem m;
        m.tipo = getpid();
        m.consulta.status = 5;

        int status = msgsnd(id, &m, sizeof(m.consulta), 0);
        exit_on_error(status, "erro ao enviar");

        exit(0);
    }
}

void trata_sinal_alarme(int signal) {
    int id = msgget(IPC_KEY, 0);
    exit_on_error(id, "O cliente não se conseguiu ligar ao servidor.");

    mensagem m;
    m.tipo = getpid();
    m.consulta.status = 0;

    int status = msgsnd(id, &m, sizeof(m.consulta), 0);
    exit_on_error(status, "erro ao enviar");
}

int main(int argc, char *argv[]) {
    int tipo; char descricao[100], tmp[10];
    Consulta consulta; mensagem m;

    signal(SIGINT, trata_sinal);
    signal(SIGALRM, trata_sinal_alarme);

    while(1) {
        printf("Preencha os seguintes campos.\n");
        printf("Tipo: "); fgets(tmp, sizeof(tmp), stdin); tipo = atoi(tmp);
        printf("Descrição: "); fgets(descricao, sizeof(descricao), stdin);
        descricao[strlen(descricao)-1] = '\0';

        if(tipo == 1 || tipo == 2 || tipo == 3) break;
        printf("Tipo inválido. Tente novamente.\n\n");
    }

    consulta.tipo = tipo; strcpy(consulta.descricao, descricao); 
    consulta.pid_consulta = getpid();

    int id = msgget( IPC_KEY, 0 );
    exit_on_error(id, "O cliente não se conseguiu ligar ao servidor.");

    m.tipo = MSG_TYP;
    m.consulta = consulta;
    m.consulta.status = 1;

    int status = msgsnd(id, &m, sizeof(m.consulta), 0);
    exit_on_error(status, "erro ao enviar");

    while(1) {
        status = msgrcv(id, &m, sizeof(m.consulta), m.consulta.pid_consulta, 0);

        if(m.consulta.status == 2) {
            if(!CONSULTA_INICIADA) {
                printf("Consulta iniciada para o processo %d\n", m.consulta.pid_consulta);

                CONSULTA_INICIADA = 1; alarm(10);
            }
        }

        if(m.consulta.status == 3) {
            if(CONSULTA_INICIADA) {
                printf("Consulta concluída para o processo %d\n", m.consulta.pid_consulta);
                exit(0);
            } else printf("Erro a consulta ainda não foi iniciada.\n");
        }

        if(m.consulta.status == 4) {
            printf("Consulta não é possível para o processo %d\n", m.consulta.pid_consulta);
            break;
        }
    }

    exit(0);
}

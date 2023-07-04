#include "servidor.h"

typedef struct {
    int contador;
} contadores;

/*
contador[0] - NORMAL
contador[1] - COVID19
contador[2] - URGENTE
contador[3] - CONSULTA_PERDIDA 
*/ 

struct sembuf DOWN = { .sem_op = -1 };
struct sembuf UP = { .sem_op = +1 };

void iniciar_consultas() {
    int sem_id = semget( IPC_KEY, 1, IPC_CREAT | 0600 );
    exit_on_error(sem_id, "semget");

    int status = semctl(sem_id, 0, SETVAL, 0);
    exit_on_error(status, "semctl(SETVAL)");

    int mem_id = shmget( IPC_KEY, MAX_CONSULTAS * sizeof(Consulta), IPC_CREAT | IPC_EXCL | 0600 );

    if(mem_id > 0) {
        Consulta* c = (Consulta *) shmat(mem_id, NULL, 0);
        exit_on_null(c, "shmat");

        for(int i = 0; i < MAX_CONSULTAS; i++) 
            c[i].tipo = -1;
    }

    status = semctl(sem_id, 0, SETVAL, 1);
    exit_on_error(status, "semctl(SETVAL)");
}

void inicar_contadores() {
    int sem_id = semget( IPC_KEY, 1, 0 );
    exit_on_error(sem_id, "semget");

    int status = semctl(sem_id, 0, SETVAL, 0);
    exit_on_error(status, "semctl(SETVAL)");

    int mem_id = shmget( 9000, 4 * sizeof(contadores), IPC_CREAT | IPC_EXCL | 0600 );

    if(mem_id > 0) {
        contadores* c = (contadores *) shmat(mem_id, NULL, 0);
        exit_on_null(c, "shmat");

        for(int i = 0; i < 4; i++) 
            c[i].contador = 0;
    }

    status = semctl(sem_id, 0, SETVAL, 1);
    exit_on_error(status, "semctl(SETVAL)");
}

int verificar_vagas(Consulta consulta) {
    int mem_id = shmget(IPC_KEY, MAX_CONSULTAS * sizeof(Consulta), 0);
    exit_on_error(mem_id, "shmget");

    Consulta* c = (Consulta *) shmat(mem_id, NULL, 0);
    exit_on_null(c, "shmat");

    int sem_id = semget(IPC_KEY, 1, 0);
    exit_on_error(sem_id, "semget");

    int status = semop(sem_id, &DOWN, 1);
    exit_on_error(status, "DOWN");

    int indice = -1;
    for(int i = 0; i < MAX_CONSULTAS; i++) {
        if(c[i].tipo == -1) {
            c[i] = consulta;
            indice = i;
            
            break;
        }
    }

    status = semop(sem_id, &UP, 1);
    exit_on_error(status, "UP");

    return indice;
}

void incrementar_contador(int tipo) {
    int mem_id = shmget(9000, 4 * sizeof(contadores), 0);
    exit_on_error(mem_id, "shmget");

    contadores* c = (contadores *) shmat(mem_id, NULL, 0);
    exit_on_null(c, "shmat");

    int sem_id = semget(IPC_KEY, 1, 0);
    exit_on_error(sem_id, "semget");

    int status = semop(sem_id, &DOWN, 1);
    exit_on_error(status, "DOWN");

    c[tipo].contador += 1;

    status = semop(sem_id, &UP, 1);
    exit_on_error(status, "UP");
}

void apagar_consulta(int indice) {
    int mem_id = shmget(IPC_KEY, MAX_CONSULTAS * sizeof(Consulta), 0);
    exit_on_error(mem_id, "shmget");

    Consulta* c = (Consulta *) shmat(mem_id, NULL, 0);
    exit_on_null(c, "shmat");

    int sem_id = semget(IPC_KEY, 1, 0);
    exit_on_error(sem_id, "semget");

    int status = semop(sem_id, &DOWN, 1);
    exit_on_error(status, "DOWN");

    c[indice].tipo = -1;

    status = semop(sem_id, &UP, 1);
    exit_on_error(status, "UP");
}

void trata_sinal(int signal) {
    int mem_id = shmget(9000, 4 * sizeof(contadores), 0);
    exit_on_error(mem_id, "shmget");

    contadores* c = (contadores *) shmat(mem_id, NULL, 0);
    exit_on_null(c, "shmat");

    printf("\nNORMAL - %d\tCOVID19 - %d\tURGENTE - %d\tCONSULTAS_PERDIDAS - %d\n", c[0].contador, c[1].contador, c[2].contador, c[3].contador);
    exit(0);
}

int main() {
    iniciar_consultas(); inicar_contadores();
    printf("Memória iniciada.\n");

    int id, status;
    id = msgget( IPC_KEY, IPC_CREAT | IPC_EXCL | 0600 );

    if(id == -1) id = msgget(IPC_KEY, 0 | 0600);

    signal(SIGINT, trata_sinal);
    signal(SIGCHLD, SIG_IGN);

    mensagem m;
    while(1) {
        status = msgrcv(id, &m, sizeof(m.consulta), MSG_TYP, 0);
        exit_on_error(status, "erro ao receber");

        if(m.consulta.status == 1) {
            printf("Chegou novo pedido de consulta do tipo %d, descrição %s e PID %d\n", m.consulta.tipo, m.consulta.descricao, m.consulta.pid_consulta);
            int pid = fork();

            if(pid == 0) {
                int indice = verificar_vagas(m.consulta);
                
                if(indice != -1) {
                    printf("Consulta agendada para a sala %d\n", indice);
                    incrementar_contador(m.consulta.tipo-1);

                    m.tipo = m.consulta.pid_consulta;
                    m.consulta.status = 2;

                    status = msgsnd(id, &m, sizeof(m.consulta), 0);
                    exit_on_error(status, "erro ao enviar");

                    sleep(1);

                    mensagem d; d.tipo = 0; d.consulta.status = 0;

                    status = msgrcv(id, &d, sizeof(d.consulta), m.consulta.pid_consulta, 0);
                    exit_on_error(status, "erro ao receber"); 

                    if(d.consulta.status == 5) {
                        printf("Consulta cancelada pelo utilizador %d\n", m.consulta.pid_consulta);
                        apagar_consulta(indice); exit(0);
                    }

                    printf("Consulta terminada na sala %d\n", indice);

                    m.consulta.status = 3;
                    status = msgsnd(id, &m, sizeof(m.consulta), 0);

                    apagar_consulta(indice); exit(0);
                } else {
                    printf("Lista de consultas cheia\n");

                    m.tipo = m.consulta.pid_consulta;
                    m.consulta.status = 4;
                    status = msgsnd(id, &m, sizeof(m.consulta), 0);

                    incrementar_contador(3);

                    exit(0);
                }
            }
        }
    }
}
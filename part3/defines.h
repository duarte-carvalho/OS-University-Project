#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define exit_on_error(s,m) if (s<0) {perror(m); exit(1);}
#define exit_on_null(s,m) if (s==NULL) { perror(m); exit(1); }

#define IPC_KEY 0x0000000 // 0x0nraluno
#define MSG_TYP 1
#define MAX_CONSULTAS 10

#endif
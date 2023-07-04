#ifndef __SERVIDOR_H__
#define __SERVIDOR_H__

#include "defines.h"

typedef struct {
 int tipo; 
 char descricao[100]; 
 int pid_consulta; 
 int status; 
} Consulta; 

typedef struct {
    long tipo;
    Consulta consulta;
} mensagem;

#endif
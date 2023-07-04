typedef struct {
    int tipo;
    char descricao[20];
    int pid_consulta;
} Consulta;

typedef struct {
	int consultas_perdidas;
	int normal;
	int covid19;
	int urgente;
} Stats;
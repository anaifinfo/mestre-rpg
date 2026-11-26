#ifndef GRUPO_H
#define GRUPO_H

#include "../personagens_e_equipamentos/personagem.h"

#define MAX_PERSONAGENS_GRUPO 6

typedef struct {
    int idsPersonagens[MAX_PERSONAGENS_GRUPO];
    int quantidade;
    int poderTotal;
} GrupoAventura;

// Protótipos obrigatórios
void inicializaGrupo(GrupoAventura *grupo);
int adicionaPersonagemGrupo(GrupoAventura *grupo, const CadastroPersonagens *cadastro, int idPersonagem);
int removePersonagemGrupo(GrupoAventura *grupo, int idPersonagem);
int calculaPoderGrupo(GrupoAventura *grupo, const CadastroPersonagens *cadastro);
void exibeGrupo(const GrupoAventura *grupo, const CadastroPersonagens *cadastro);

#endif // GRUPO_H
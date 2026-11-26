#ifndef ENCONTRO_H
#define ENCONTRO_H

#include "../bestiario/monstro.h"
#include "../bestiario/bestiario.h"
#include "grupo.h"

#define MAX_CANDIDATOS MAX_MONSTROS
#define MAX_ESPECIES_ENCONTRO 4
#define MAX_MONSTROS_ENCONTRO 50
#define NUM_PROPOSTAS 3

typedef enum {
    FACIL,
    MEDIA,
    DIFICIL
} Dificuldade;

typedef enum {
    PERIODO_DIA,
    PERIODO_NOITE
} Periodo;

typedef struct {
    Terreno terreno;
    Periodo periodo;
    Dificuldade dificuldade;
} ContextoEncontro;

typedef struct {
    const Monstro *itens[MAX_CANDIDATOS];
    int quantidade;
} ListaCandidatos;

typedef struct {
    int idMonstro;
    int quantidade;
    int poderCalculado;
} ComponenteEncontro;

typedef struct {
    ComponenteEncontro componentes[MAX_ESPECIES_ENCONTRO];
    int quantidadeEspecies;
    int quantidadeMonstros;
    int poderTotal;
} Encontro;

typedef struct {
    Encontro propostas[NUM_PROPOSTAS];
    int quantidade;
    int indiceEscolhido;
} ConjuntoPropostas;

// --- Contexto, filtro e poder ---
void calculaFaixaDificuldade(int poderGrupo, Dificuldade dificuldade, int *minimo, int *maximo);
int monstroCompativel(const Monstro *m, const ContextoEncontro *contexto);
int filtraCandidatos(const Bestiario *b, const ContextoEncontro *contexto, ListaCandidatos *saida);
int poderIndividualContextual(const Monstro *m, const ContextoEncontro *contexto);
int poderComponente(const Monstro *m, int quantidade, const ContextoEncontro *contexto);
int calculaPoderEncontro(Encontro *e, const Bestiario *b, const ContextoEncontro *contexto);

// --- Sorteio e geracao ---
const Monstro *sorteiaMonstroPonderado(const ListaCandidatos *candidatos);
int tentaGerarEncontro(const Bestiario *b, const ListaCandidatos *candidatos,
                       const ContextoEncontro *contexto, int poderMinimo,
                       int poderMaximo, Encontro *saida);
int encontrosIguais(const Encontro *a, const Encontro *b);
int geraPropostas(const Bestiario *b, const GrupoAventura *grupo,
                  const ContextoEncontro *contexto, ConjuntoPropostas *saida);
int escolheProposta(ConjuntoPropostas *conjunto, int opcao);
void exibeEncontro(const Encontro *e, const Bestiario *b);

#endif // ENCONTRO_H
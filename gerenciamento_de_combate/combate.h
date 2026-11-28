#ifndef COMBATE_H
#define COMBATE_H

#include "personagem.h"
#include "grupo.h"
#include "encontro.h"
#include "bestiario.h"
#include "dados.h"

#define MAX_COMBATENTES \
    (MAX_PERSONAGENS_GRUPO + MAX_MONSTROS_ENCONTRO)
#define MAX_RODADAS 100

typedef enum {
    COMBATENTE_ATIVO,
    COMBATENTE_DERROTADO,
    COMBATENTE_FUGIU
} EstadoCombatente;

typedef enum {
    LADO_PERSONAGENS,
    LADO_MONSTROS,
    LADO_NENHUM
} LadoCombate;

typedef struct {
    Personagem *ficha;
    int pontosVidaMaximos;
    int pontosVidaAtuais;
    int iniciativa;
    EstadoCombatente estado;
} PersonagemCombate;

typedef struct {
    int numeroExemplar;
    const Monstro *especie;
    int pontosVidaMaximos;
    int pontosVidaAtuais;
    int iniciativa;
    EstadoCombatente estado;
} MonstroCombate;

typedef struct {
    LadoCombate lado;
    int indice;
    int iniciativa;
} EntradaOrdem;

typedef struct {
    PersonagemCombate personagens[MAX_PERSONAGENS_GRUPO];
    int quantidadePersonagens;
    MonstroCombate monstros[MAX_MONSTROS_ENCONTRO];
    int quantidadeMonstros;
    EntradaOrdem ordem[MAX_COMBATENTES];
    int quantidadeOrdem;
    int rodadaAtual;
    int encerrado;
    LadoCombate vencedor;
} Combate;

/* Preparação e consulta */
void inicializaCombate(Combate *combate);
int preparaCombate(Combate *combate,
                   CadastroPersonagens *cadastro,
                   const GrupoAventura *grupo,
                   const Encontro *encontro,
                   const Bestiario *bestiario);
int contaPersonagensAtivos(const Combate *combate);
int contaMonstrosAtivos(const Combate *combate);
int combateTerminou(Combate *combate);

/* Iniciativa e alvo */
void rolaIniciativas(Combate *combate);
void montaOrdemCombate(Combate *combate);
void exibeOrdem(const Combate *combate);
int selecionaPersonagemAlvo(const Combate *combate);
int selecionaMonstroAlvo(const Combate *combate);

/* Ataque e dano */
int defesaEquivalenteMonstro(const Monstro *monstro);
int bonusAtaqueMonstro(const Monstro *monstro);
int verificaAcerto(int resultadoD20,
                   int bonusAtaque, int defesaAlvo);
int aplicaDanoPersonagem(PersonagemCombate *alvo, int dano);
int aplicaDanoMonstro(MonstroCombate *alvo, int dano);
int executaAtaquePersonagem(Combate *combate,
                            int atacante, int alvo);
int executaAtaquesMonstro(Combate *combate,
                          int atacante, int alvo);

/* Fluxo e apresentação */
int executaTurno(Combate *combate, int posicaoOrdem);
int executaRodada(Combate *combate);
int executaCombate(Combate *combate);
void exibeEstadoCombate(const Combate *combate);
void exibeResumoCombate(const Combate *combate);
int calculaXPDerrotados(const Combate *combate);
int atualizaPersonagensAposCombate(const Combate *combate);

/* Menu chamado pelo main.c */
void menuCombateFluxo(Combate *combate,
                      CadastroPersonagens *cadastro,
                      const GrupoAventura *grupo,
                      const Encontro *encontro,
                      const Bestiario *bestiario);

#endif
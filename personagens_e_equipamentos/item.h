#ifndef ITEM_H
#define ITEM_H

#include "dados.h"

/* Tipos de itens/equipamentos obrigatórios exigidos pelo enunciado */
typedef enum {
    ELMO,
    PEITORAL,
    MANOPLAS,
    CALCA,
    BOTAS,
    ANEL,
    COLAR,
    CINTO,
    ARMA_UMA_MAO,
    ARMA_DUAS_MAOS
} TipoItem;

/* Estrutura que representa um item individual na mochila ou equipado */
typedef struct {
    int id;
    char nome[50];
    TipoItem tipo;
    int espacos;
    int bonusAtaque;
    int bonusDefesa;
    int bonusVida;
    int bonusIniciativa;
    int poder;
    Dado dano;
} Item;

/* Função auxiliar para validar se os dados de um item respeitam as regras */
int itemValido(Item item);

/* Função auxiliar para converter o tipo do item em string legível para exibição */
const char *nomeTipoItem(TipoItem tipo);

#endif
#include "grupo.h"
#include "../personagens_e_equipamentos/personagem.h"
#include <stdio.h>
#include <stddef.h>

void inicializaGrupo(GrupoAventura *grupo) {
    if (grupo == NULL) {
        return;
    }

    grupo->quantidade = 0;
    grupo->poderTotal = 0;

    /* Zera os IDs por seguranca */
    for (int i = 0; i < MAX_PERSONAGENS_GRUPO; i++) {
        grupo->idsPersonagens[i] = 0;
    }
}

int adicionaPersonagemGrupo(GrupoAventura *grupo, const CadastroPersonagens *cadastro, int idPersonagem) {
    if (grupo == NULL || cadastro == NULL) {
        return 0;
    }

    /* Impede adicao se a capacidade maxima (6) ja foi atingida */
    if (grupo->quantidade >= MAX_PERSONAGENS_GRUPO) {
        return 0;
    }

    /* Impede repeticao verificando se o ID ja existe no vetor */
    for (int i = 0; i < grupo->quantidade; i++) {
        if (grupo->idsPersonagens[i] == idPersonagem) {
            return 0; /* Falha: personagem repetido */
        }
    }

    /* Valida se o personagem existe no cadastro da Etapa 1 */
    if (buscarPersonagem((CadastroPersonagens *)cadastro, idPersonagem) == NULL) {
        return 0; /* Falha: personagem inexistente */
    }

    /* Adiciona na proxima posicao livre e incrementa a quantidade */
    grupo->idsPersonagens[grupo->quantidade] = idPersonagem;
    grupo->quantidade++;

    calculaPoderGrupo(grupo, cadastro);
    return 1; /* Sucesso */
}

int removePersonagemGrupo(GrupoAventura *grupo, int idPersonagem) {
    if (grupo == NULL || grupo->quantidade == 0) {
        return 0;
    }

    int indiceRemocao = -1;

    /* Localiza o indice do personagem a ser removido */
    for (int i = 0; i < grupo->quantidade; i++) {
        if (grupo->idsPersonagens[i] == idPersonagem) {
            indiceRemocao = i;
            break;
        }
    }

    /* Retorna falha se o personagem nao estiver no grupo */
    if (indiceRemocao == -1) {
        return 0;
    }

    /* Desloca os IDs posteriores para a esquerda, mantendo o vetor contiguo */
    for (int i = indiceRemocao; i < grupo->quantidade - 1; i++) {
        grupo->idsPersonagens[i] = grupo->idsPersonagens[i + 1];
    }

    /* Limpa ultima posicao e decrementa a quantidade */
    grupo->idsPersonagens[grupo->quantidade - 1] = 0;
    grupo->quantidade--;

    return 1; /* Sucesso */
}

int calculaPoderGrupo(GrupoAventura *grupo, const CadastroPersonagens *cadastro) {
    if (grupo == NULL) {
        return 0;
    }

    int somaPoder = 0;

    if (cadastro != NULL) {
        for (int i = 0; i < grupo->quantidade; i++) {
            const Personagem *p = buscarPersonagem((CadastroPersonagens *)cadastro, grupo->idsPersonagens[i]);
            if (p != NULL) {
                somaPoder += calcularPoderTotal(p);
            }
        }
    }

    grupo->poderTotal = somaPoder;
    return somaPoder;
}

void exibeGrupo(const GrupoAventura *grupo, const CadastroPersonagens *cadastro) {
    if (grupo == NULL) {
        printf("Grupo invalido.\n");
        return;
    }

    printf("\n=== GRUPO DE AVENTURA (%d/%d) ===\n", grupo->quantidade, MAX_PERSONAGENS_GRUPO);

    if (grupo->quantidade == 0) {
        printf("Nenhum personagem no grupo.\n");
        printf("Poder Total do Grupo: %d\n", grupo->poderTotal);
        return;
    }

    for (int i = 0; i < grupo->quantidade; i++) {
        int id = grupo->idsPersonagens[i];
        const Personagem *p = NULL;

        if (cadastro != NULL) {
            p = buscarPersonagem((CadastroPersonagens *)cadastro, id);
        }

        if (p != NULL) {
            printf("  [%d] %s | %s %s (Nv %d) | PV: %d/%d | Poder: %d\n",
                   p->id,
                   p->nome,
                   nomeRaca(p->raca),
                   nomeClasse(p->classe),
                   p->nivel,
                   p->hp,
                   p->vidaMaxima,
                   calcularPoderTotal(p));
        } else {
            printf("  [ID %d] (Personagem nao encontrado no cadastro)\n", id);
        }
    }

    printf("------------------------------------\n");
    printf("Poder Total do Grupo: %d\n", grupo->poderTotal);
}
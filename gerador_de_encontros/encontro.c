#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encontro.h"
#include "dados.h"

/* --- Auxiliares internos --- */

static const Monstro *encontraMonstroPorId(const Bestiario *b, int idMonstro) {
    if (b == NULL) {
        return NULL;
    }

    return consultaMonstroPorId(b, idMonstro);
}

static int verificaTerrenoCompativel(const Monstro *m, Terreno terreno) {
    if (m == NULL) {
        return 0;
    }

    return viveNoTerreno(m, terreno);
}

static int verificaPeriodoCompativel(const Monstro *m, Periodo periodo) {
    if (m == NULL) {
        return 0;
    }

    Atividade atividadeEncontro;
    if (periodo == PERIODO_DIA) {
        atividadeEncontro = DIURNO;
    } else if (periodo == PERIODO_NOITE) {
        atividadeEncontro = NOTURNO;
    } else {
        atividadeEncontro = QUALQUER_HORARIO;
    }

    return ativoNoPeriodo(m, atividadeEncontro);
}

static int pesoFrequencia(Frequencia freq) {
    switch (freq) {
        case MUITO_COMUM:
            return 50;
        case COMUM:
            return 35;
        case INCOMUM:
            return 20;
        case RARO:
            return 10;
        case MUITO_RARO:
            return 4;
        default:
            return 10;
    }
}

static int sorteiaQuantidadeMonstro(const Monstro *m, int vagasRestantes) {
    if (m == NULL || vagasRestantes <= 0) {
        return 0;
    }

    int qtd = 1;
    if (validaDado(m->numeroEncontrado)) {
        qtd = rolaDado(m->numeroEncontrado);
    }

    if (m->minimoCovil > 0 && qtd < m->minimoCovil) {
        qtd = m->minimoCovil;
    }

    if (m->maximoCovil > 0 && m->maximoCovil >= m->minimoCovil && qtd > m->maximoCovil) {
        qtd = m->maximoCovil;
    }

    if (qtd < 1) {
        qtd = 1;
    }
    if (qtd > vagasRestantes) {
        qtd = vagasRestantes;
    }

    return qtd;
}

/* --- Contexto, filtro e poder --- */

void calculaFaixaDificuldade(int poderGrupo, Dificuldade dificuldade, int *minimo, int *maximo) {
    if (minimo == NULL || maximo == NULL) {
        return;
    }

    if (poderGrupo < 0) {
        poderGrupo = 0;
    }

    switch (dificuldade) {
        case FACIL:
            *minimo = (poderGrupo * 40) / 100;
            *maximo = (poderGrupo * 75) / 100;
            break;
        case MEDIA:
            *minimo = (poderGrupo * 75) / 100;
            *maximo = (poderGrupo * 115) / 100;
            break;
        case DIFICIL:
            *minimo = (poderGrupo * 115) / 100;
            *maximo = (poderGrupo * 160) / 100;
            break;
        default:
            *minimo = (poderGrupo * 75) / 100;
            *maximo = (poderGrupo * 115) / 100;
            break;
    }

    if (*maximo < *minimo) {
        *maximo = *minimo;
    }
}

int monstroCompativel(const Monstro *m, const ContextoEncontro *contexto) {
    if (m == NULL || contexto == NULL) {
        return 0;
    }
    if (!validaMonstro(m)) {
        return 0;
    }
    if (!verificaTerrenoCompativel(m, contexto->terreno)) {
        return 0;
    }
    if (!verificaPeriodoCompativel(m, contexto->periodo)) {
        return 0;
    }

    return 1;
}

int filtraCandidatos(const Bestiario *b, const ContextoEncontro *contexto, ListaCandidatos *saida) {
    if (saida == NULL) {
        return 0;
    }

    saida->quantidade = 0;

    if (b == NULL || contexto == NULL) {
        return 0;
    }

    for (int i = 0; i < b->quantidade && saida->quantidade < MAX_CANDIDATOS; i++) {
        const Monstro *m = &b->monstros[i];
        if (monstroCompativel(m, contexto)) {
            saida->itens[saida->quantidade] = m;
            saida->quantidade++;
        }
    }

    return saida->quantidade;
}

int poderIndividualContextual(const Monstro *m, const ContextoEncontro *contexto) {
    if (m == NULL) {
        return 0;
    }

    int poder = m->poder.poderBase;
    if (poder <= 0) {
        if (m->valorXP > 0) {
            poder = m->valorXP;
        } else {
            poder = 1;
        }
    }

    if (contexto != NULL) {
        if (contexto->periodo == PERIODO_DIA) {
            poder += m->poder.modDia;
        } else if (contexto->periodo == PERIODO_NOITE) {
            poder += m->poder.modNoite;
        }

        for (int i = 0; i < m->poder.quantidadeModTerrenos && i < MAX_TERRENOS; i++) {
            if (m->poder.modTerreno[i].terreno == contexto->terreno) {
                poder += m->poder.modTerreno[i].modificador;
                break;
            }
        }
    }

    if (poder < 1) {
        return 1;
    } else {
        return poder;
    }
}

int poderComponente(const Monstro *m, int quantidade, const ContextoEncontro *contexto) {
    if (m == NULL || quantidade <= 0) {
        return 0;
    }

    int unitario = poderIndividualContextual(m, contexto);
    int total = unitario * quantidade;

    if (quantidade > 1 && m->poder.bonusPorAdicional != 0) {
        total += (quantidade - 1) * m->poder.bonusPorAdicional;
    }

    if (m->poder.limiteGrupoGrande > 0 &&
        quantidade >= m->poder.limiteGrupoGrande &&
        m->poder.bonusGrupoGrande != 0) {
        total += m->poder.bonusGrupoGrande;
    }

    if (total < 1) {
        return 1;
    } else {
        return total;
    }
}

int calculaPoderEncontro(Encontro *e, const Bestiario *b, const ContextoEncontro *contexto) {
    if (e == NULL) {
        return 0;
    }

    int somaPoder = 0;
    int somaMonstros = 0;

    for (int i = 0; i < e->quantidadeEspecies && i < MAX_ESPECIES_ENCONTRO; i++) {
        ComponenteEncontro *comp = &e->componentes[i];
        const Monstro *m = encontraMonstroPorId(b, comp->idMonstro);

        if (m != NULL && comp->quantidade > 0) {
            comp->poderCalculado = poderComponente(m, comp->quantidade, contexto);
            somaPoder += comp->poderCalculado;
            somaMonstros += comp->quantidade;
        } else {
            comp->poderCalculado = 0;
        }
    }

    e->quantidadeMonstros = somaMonstros;
    e->poderTotal = somaPoder;
    return somaPoder;
}

/* --- Sorteio e geracao --- */

const Monstro *sorteiaMonstroPonderado(const ListaCandidatos *candidatos) {
    if (candidatos == NULL || candidatos->quantidade <= 0) {
        return NULL;
    }

    int pesoTotal = 0;
    for (int i = 0; i < candidatos->quantidade; i++) {
        if (candidatos->itens[i] != NULL) {
            pesoTotal += pesoFrequencia(candidatos->itens[i]->frequencia);
        }
    }

    if (pesoTotal <= 0) {
        return candidatos->itens[rand() % candidatos->quantidade];
    }

    int sorteio = rand() % pesoTotal;
    int acumulado = 0;

    for (int i = 0; i < candidatos->quantidade; i++) {
        const Monstro *m = candidatos->itens[i];
        if (m != NULL) {
            acumulado += pesoFrequencia(m->frequencia);
            if (sorteio < acumulado) {
                return m;
            }
        }
    }

    return candidatos->itens[candidatos->quantidade - 1];
}

int tentaGerarEncontro(const Bestiario *b, const ListaCandidatos *candidatos,
                       const ContextoEncontro *contexto, int poderMinimo,
                       int poderMaximo, Encontro *saida) {
    if (saida == NULL) {
        return 0;
    }

    memset(saida, 0, sizeof(*saida));

    if (b == NULL || candidatos == NULL || candidatos->quantidade <= 0 || contexto == NULL) {
        return 0;
    }

    int maxEspeciesDesejadas = 1 + (rand() % MAX_ESPECIES_ENCONTRO);
    int totalMonstrosAtual = 0;

    for (int tentativa = 0; tentativa < 25; tentativa++) {
        if (saida->quantidadeEspecies >= maxEspeciesDesejadas) {
            break;
        }
        if (totalMonstrosAtual >= MAX_MONSTROS_ENCONTRO) {
            break;
        }

        const Monstro *escolhido = sorteiaMonstroPonderado(candidatos);
        if (escolhido == NULL) {
            continue;
        }

        /* Evita repetir a mesma especie no mesmo encontro */
        int jaExiste = 0;
        for (int j = 0; j < saida->quantidadeEspecies; j++) {
            if (saida->componentes[j].idMonstro == escolhido->id) {
                jaExiste = 1;
                break;
            }
        }
        if (jaExiste) {
            continue;
        }

        int vagas = MAX_MONSTROS_ENCONTRO - totalMonstrosAtual;
        int qtd = sorteiaQuantidadeMonstro(escolhido, vagas);
        if (qtd <= 0) {
            continue;
        }

        int poderComp = poderComponente(escolhido, qtd, contexto);
        if (saida->poderTotal + poderComp > poderMaximo) {
            poderComp = poderComponente(escolhido, 1, contexto);
            if (saida->poderTotal + poderComp <= poderMaximo) {
                qtd = 1;
            } else {
                continue;
            }
        }

        int idx = saida->quantidadeEspecies;
        saida->componentes[idx].idMonstro = escolhido->id;
        saida->componentes[idx].quantidade = qtd;
        saida->componentes[idx].poderCalculado = poderComp;
        saida->quantidadeEspecies++;
        totalMonstrosAtual += qtd;
        saida->poderTotal += poderComp;

        if (saida->poderTotal >= poderMinimo) {
            if (saida->quantidadeEspecies >= 1 && (rand() % 100) < 60) {
                break;
            }
        }
    }

    calculaPoderEncontro(saida, b, contexto);

    if (saida->quantidadeEspecies > 0 &&
        saida->poderTotal >= poderMinimo &&
        saida->poderTotal <= poderMaximo) {
        return 1;
    }

    return 0;
}

int encontrosIguais(const Encontro *a, const Encontro *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    if (a->quantidadeEspecies != b->quantidadeEspecies) {
        return 0;
    }
    if (a->quantidadeMonstros != b->quantidadeMonstros) {
        return 0;
    }
    if (a->poderTotal != b->poderTotal) {
        return 0;
    }

    for (int i = 0; i < a->quantidadeEspecies; i++) {
        int achou = 0;
        for (int j = 0; j < b->quantidadeEspecies; j++) {
            if (a->componentes[i].idMonstro == b->componentes[j].idMonstro &&
                a->componentes[i].quantidade == b->componentes[j].quantidade) {
                achou = 1;
                break;
            }
        }
        if (!achou) {
            return 0;
        }
    }

    return 1;
}

int geraPropostas(const Bestiario *b, const GrupoAventura *grupo,
                  const ContextoEncontro *contexto, ConjuntoPropostas *saida) {
    if (saida == NULL) {
        return 0;
    }

    saida->quantidade = 0;
    saida->indiceEscolhido = -1;

    if (b == NULL || grupo == NULL || contexto == NULL) {
        return 0;
    }
    if (grupo->quantidade <= 0 || grupo->poderTotal <= 0) {
        return 0;
    }

    ListaCandidatos candidatos;
    if (filtraCandidatos(b, contexto, &candidatos) <= 0) {
        return 0;
    }

    int poderMin = 0;
    int poderMax = 0;
    calculaFaixaDificuldade(grupo->poderTotal, contexto->dificuldade, &poderMin, &poderMax);

    int maxTentativasGlobais = 300;
    for (int tent = 0; tent < maxTentativasGlobais && saida->quantidade < NUM_PROPOSTAS; tent++) {
        Encontro candidata;
        if (tentaGerarEncontro(b, &candidatos, contexto, poderMin, poderMax, &candidata)) {
            int duplicada = 0;
            for (int k = 0; k < saida->quantidade; k++) {
                if (encontrosIguais(&saida->propostas[k], &candidata)) {
                    duplicada = 1;
                    break;
                }
            }

            if (!duplicada) {
                saida->propostas[saida->quantidade] = candidata;
                saida->quantidade++;
            }
        }
    }

    return saida->quantidade;
}

int escolheProposta(ConjuntoPropostas *conjunto, int opcao) {
    if (conjunto == NULL || opcao < 0 || opcao >= conjunto->quantidade) {
        return 0;
    }
    conjunto->indiceEscolhido = opcao;
    return 1;
}

void exibeEncontro(const Encontro *e, const Bestiario *b) {
    if (e == NULL) {
        printf("Encontro invalido.\n");
        return;
    }

    printf("\n=== DETALHES DO ENCONTRO ===\n");
    printf("Especies distintas : %d\n", e->quantidadeEspecies);
    printf("Total de monstros  : %d\n", e->quantidadeMonstros);
    printf("Poder total        : %d\n", e->poderTotal);
    printf("----------------------------\n");

    if (e->quantidadeEspecies == 0) {
        printf("Nenhum componente no encontro.\n");
        return;
    }

    for (int i = 0; i < e->quantidadeEspecies; i++) {
        const ComponenteEncontro *comp = &e->componentes[i];
        const Monstro *m = encontraMonstroPorId(b, comp->idMonstro);

        if (m != NULL) {
            printf("  %d. %dx %s [ID %d] - Poder do componente: %d\n",
                   i + 1, comp->quantidade, m->nome, m->id, comp->poderCalculado);
        } else {
            printf("  %d. %dx Monstro [ID %d] - Poder do componente: %d\n",
                   i + 1, comp->quantidade, comp->idMonstro, comp->poderCalculado);
        }
    }
}
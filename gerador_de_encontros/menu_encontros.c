#include <stdio.h>
#include "encontro.h"

/* Le um inteiro validado e limpa o restante da linha */
static int leInteiroEncontros(int *valor)
{
    while (scanf("%d", valor) != 1) {
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
        }
        if (caractere == EOF) {
            return 0;
        }
        printf("Entrada invalida. Digite um numero: ");
    }

    int caractere;
    while ((caractere = getchar()) != '\n' && caractere != EOF) {
    }
    return 1;
}

/* Menu interativo do modulo gerador_de_encontros */
void menuEncontros(const Bestiario *bestiario,
                   const CadastroPersonagens *cadastro,
                   GrupoAventura *grupoSelecionado,
                   Encontro *encontroSelecionado,
                   int *temEncontroSelecionado)
{
    if (bestiario == NULL || cadastro == NULL || grupoSelecionado == NULL ||
        encontroSelecionado == NULL || temEncontroSelecionado == NULL) {
        return;
    }

    if (cadastro->quantidade == 0) {
        printf("Cadastre personagens antes de gerar encontros.\n");
        return;
    }

    GrupoAventura grupo;
    inicializaGrupo(&grupo);

    int quantidade;
    printf("\n=== GERADOR DE ENCONTROS ===\n");
    printf("Quantos personagens participarao (1 a %d)? ", MAX_PERSONAGENS_GRUPO);
    if (!leInteiroEncontros(&quantidade) || quantidade < 1 ||
        quantidade > MAX_PERSONAGENS_GRUPO) {
        printf("Quantidade invalida.\n");
        return;
    }

    for (int i = 0; i < quantidade; i++) {
        int id;
        printf("ID do personagem %d: ", i + 1);
        if (!leInteiroEncontros(&id) || !adicionaPersonagemGrupo(&grupo, cadastro, id)) {
            printf("ID invalido, repetido ou personagem nao encontrado.\n");
            return;
        }
    }

    int terreno;
    int periodo;
    int dificuldade;

    printf("Terreno (0 caverna, 1 ruinas, 2 floresta, 3 montanha, 4 planicie, "
           "5 deserto, 6 urbano, 7 pantano): ");
    if (!leInteiroEncontros(&terreno) || terreno < CAVERNA || terreno > PANTANO) {
        printf("Terreno invalido.\n");
        return;
    }

    printf("Periodo (0 dia, 1 noite): ");
    if (!leInteiroEncontros(&periodo) || periodo < PERIODO_DIA || periodo > PERIODO_NOITE) {
        printf("Periodo invalido.\n");
        return;
    }

    printf("Dificuldade (0 facil, 1 media, 2 dificil): ");
    if (!leInteiroEncontros(&dificuldade) || dificuldade < FACIL || dificuldade > DIFICIL) {
        printf("Dificuldade invalida.\n");
        return;
    }

    ContextoEncontro contexto = {
        .terreno = (Terreno)terreno,
        .periodo = (Periodo)periodo,
        .dificuldade = (Dificuldade)dificuldade
    };
    ConjuntoPropostas propostas;

    if (geraPropostas(bestiario, &grupo, &contexto, &propostas) == 0) {
        printf("Nao foi possivel gerar propostas para esse grupo e contexto.\n");
        return;
    }

    for (int i = 0; i < propostas.quantidade; i++) {
        printf("\nProposta %d:\n", i + 1);
        exibeEncontro(&propostas.propostas[i], bestiario);
    }

    int escolha;
    printf("Escolha uma proposta (1 a %d, ou 0 para voltar): ", propostas.quantidade);
    if (!leInteiroEncontros(&escolha) || escolha == 0) {
        return;
    }

    if (escolheProposta(&propostas, escolha - 1)) {
        *grupoSelecionado = grupo;
        *encontroSelecionado = propostas.propostas[escolha - 1];
        *temEncontroSelecionado = 1;
        printf("Proposta %d escolhida e pronta para o combate.\n", escolha);
    } else {
        printf("Opcao invalida.\n");
    }
}
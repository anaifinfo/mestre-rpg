#include <stdio.h>

#include "bestiario/bestiario.h"
#include "gerador_de_encontros/encontro.h"
#include "gerenciamento_de_combate/combate.h"
#include "personagens_e_equipamentos/personagem.h"

static int leInteiro(int *valor)
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

static void exibeMenuPrincipal(void)
{
    printf("\n=== Mestre de RPG ===\n");
    printf("1 - Personagens e equipamentos\n");
    printf("2 - Bestiario\n");
    printf("3 - Gerar encontros\n");
    printf("4 - Gerenciamento de combate\n");
    printf("0 - Sair\n");
    printf("Opcao: ");
}

static void menuGeracaoEncontros(const Bestiario *bestiario,
                                 const CadastroPersonagens *cadastro)
{
    if (cadastro->quantidade == 0) {
        printf("Cadastre personagens antes de gerar encontros.\n");
        return;
    }

    GrupoAventura grupo;
    inicializaGrupo(&grupo);

    int quantidade;
    printf("Quantos personagens participarao (1 a %d)? ", MAX_PERSONAGENS_GRUPO);
    if (!leInteiro(&quantidade) || quantidade < 1 ||
        quantidade > MAX_PERSONAGENS_GRUPO) {
        printf("Quantidade invalida.\n");
        return;
    }

    for (int i = 0; i < quantidade; i++) {
        int id;
        printf("ID do personagem %d: ", i + 1);
        if (!leInteiro(&id) || !adicionaPersonagemGrupo(&grupo, cadastro, id)) {
            printf("ID invalido, repetido ou personagem nao encontrado.\n");
            return;
        }
    }

    int terreno;
    int periodo;
    int dificuldade;
    printf("Terreno (0 caverna, 1 ruinas, 2 floresta, 3 montanha, 4 planicie, "
           "5 deserto, 6 urbano, 7 pantano): ");
    if (!leInteiro(&terreno) || terreno < CAVERNA || terreno > PANTANO) {
        printf("Terreno invalido.\n");
        return;
    }

    printf("Periodo (0 dia, 1 noite): ");
    if (!leInteiro(&periodo) || periodo < PERIODO_DIA || periodo > PERIODO_NOITE) {
        printf("Periodo invalido.\n");
        return;
    }

    printf("Dificuldade (0 facil, 1 media, 2 dificil): ");
    if (!leInteiro(&dificuldade) || dificuldade < FACIL || dificuldade > DIFICIL) {
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
    printf("Escolha uma proposta (1 a %d, ou 0 para voltar): ",
           propostas.quantidade);
    if (!leInteiro(&escolha)) {
        return;
    }
    if (escolha == 0) {
        return;
    }
    if (escolheProposta(&propostas, escolha - 1)) {
        printf("Proposta %d escolhida.\n", escolha);
    } else {
        printf("Opcao invalida.\n");
    }
}

int main(void)
{
    CadastroPersonagens cadastro;
    Bestiario bestiario;
    int opcao;

    inicializarCadastro(&cadastro);
    inicializaBestiario(&bestiario);

    if (carregaBestiario(&bestiario, "bestiario/bestiario.txt")) {
        printf("Bestiario carregado: %d monstros.\n", bestiario.quantidade);
    } else {
        printf("Aviso: nao foi possivel carregar bestiario/bestiario.txt.\n");
    }

    do {
        exibeMenuPrincipal();
        if (!leInteiro(&opcao)) {
            break;
        }

        switch (opcao) {
        case 1:
            menuPersonagens(&cadastro);
            break;
        case 2:
            menuBestiario(&bestiario);
            break;
        case 3:
            menuGeracaoEncontros(&bestiario, &cadastro);
            break;
        case 4:
            menuCombate();
            break;
        case 0:
            printf("Encerrando.\n");
            break;
        default:
            printf("Opcao invalida.\n");
            break;
        }
    } while (opcao != 0);

    return 0;
}

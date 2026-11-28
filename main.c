#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bestiario.h"
#include "encontro.h"
#include "combate.h"
#include "personagem.h"

/* Prototipos dos menus localizados em cada subpasta */
void menuBestiario(Bestiario *b);
void menuEncontros(const Bestiario *bestiario,
                   const CadastroPersonagens *cadastro,
                   GrupoAventura *grupoSelecionado,
                   Encontro *encontroSelecionado,
                   int *temEncontroSelecionado);
void menuCombateFluxo(Combate *combate,
                      CadastroPersonagens *cadastro,
                      const GrupoAventura *grupo,
                      const Encontro *encontro,
                      const Bestiario *bestiario);

/* Le um inteiro com seguranca e limpa o restante da linha */
static int leInteiroPrincipal(int *valor)
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

/* Exibe o menu principal do sistema */
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

int main(void)
{
    CadastroPersonagens cadastro;
    Bestiario bestiario;
    GrupoAventura grupoAtual;
    Encontro encontroAtual;
    Combate combateAtual;
    int temEncontro = 0;
    int opcao;

    srand((unsigned int)time(NULL));
    inicializarCadastro(&cadastro);
    inicializaBestiario(&bestiario);
    inicializaGrupo(&grupoAtual);
    inicializaCombate(&combateAtual);

    if (carregaBestiario(&bestiario, "bestiario/bestiario.txt")) {
        printf("Bestiario carregado: %d monstros.\n", bestiario.quantidade);
    } else {
        printf("Aviso: nao foi possivel carregar bestiario/bestiario.txt.\n");
    }

    do {
        exibeMenuPrincipal();
        if (!leInteiroPrincipal(&opcao)) {
            break;
        }

        switch (opcao) {
        case 1:
            /* Menu em personagens_e_equipamentos/menu_personagens.c */
            menuPersonagens(&cadastro);
            break;
        case 2:
            /* Menu em bestiario/menu_bestiario.c */
            menuBestiario(&bestiario);
            break;
        case 3:
            /* Menu em gerador_de_encontros/menu_encontros.c */
            menuEncontros(&bestiario, &cadastro, &grupoAtual,
                          &encontroAtual, &temEncontro);
            break;
        case 4:
            /* Menu em gerenciamento_de_combate/menu_combate.c */
            if (!temEncontro) {
                printf("Gere e escolha um encontro na opcao 3 antes de iniciar o combate.\n");
            } else {
                menuCombateFluxo(&combateAtual, &cadastro, &grupoAtual,
                                 &encontroAtual, &bestiario);
            }
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
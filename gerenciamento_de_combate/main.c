#include <stdio.h>

void menuCombate(void) {
    int opcao;

    do {
        printf("\n1 - Preparar combate com o encontro escolhido\n");
        printf("2 - Exibir participantes e ordem\n");
        printf("3 - Executar uma rodada\n");
        printf("4 - Executar até o encerramento\n");
        printf("5 - Exibir estado atual\n");
        printf("6 - Exibir resumo do último combate\n");
        printf("0 - Voltar ao menu principal\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            return;
        }

        switch (opcao) {
            case 1:
                printf("Preparar combate com o encontro escolhido.\n");
                break;
            case 2:
                printf("Exibir participantes e ordem.\n");
                break;
            case 3:
                printf("Executar uma rodada.\n");
                break;
            case 4:
                printf("Executar até o encerramento.\n");
                break;
            case 5:
                printf("Exibir estado atual.\n");
                break;
            case 6:
                printf("Exibir resumo do último combate.\n");
                break;
            case 0:
                printf("Voltando ao menu principal.\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (opcao != 0);

}
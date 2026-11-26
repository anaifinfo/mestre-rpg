#include <stdlib.h>
#include "dados.h"

int rolaDado(Dado dado) {
    if (!validaDado(dado)) {
        return 0;
    }

    int soma = 0;
    for (int i = 0; i < dado.quantidade; i++) {
        /* rand() % dado.faces gera [0, faces - 1], somamos 1 para [1, faces] */
        int rolagem = (rand() % dado.faces) + 1;
        soma += rolagem;
    }

    int total = soma + dado.modificador;
    if (total < 0) {
        return 0;
    } else {
        return total;
    }
}
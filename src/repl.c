#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../include/parser.h"
#include "../include/ari.h"
#include "../include/eval.h"
#include "../include/io.h"
#include "../lib/mpc.h"

int numNodes(mpc_ast_t *t) {
    if (t->children_num == 0 && strlen(t->contents) > 0)
        return 1;

    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
        total += numNodes(t->children[i]);
    }
    return total;
}

int main(void) {
    parser_init();

    puts("Welcome! You are using Crox.");
    puts("A terribly useless language to solve none of your problems.");
    puts("You are using version: 0.0.0.4");

    while (1) {
        char *input = readline("\033[m\033[31mcrox -> \033[0m");
        if (!input) break;
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, CroxParser, &r)) {
            ari *result = ari_eval(ari_read(r.output));
            ari_println(result);
            ari_del(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    parser_cleanup();
    return 0;
}

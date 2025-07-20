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

int main(int argc, char** argv) {
    parser_init();

    puts("Welcome! You are using Crox.");
    puts("A terribly useless language to solve none of your problems.");
    puts("You are using version: 0.0.0.5");

    anv* e = anv_new();

    if (argc >= 2) {
        FILE* f = fopen(argv[1], "r"); 
        if (f == NULL) {
            perror("Crox"); 
            return 1;
        }
        
        char buffer[2048];
        while (fgets(buffer, sizeof(buffer), f) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;

            if (strlen(buffer) == 0 || strspn(buffer, " \t\r") == strlen(buffer)) {
                continue;
            }

            printf("\033[m\033[31mCrox -> %s\033[0m\n", buffer); 

            mpc_result_t r;
            size_t len = strlen(buffer);
            char* line_copy = (char*) malloc(len + 1); 
            if (line_copy == NULL) {
                fprintf(stderr, "Error: Memory allocation failed for line_copy.\n");
                fclose(f);
                anv_del(e);
                parser_cleanup();
                return 1;
            }
            strcpy(line_copy, buffer); // Copy the content

            if (mpc_parse("<stdin>", line_copy, CroxParser, &r)) {
                ari *result = ari_eval(e, ari_read(r.output));
                ari_println(result);
                ari_del(result);
                mpc_ast_delete(r.output);
            } else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }
            free(line_copy); // Free the duplicated line
        }
        fclose(f); // Close the file
    } else {
        while (1) {
            char *input = readline("\033[m\033[31mCrox -> \033[0m");
            if (!input) break;
            add_history(input);

            mpc_result_t r;
            if (mpc_parse("<stdin>", input, CroxParser, &r)) {
                ari *result = ari_eval(e, ari_read(r.output));
                ari_println(result);
                ari_del(result);
                mpc_ast_delete(r.output);
            } else {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }

            free(input);
        }
    }

    anv_del(e); 
    parser_cleanup();
    return 0;
}
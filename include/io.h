#ifndef IO_H
#define IO_H

#include "ari.h"
#include"../lib/mpc.h"

// To read numbers
ari* ari_read_num(mpc_ast_t *t);

// To read the AST TREE
ari* ari_read(mpc_ast_t * t);

// print functions
void ari_print(ari* v);
void ari_exp_print(ari*v, char open, char close);
void ari_print(ari* n);
void ari_println(ari* n);

#endif
#ifndef PARSER_H
#define PARSER_H

#include "../lib/mpc.h"

// Declare parser objects
extern mpc_parser_t *Number;
extern mpc_parser_t *Symbol;
extern mpc_parser_t *Sexpr;
extern mpc_parser_t *Qexpr;
extern mpc_parser_t *Sfactor;
extern mpc_parser_t *Expression;
extern mpc_parser_t *Term;
extern mpc_parser_t *Power;
extern mpc_parser_t *Factor;
extern mpc_parser_t *Statement;
extern mpc_parser_t *CroxParser;

// Setup and teardown
void parser_init(void);
void parser_cleanup(void);

#endif

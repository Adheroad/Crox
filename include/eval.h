#ifndef EVAL_H
#define EVAL_H

#include "ari.h"

// core eval
ari *ari_eval(ari *n);
ari* ari_eval_sexpr(ari* n);

// builtin helpers
ari *ari_join(ari *x, ari *y);
ari *ari_cons(ari *val, ari *qexpr);

// builtins
ari *builtin(ari *a, char *func);
ari *builtin_op(ari *n, char *op);

ari *builtin_list(ari *a);
ari *builtin_eval(ari *a);
ari *builtin_head(ari *a);
ari *builtin_tail(ari *a);
ari *builtin_unhead(ari *a);
ari *builtin_untail(ari *a);
ari *builtin_len(ari *a);
ari *builtin_cons(ari *a);
ari *builtin_conlast(ari *a);
ari *builtin_init(ari *a);
ari *builtin_uninit(ari *a);
ari* builtin_join(ari* a);

#endif

#ifndef EVAL_H
#define EVAL_H

#include "ari.h"

// core eval
ari *ari_eval(anv *e, ari *n);
ari *ari_eval_sexpr(anv *e, ari *n);

// builtin helpers
ari *ari_join(ari *x, ari *y);
ari *ari_cons(ari *val, ari *qexpr);

// builtins
void builtins(anv *e);
ari *builtin_op(anv *e, ari *n, char *op);
void anv_add_builtin(anv* e, char* name, abuiltin fun);

// builrin for ops
ari *builtin_add(anv *e, ari *n);
ari *builtin_del(anv *e, ari *n);
ari *builtin_mul(anv *e, ari *n);
ari *builtin_div(anv *e, ari *n);
ari *builtin_mod(anv *e, ari *n);
ari *builtin_pow(anv *e, ari *n);
ari *builtin_log(anv *e, ari *n);

// builtin for funcs
ari *builtin_list(anv *e, ari *a);
ari *builtin_eval(anv *e, ari *a);
ari *builtin_head(anv *e, ari *a);
ari *builtin_tail(anv *e, ari *a);
ari *builtin_unhead(anv *e, ari *a);
ari *builtin_untail(anv *e, ari *a);
ari *builtin_len(anv *e, ari *a);
ari *builtin_cons(anv *e, ari *a);
ari *builtin_conlast(anv *e, ari *a);
ari *builtin_init(anv *e, ari *a);
ari *builtin_uninit(anv *e, ari *a);
ari *builtin_join(anv *e, ari *a);

#endif

#include "../include/eval.h"
#include "../include/ari.h"
#include "../include/macros.h"
#include <math.h>
#include <string.h>

ari *ari_eval(anv *e, ari *n) {
  if (!e || !n)
    return NULL;
  if (n->type == ARI_SYM) {
    ari *x = anv_get(e, n);
    ari_del(n);
    return x;
  }
  if (n->type == ARI_SEXPR)
    return ari_eval_sexpr(e, n);
  return n;
}

ari *ari_eval_sexpr(anv *e, ari *n) {
  for (int i = 0; i < n->count; i++) {
    n->cell[i] = ari_eval(e, n->cell[i]);
    if (n->cell[i]->type == ARI_ERR) {
      return ari_take(n, i);
    }
  }

  if (n->count == 0) {
    return n;
  }

  if (n->count == 1) {
    return ari_take(n, 0);
  }

  ari *fun = ari_pop(n, 0);
  if (fun->type != ARI_FUN) {
    ari_del(fun);
    ari_del(n);
    return ari_err("first no is not a function");
  }

  ari *result = fun->func(e, n);
  ari_del(fun);
  return result;
}

char *atype_name(int t) {
  switch (t) {
  case ARI_FUN:
    return "Function";
  case ARI_VAL:
    return "Number";
  case ARI_ERR:
    return "Error";
  case ARI_SYM:
    return "Symbol";
  case ARI_SEXPR:
    return "S-Expression";
  case ARI_QEXPR:
    return "Q-Expression";
  default:
    return "Unknown";
  }
}

void anv_add_builtin(anv *e, char *name, abuiltin func) {
  if (!e || !name || !func)
    return;
  ari *n = ari_sym(name);
  ari *fun = ari_fun(func);
  if (!n || !fun) {
    if (n)
      ari_del(n);
    if (fun)
      ari_del(fun);
    return;
  }
  anv_put(e, n, fun);
  ari_del(n);
  ari_del(fun);
}

void builtins(anv *e) {
  // mathematical builtins
  anv_add_builtin(e, "+", builtin_add);
  anv_add_builtin(e, "-", builtin_del);
  anv_add_builtin(e, "*", builtin_mul);
  anv_add_builtin(e, "/", builtin_div);
  anv_add_builtin(e, "%", builtin_mod);
  anv_add_builtin(e, "^", builtin_pow);
  anv_add_builtin(e, "log", builtin_log);

  anv_add_builtin(e, "list", builtin_list);
  anv_add_builtin(e, "eval", builtin_eval);
  anv_add_builtin(e, "head", builtin_head);
  anv_add_builtin(e, "tail", builtin_tail);
  anv_add_builtin(e, "unhead", builtin_unhead);
  anv_add_builtin(e, "untail", builtin_untail);
  anv_add_builtin(e, "len", builtin_len);
  anv_add_builtin(e, "cons", builtin_cons);
  anv_add_builtin(e, "conlast", builtin_conlast);
  anv_add_builtin(e, "init", builtin_init);
  anv_add_builtin(e, "uninit", builtin_uninit);
  anv_add_builtin(e, "join", builtin_join);
  anv_add_builtin(e, "def", builtin_def);
}

ari *builtin_op(anv *e, ari *n, char *op) {
  for (int i = 0; i < n->count; i++) {
    if (n->cell[i]->type != ARI_VAL) {
      ari_del(n);
      return ari_err("Cannot operate on a non-number");
    }
  }

  ari *x = ari_pop(n, 0);

  if (n->count == 0 && strcmp(op, "-") == 0) {
    x->val = -x->val;
  }

  while (n->count > 0) {
    ari *y = ari_pop(n, 0);

    if (strcmp(op, "+") == 0)
      x->val += y->val;
    else if (strcmp(op, "*") == 0)
      x->val *= y->val;
    else if (strcmp(op, "-") == 0)
      x->val -= y->val;
    else if (strcmp(op, "/") == 0) {
      if (y->val == 0) {
        ari_del(y);
        ari_del(x);
        ari_del(n);
        return ari_err("Division by zero not allowed");
      }
      x->val /= y->val;
    } else if (strcmp(op, "%") == 0) {
      if (y->val == 0) {
        ari_del(y);
        ari_del(x);
        ari_del(n);
        return ari_err("Modulo by zero not allowed");
      }
      x->val = fmodl(x->val, y->val);
    } else if (strcmp(op, "log") == 0) {
      x->val = log(y->val) / log(x->val);
    } else if (strcmp(op, "^") == 0) {
      x->val = powl(x->val, y->val);
    } else {
      ari_del(y);
      ari_del(x);
      ari_del(n);
      return ari_err("Unknown operator");
    }

    ari_del(y);
  }

  ari_del(n);
  return x;
}

ari *builtin_log(anv *e, ari *n) { return builtin_op(e, n, "log"); }
ari *builtin_add(anv *e, ari *n) { return builtin_op(e, n, "+"); }
ari *builtin_del(anv *e, ari *n) { return builtin_op(e, n, "-"); }
ari *builtin_mul(anv *e, ari *n) { return builtin_op(e, n, "*"); }
ari *builtin_div(anv *e, ari *n) { return builtin_op(e, n, "/"); }
ari *builtin_mod(anv *e, ari *n) { return builtin_op(e, n, "%"); }
ari *builtin_pow(anv *e, ari *n) { return builtin_op(e, n, "^"); }

ari *ari_cons(ari *val, ari *qexpr) {
  ari *result = ari_qexpr();
  ari_add(result, val);

  while (qexpr->count) {
    ari_add(result, ari_pop(qexpr, 0));
  }

  ari_del(qexpr); // cleanup original qexpr
  return result;
}

ari *ari_join(ari *x, ari *y) {

  /* For each cell in 'y' add it to 'x' */
  while (y->count) {
    x = ari_add(x, ari_pop(y, 0));
  }

  /* Delete the empty 'y' and return 'x' */
  ari_del(y);
  return x;
}

ari *builtin_head(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'head' passed too many arguments. "
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'head' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

  ari *v = ari_take(a, 0);
  while (v->count > 1) {
    ari_del(ari_pop(v, 1));
  }
  return v;
}

ari *builtin_unhead(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'unhead' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'unhead' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count != 0, "Function 'unhead' passed {}!");

  ari *v = ari_take(a, 0);
  ari_del(ari_pop(v, 0));
  return v;
}

ari *builtin_tail(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'tail' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'tail' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}!");

  ari *v = ari_take(a, 0);

  while (v->count > 1) {
    ari_del(ari_pop(v, 0)); // delete from front
  }

  return v;
}

ari *builtin_untail(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'untail' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'untail' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count != 0, "Function 'untail' passed {}!");

  // Take the Q-expression from the argument
  ari *v = ari_take(a, 0);

  // Delete the last element
  ari_del(ari_pop(v, v->count - 1));

  return v;
}

ari *builtin_list(anv *e, ari *a) {
  a->type = ARI_QEXPR;
  return a;
}

ari *builtin_eval(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'eval' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'eval' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));

  ari *x = ari_take(a, 0);
  x->type = ARI_SEXPR;
  return ari_eval(e, x);
}

ari *builtin_join(anv *e, ari *a) {

  for (int i = 0; i < a->count; i++) {
    ARISSERT(a, a->cell[i]->type == ARI_QEXPR,
             "Function 'join' passed incorrect type."
             "\n\033[0;34mGot %s, Expected %s.",
             atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  }

  ari *x = ari_pop(a, 0);

  while (a->count) {
    x = ari_join(x, ari_pop(a, 0));
  }

  ari_del(a);
  return x;
}

ari *builtin_cons(anv *e, ari *a) {
  ARISSERT(a, a->count >= 2, "Function 'cons' needs at least two arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_VAL,
           "Function 'cons' must start with a value."
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));

  for (int i = 1; i < a->count; i++) {
    ARISSERT(a, a->cell[i]->type == ARI_QEXPR,
             "Function 'cons' expects Q-expressions after the first value.");
  }

  ari *x = ari_pop(a, 0); // first value

  while (a->count) {
    x = ari_cons(x, ari_pop(a, 0));
  }

  ari_del(a);
  return x;
}

ari *builtin_conlast(anv *e, ari *a) {
  ARISSERT(a, a->count >= 2,
           "Function 'conlast' needs at least two arguments!");
  ARISSERT(a, a->cell[a->count - 1]->type == ARI_VAL,
           "Function 'conlast' must end with a value."
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));

  for (int i = 0; i < a->count - 1; i++) {
    ARISSERT(a, a->cell[i]->type == ARI_QEXPR,
             "Function 'conlast' expects Q-expressions before the value.");
  }

  ari *val = ari_pop(a, a->count - 1); // final value
  ari *q = ari_pop(a, 0);

  while (a->count) {
    q = ari_join(q, ari_pop(a, 0));
  }

  ari_add(q, val); // append the value
  ari_del(a);
  return q;
}

ari *builtin_len(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'len' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'len' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));

  int count = a->cell[0]->count;
  ari_del(a);
  return ari_val(count);
}

ari *builtin_init(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'init' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'init' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count >= 2,
           "Function 'init' needs at least two elements!");

  if (a->cell[0]->count == 2) {
    ari_del(a);
    return ari_qexpr();
  }

  // Take ownership of the Q-expression
  ari *q = ari_take(a, 0);

  // Delete first element (head)
  ari_del(ari_pop(q, 0));

  // Delete last element (tail)
  ari_del(ari_pop(q, q->count - 1));

  // Return what’s left (the middle)
  return q;
}

ari *builtin_uninit(anv *e, ari *a) {
  ARISSERT(a, a->count == 1,
           "Function 'uninit' passed too many arguments!"
           "\n\033[0;34mGot %i, Expected %i.",
           a->count, 1);
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'uninit' passed incorrect type!"
           "\n\033[0;34mGot %s, Expected %s.",
           atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));
  ARISSERT(a, a->cell[0]->count >= 2,
           "Function 'uninit' needs at least two elements!");

  ari *q = ari_take(a, 0);

  ari *head = ari_pop(q, 0);            // get first
  ari *last = ari_pop(q, q->count - 1); // get last

  ari *result = ari_qexpr();
  ari_add(result, head);
  ari_add(result, last);

  ari_del(q); // cleanup the rest
  return result;
}

ari *builtin_def(anv *e, ari *a) {
    ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
             "Function 'def' passed incorrect type!"
             "\n\033[0;34mGot %s, Expected %s.",
             atype_name(a->cell[0]->type), atype_name(ARI_QEXPR));

    ari *syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        ARISSERT(a, syms->cell[i]->type == ARI_SYM, "Cannot define without symbol");
    }

    ARISSERT(a, syms->count == a->count - 1,
             "Function 'def' cannot define incorrect "
             "number of values to symbols");

    for (int i = 0; i < syms->count; i++) {
        anv_put(e, syms->cell[i], a->cell[i + 1]);
    }
    
    ari_del(a);
    return ari_sexpr();
}
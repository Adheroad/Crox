#include <string.h>
#include <math.h>
#include "../include/macros.h"
#include "../include/eval.h"
#include "../include/ari.h"


ari *ari_eval(ari *n) {
  if (n->type == ARI_SEXPR)
    return ari_eval_sexpr(n);
  return n;
}


ari *ari_eval_sexpr(ari *n) {
  for (int i = 0; i < n->count; i++) {
    n->cell[i] = ari_eval(n->cell[i]);

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

  ari *sym = ari_pop(n, 0);
  if (sym->type != ARI_SYM) {
    ari_del(sym);
    ari_del(n);
    return ari_err("S-Expression must start with a symbol");
  }

  ari *result = builtin(n, sym->sym);
  ari_del(sym);
  return result;
}


ari *builtin(ari *a, char *func) {
  if (strcmp("list", func) == 0) {
    return builtin_list(a);
  } else if (strcmp("head", func) == 0) {
    return builtin_head(a);
  } else if (strcmp("unhead", func) == 0) {
    return builtin_unhead(a);
  } else if (strcmp("tail", func) == 0) {
    return builtin_tail(a);
  } else if (strcmp("untail", func) == 0) {
    return builtin_untail(a);
  } else if (strcmp("join", func) == 0) {
    return builtin_join(a);
  } else if (strcmp("eval", func) == 0) {
    return builtin_eval(a);
  } else if (strcmp("len", func) == 0) {
    return builtin_len(a);
  } else if (strcmp("cons", func) == 0) {
    return builtin_cons(a);
  } else if (strcmp("conlast", func) == 0) {
    return builtin_conlast(a);
  } else if (strcmp("init", func) == 0) {
    return builtin_init(a);
  } else if (strcmp("uninit", func) == 0) {
    return builtin_uninit(a);
  } else {
    return builtin_op(a, func);
  }
  ari_del(a);
  return ari_err("Unknown Function!");
}


ari *builtin_op(ari *n, char *op) {
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


ari* builtin_head(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'head' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'head' passed incorrect type!");
  ARISSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

  ari *v = ari_take(a, 0);
  while (v->count > 1) {
    ari_del(ari_pop(v, 1));
  }
  return v;
}

ari *builtin_unhead(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'unhead' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'unhead' passed incorrect type!");
  ARISSERT(a, a->cell[0]->count != 0, "Function 'unhead' passed {}!");

  ari *v = ari_take(a, 0);
  ari_del(ari_pop(v, 0));
  return v;
}

ari *builtin_tail(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'tail' passed incorrect type!");
  ARISSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}!");

  ari *v = ari_take(a, 0);

  while (v->count > 1) {
    ari_del(ari_pop(v, 0)); // delete from front
  }

  return v;
}

ari *builtin_untail(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'untail' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'untail' passed incorrect type!");
  ARISSERT(a, a->cell[0]->count != 0, "Function 'untail' passed {}!");

  // Take the Q-expression from the argument
  ari *v = ari_take(a, 0);

  // Delete the last element
  ari_del(ari_pop(v, v->count - 1));

  return v;
}

ari *builtin_list(ari *a) {
  a->type = ARI_QEXPR;
  return a;
}

ari *builtin_eval(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'eval' passed incorrect type!");

  ari *x = ari_take(a, 0);
  x->type = ARI_SEXPR;
  return ari_eval(x);
}



ari *builtin_join(ari *a) {

  for (int i = 0; i < a->count; i++) {
    ARISSERT(a, a->cell[i]->type == ARI_QEXPR,
             "Function 'join' passed incorrect type.");
  }

  ari *x = ari_pop(a, 0);

  while (a->count) {
    x = ari_join(x, ari_pop(a, 0));
  }

  ari_del(a);
  return x;
}



ari *builtin_cons(ari *a) {
  ARISSERT(a, a->count >= 2, "Function 'cons' needs at least two arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_VAL,
           "Function 'cons' must start with a value.");

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

ari *builtin_conlast(ari *a) {
  ARISSERT(a, a->count >= 2,
           "Function 'conlast' needs at least two arguments!");
  ARISSERT(a, a->cell[a->count - 1]->type == ARI_VAL,
           "Function 'conlast' must end with a value.");

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

ari *builtin_len(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'len' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'len' passed incorrect type!");

  int count = a->cell[0]->count;
  ari_del(a);
  return ari_val(count);
}

ari *builtin_init(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'init' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'init' passed incorrect type!");
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

ari *builtin_uninit(ari *a) {
  ARISSERT(a, a->count == 1, "Function 'uninit' passed too many arguments!");
  ARISSERT(a, a->cell[0]->type == ARI_QEXPR,
           "Function 'uninit' passed incorrect type!");
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
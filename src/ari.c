#include "../include/ari.h"
#include "../include/eval.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
ari *ari_val(double x) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_VAL;
  n->val = x;
  n->count = 0;
  n->cell = NULL;
  return n;
}

ari *ari_err(char *fmt, ...) {
  ari *n = malloc(sizeof(ari));
  if (n == NULL) {
    fprintf(stderr, "Crox FATAL ERROR: Out of memory during error creation " "(ari object)!\n");
    exit(1);
  }
  n->type = ARI_ERR;
  n->count = 0;
  n->cell = NULL;
  va_list va;
  va_start(va, fmt);
  n->err = malloc(512);
  if (n->err == NULL) {
    fprintf(stderr, "Crox FATAL ERROR: Out of memory during error creation "
                    "(error string)!\n");
    free(n);
    exit(1);
  }
  vsnprintf(n->err, 512, fmt, va);
  char *temp_err = realloc(n->err, strlen(n->err) + 1);
  if (temp_err == NULL) {
    fprintf(stderr, "Crox WARNING: Failed to reallocate error string to exact "
                    "size. Using larger buffer.\n");
  } else {
    n->err = temp_err;
  }
  va_end(va);
  return n;
}

ari *ari_sym(char *sym) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_SYM;
  if (sym) {
    n->sym = malloc(strlen(sym) + 1);
    strcpy(n->sym, sym);
  } else {
    n->sym = malloc(5);
    strcpy(n->sym, "NULL");
  }
  n->count = 0;
  n->cell = NULL;
  return n;
}

ari *ari_sexpr(void) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_SEXPR;
  n->count = 0;
  n->cell = NULL;
  return n;
}

ari *ari_qexpr(void) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_QEXPR;
  n->count = 0;
  n->cell = NULL;
  return n;
}

ari *ari_fun(abuiltin func) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_FUN;
  n->func = func;
  n->count = 0;
  n->cell = NULL;
  return n;
}

void ari_del(ari *v) {
  if (!v)
    return;

  switch (v->type) {
  case ARI_FUN:
  case ARI_VAL:
    break;
  case ARI_ERR:
    free(v->err);
    break;
  case ARI_SYM:
    free(v->sym);
    break;
  case ARI_SEXPR:
  case ARI_QEXPR:
    for (int i = 0; i < v->count; i++) {
      ari_del(v->cell[i]);
    }
    free(v->cell);
    break;
  }

  free(v);
}

ari *ari_add(ari *v, ari *x) {
  v->count++;
  ari **tmp = realloc(v->cell, sizeof(ari *) * v->count);
  if (!tmp) {
    ari_del(x);
    return ari_err("Out of memory!");
  }
  v->cell = tmp;
  v->cell[v->count - 1] = x;
  return v;
}

ari *ari_pop(ari *v, int i) {
  ari *x = v->cell[i];

  memmove(&v->cell[i], &v->cell[i + 1], sizeof(ari *) * (v->count - i - 1));
  v->count--;

  if (v->count == 0) {
    free(v->cell);
    v->cell = NULL;
  } else {
    ari **tmp = realloc(v->cell, sizeof(ari *) * v->count);
    if (tmp)
      v->cell = tmp;
  }

  return x;
}

ari *ari_take(ari *v, int i) {
  ari *x = ari_pop(v, i);
  ari_del(v);
  return x;
}

ari *ari_copy(ari *v) {
  if (!v)
    return NULL;

  ari *x = malloc(sizeof(ari));
  if (!x)
    return NULL;

  x->type = v->type;
  x->count = 0;
  switch (v->type) {
  case ARI_FUN:
    x->func = v->func;
    break;
  case ARI_VAL:
    x->val = v->val;
    break;
  case ARI_SYM:
    x->sym = malloc(strlen(v->sym) + 1);
    if (!x->sym) {
      free(x);
      return NULL;
    }
    strcpy(x->sym, v->sym);
    break;
  case ARI_ERR:
    x->err = malloc(strlen(v->err) + 1);
    if (!x->err) {
      free(x);
      return NULL;
    }
    strcpy(x->err, v->err);
    break;
  case ARI_QEXPR:
  case ARI_SEXPR:
    x->count = v->count;
    x->cell = malloc(sizeof(ari *) * v->count);
    if (!x->cell) {
      free(x);
      return NULL;
    }
    for (int i = 0; i < v->count; i++) {
      x->cell[i] = ari_copy(v->cell[i]);
      if (!x->cell[i]) {
        for (int j = 0; j < i; j++)
          ari_del(x->cell[j]);
        free(x->cell);
        free(x);
        return NULL;
      }
    }
    break;
  default:
    free(x);
    return NULL;
  }

  return x;
}

anv *anv_new() {
  anv *e = malloc(sizeof(anv));
  if (!e)
    return NULL;
  e->count = 0;
  e->sym = NULL;
  e->list = NULL;
  builtins(e);
  return e;
}

void anv_del(anv *e) {
  if (!e)
    return;
  for (int i = 0; i < e->count; i++) {
    free(e->sym[i]);
    ari_del(e->list[i]);
  }
  free(e->sym);
  free(e->list);
  free(e);
}

ari *anv_get(anv *e, ari *k) {
  if (!e || !k)
    return NULL;
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->sym[i], k->sym) == 0) {
      return ari_copy(e->list[i]);
    }
  }
  return ari_err("Symbol not found '%s'", k->sym);
}

void anv_put(anv *e, ari *k, ari *v) {
  if (!e || !k || !v)
    return;

  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->sym[i], k->sym) == 0) {
      ari_del(e->list[i]);
      e->list[i] = ari_copy(v);
      return;
    }
  }

  e->count++;
  e->list = realloc(e->list, sizeof(ari *) * e->count);
  e->sym = realloc(e->sym, sizeof(char *) * e->count);

  if (!e->list || !e->sym)
    return;

  e->list[e->count - 1] = ari_copy(v);
  e->sym[e->count - 1] = malloc(strlen(k->sym) + 1);
  if (!e->sym[e->count - 1])
    return;
  strcpy(e->sym[e->count - 1], k->sym);
}
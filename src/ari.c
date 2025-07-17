#include <stdlib.h>
#include <string.h>
#include "../include/ari.h"

ari *ari_val(double x) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_VAL;
  n->val = x;
  n->count = 0;
  n->cell = NULL;
  return n;
}

ari *ari_err(char *err) {
  ari *n = malloc(sizeof(ari));
  n->type = ARI_ERR;
  n->err = malloc(strlen(err) + 1);
  strcpy(n->err, err);
  n->count = 0;
  n->cell = NULL;
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

void ari_del(ari *v) {
  if (!v) return;

  switch (v->type) {
    case ARI_VAL: break;
    case ARI_ERR: free(v->err); break;
    case ARI_SYM: free(v->sym); break;
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
  ari **tmp = realloc(v->cell, sizeof(ari*) * v->count);
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
    if (tmp) v->cell = tmp;
  }

  return x;
}

ari *ari_take(ari *v, int i) {
  ari *x = ari_pop(v, i);
  ari_del(v);
  return x;
}

#ifndef ARITY_H
#define ARITY_H

// Error types for ari_err
enum errTypes { DIV_ZERO, MOD_ZERO, BAD_OP, BAD_NUM };

// Value types
enum typeOfNum { ARI_VAL, ARI_ERR, ARI_SYM, ARI_SEXPR, ARI_QEXPR };

// Forward declaration
typedef struct ari {
  int type;
  int count;

  union {
    double val;      // ARI_VAL
    char *sym;       // ARI_SYM
    char *err;       // ARI_ERR
  };

  struct ari **cell; // For (Q|S)-expressions
} ari;

// Constructors
ari *ari_val(double x);
ari *ari_err(char *err);
ari *ari_sym(char *sym);
ari *ari_sexpr(void);
ari *ari_qexpr(void);

// Memory management
void ari_del(ari *v);

// List ops
ari *ari_add(ari *v, ari *x);
ari *ari_pop(ari *v, int i);
ari *ari_take(ari *v, int i);

#endif

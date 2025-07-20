#ifndef ARITY_H
#define ARITY_H

struct ari;
struct anv;
typedef struct ari ari;
typedef struct anv anv;
typedef ari *(*abuiltin)(anv *, ari *);

// Error types for ari_err
enum errTypes { DIV_ZERO, MOD_ZERO, BAD_OP, BAD_NUM };

// Value types
enum typeOfNum { ARI_VAL, ARI_ERR, ARI_SYM, ARI_SEXPR, ARI_QEXPR, ARI_FUN };

// Forward declaration
struct ari {
  int type;
  int count;
  union {
    double val; // ARI_VAL
    char *sym;  // ARI_SYM
    char *err;  // ARI_ERR
    abuiltin func;
  };
  struct ari **cell; // For (Q|S)-expressions
};

struct anv {
  int count;
  char **sym;
  ari **list;
};

// Constructors
ari *ari_val(double x);
ari *ari_err(char *err);
ari *ari_sym(char *sym);
ari *ari_sexpr(void);
ari *ari_qexpr(void);
ari *ari_fun();

// Memory management
void ari_del(ari *v);

// List ops
ari *ari_add(ari *v, ari *x);
ari *ari_pop(ari *v, int i);
ari *ari_take(ari *v, int i);
ari *ari_copy(ari *v);

// env
anv *anv_new(void);
void anv_del(anv *e);
ari *anv_get(anv *e, ari *v);
void anv_put(anv *e, ari *v, ari *x);

#endif

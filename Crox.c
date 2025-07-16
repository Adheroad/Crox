#include "mpc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

static char buff[2048];

char *readline(char *prompt) {
  fputs("\033[m\033[31mcrox -> \033[0m", stdout);
  fgets(buff, 2048, stdin);
  char *cpy =
      malloc(strlen(buff) + 1); // Adjust size for actual length of the input
  if (cpy == NULL) {
    perror("malloc failed");
    exit(1);
    strcpy(cpy, buff);
    cpy[strlen(cpy) - 1] = '\0'; // Corrected null terminator
    return cpy;
  }

  void add_history(char *unused) {}

#else
#include <readline/history.h>
#include <readline/readline.h>
#endif

  // defined an enum with errortypes
  enum errTypes { DIV_ZERO, MOD_ZERO, BAD_OP, BAD_NUM };

  // Defined the types for ari data type
  enum typeOfNum { ARI_VAL, ARI_ERR, ARI_SYM, ARI_SEXPR, ARI_QEXPR };

  // declared my ari data type to handle sexpr
  typedef struct ari {
    int type;
    int count;

    union {
      long double val; // ARI_NUM
      char *sym;       // ARI_SYM
      char *err;       // ARI_ERR
    };

    struct ari **cell; // Only used when type == ARI_SEXPR
  } ari;

  // function to declare the ari data type
  ari *ari_val(long double x) {
    ari *n = malloc(sizeof(ari));
    n->type = ARI_VAL;
    n->val = x;
    n->count = 0;
    n->cell = NULL;
    return n;
  };

  // function to declate ari errored type
  ari *ari_err(char *err) {
    ari *n = malloc(sizeof(ari));
    n->type = ARI_ERR;
    n->err = malloc(strlen(err) + 1);
    strcpy(n->err, err);
    n->count = 0;
    n->cell = NULL;
    return n;
  }

  // Fucntion to declare ari symbol type
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

  // function for ari_sexpr type
  ari *ari_sexpr() {
    ari *n = malloc(sizeof(ari));
    n->type = ARI_SEXPR;
    n->count = 0;
    n->cell = NULL;
    return n;
  }

  ari *ari_qexpr() {
    ari *n = malloc(sizeof(ari));
    n->type = ARI_QEXPR;
    n->count = 0;
    n->cell = NULL;
    return n;
  }

  // constructor type function to delete all allocated memory
  void ari_del(ari * v) {
    // if v is null return early
    if (!v)
      return;

    // find the type of v and free the allocated memory
    switch (v->type) {
    case ARI_VAL:
      break;
    case ARI_ERR:
      free(v->err);
      break;
    case ARI_SYM:
      free(v->sym);
      break;
    case ARI_QEXPR:
    case ARI_SEXPR:
      // recursivly free every cell
      for (int i = 0; i < v->count; i++) {
        ari_del(v->cell[i]);
      }
      free(v->cell);
      break;
    }
    // atlast free v
    free(v);
  }

  ari *ari_pop(ari * n, int i) {
    ari *x = n->cell[i];

    memmove(&n->cell[i], &n->cell[i + 1], sizeof(ari *) * (n->count - i - 1));

    n->count--;
    if (n->count == 0) {
      free(n->cell);
      n->cell = NULL;
    } else {
      ari **tmp = realloc(n->cell, sizeof(ari *) * n->count);
      if (tmp)
        n->cell = tmp;
      // else keep old pointer to avoid losing memory
    }

    return x;
  }

  ari *ari_take(ari * n, int i) {
    ari *x = ari_pop(n, i);
    ari_del(n);
    return x;
  }

  ari *ari_read_num(mpc_ast_t * t) {
    errno = 0;
    char *end;
    long double x = strtod(t->contents, &end);

    if (errno == ERANGE)
      return ari_err("number out of range");
    if (end == t->contents || *end != '\0')
      return ari_err("invalid number");

    return ari_val(x);
  }

  ari *ari_add(ari * v, ari * x) {
    v->count++;
    ari **tmp = realloc(
        v->cell,
        sizeof(ari *) *
            v->count); // reallocating to temp to make sure memory never leaks
    if (!tmp) {
      ari_del(x); // Clean up x since it won't be used
      return ari_err("Out of memory!");
    }
    v->cell = tmp;

    v->cell[v->count - 1] = x;
    return v;
  }

  ari *builtin_op(ari * n, char *op) {
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

  ari *ari_eval(ari * n);

  ari *ari_eval_sexpr(ari * n) {
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

    ari *result = builtin_op(n, sym->sym);
    ari_del(sym);
    return result;
  }

  ari *ari_eval(ari * n) {
    if (n->type == ARI_SEXPR)
      return ari_eval_sexpr(n);
    return n;
  }

  ari *ari_read(mpc_ast_t * t) {
    // If the node is a number, return a numeric ari
    if (strstr(t->tag, "number")) {
      return ari_read_num(t);
    }

    // If the node is a symbol, return a symbol ari
    if (strstr(t->tag, "symbol")) {
      return ari_sym(t->contents);
    }

    if (strstr(t->tag, "qexpr")) {
      ari *v = ari_qexpr();
      for (int i = 0; i < t->children_num; i++) {
        // Skip brackets and regex
        if (strcmp(t->children[i]->contents, "{") == 0 ||
            strcmp(t->children[i]->contents, "}") == 0 ||
            strcmp(t->children[i]->contents, "[") == 0 ||
            strcmp(t->children[i]->contents, "]") == 0 ||
            strcmp(t->children[i]->tag, "regex") == 0) {
          continue;
        }
        v = ari_add(v, ari_read(t->children[i]));
      }
      return v;
    }

    // Handle S-expressions (Polish notation) - keep original logic
    if (strstr(t->tag, "sexpr")) {
      ari *v = ari_sexpr();
      for (int i = 0; i < t->children_num; i++) {
        // Skip brackets and regex
        if (strcmp(t->children[i]->contents, "[") == 0 ||
            strcmp(t->children[i]->contents, "]") == 0 ||
            strcmp(t->children[i]->tag, "regex") == 0) {
          continue;
        }
        v = ari_add(v, ari_read(t->children[i]));
      }
      return v;
    }

    // Handle infix expressions - convert to S-expressions
    if (strstr(t->tag, "expr") || strstr(t->tag, "term") ||
        strstr(t->tag, "power")) {

      ari *result = NULL;
      ari *left = NULL;
      char *operator = NULL;

      for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->tag, "regex") == 0) {
          continue;
        }

        // If it's a char node, it's an operator
        if (strcmp(t->children[i]->tag, "char") == 0) {
          operator = t->children[i]->contents;
          continue;
        }

        // If we haven't found an operator yet, this is the left operand
        if (!operator) {
          left = ari_read(t->children[i]);
        } else {
          // We have an operator, this is the right operand
          ari *right = ari_read(t->children[i]);

          // Create S-expression: (operator left right)
          if (!result) {
            result = ari_sexpr();
            result = ari_add(result, ari_sym(operator));
            result = ari_add(result, left);
            result = ari_add(result, right);
          } else {
            // For expressions like "1 + 2 + 3", we need to handle
            // left-associativity Convert to (+ (+ 1 2) 3)
            ari *new_result = ari_sexpr();
            new_result = ari_add(new_result, ari_sym(operator));
            new_result = ari_add(new_result, result);
            // new_result = ari_add(result, new_result);
            new_result = ari_add(new_result, right);
            result = new_result;
          }

          left = NULL; // Reset for next iteration
          operator = NULL;
        }
      }

      return result ? result : left;
    }

    // Handle factors - mostly just unwrap parentheses
    if (strstr(t->tag, "factor")) {
      for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0 ||
            strcmp(t->children[i]->contents, ")") == 0 ||
            strcmp(t->children[i]->tag, "regex") == 0) {
          continue;
        }
        return ari_read(t->children[i]);
      }
    }

    // // Special case: unwrap top-level '>' if it has exactly one child that
    // isn't noise
    if (strcmp(t->tag, ">") == 0) {
      int real_children = 0;
      int child_index = -1;

      for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->tag, "regex") != 0 &&
            strcmp(t->children[i]->tag, "char") != 0) {
          real_children++;
          child_index = i;
        }
      }

      // If there's only one real child, return it directly
      if (real_children == 1) {
        return ari_read(t->children[child_index]);
      }
    }

    // Fallback - shouldn't reach here with proper grammar
    return ari_err("Unknown AST node type");
  }

  void ari_print(ari * v);
  void ari_exp_print(ari * v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
      ari_print(v->cell[i]);

      if (i != v->count - 1)
        putchar(' ');
    }
    putchar(close);
  }

  // to print
  void ari_print(ari * n) {
    switch (n->type) {
    case ARI_VAL:
      if (floorl(n->val) == n->val) // if the result is integer
      {
        printf("%.0Lf", n->val);
      } else // if the result is decimal
      {
        printf("%Lg", n->val);
      }
      break;
    case ARI_ERR:
      printf("Error: %s", n->err);
      break;

    case ARI_SYM:
      printf("%s", n->sym);
      break;
    case ARI_QEXPR:
      ari_exp_print(n, '{', '}');
      break;
    case ARI_SEXPR:
      ari_exp_print(n, '[', ']');
      break;
    }
  }

  void ari_println(ari * n) {
    ari_print(n);
    putchar('\n');
  }

  // // The following function finds the no of leaves
  // int numNodes(mpc_ast_t * t)
  // {
  //   // If there are no children and the size of the root has some size
  //   if (t->children_num == 0 && strlen(t->contents) > 0) // i don't think i
  //   need && part
  //     return 1;
  //   int total = 1;
  //   for (int i = 0; i < t->children_num; i++)
  //   {
  //     total += numNodes(t->children[i]);
  //   }
  //   return total;
  // }

  // the following function describes the behaviour of operators
  // ari eval_op(ari x, char *op, ari y)
  // {

  //   if (x.type == ARI_ERR)
  //     return x;
  //   if (y.type == ARI_ERR)
  //     return y;
  //   switch (op[0])
  //   {
  //   case 'l':
  //     return ari_val(log(y.val) / log(x.val));
  //   case '^':
  //     return ari_val(powl(x.val, y.val));
  //   case '+':
  //     return ari_val(x.val + y.val);
  //   case '-':
  //     return ari_val(x.val - y.val);
  //   case '*':
  //     return ari_val(x.val * y.val);
  //   case '/':
  //     if (y.val == 0)
  //     {
  //       return ari_err(DIV_ZERO);
  //     }
  //     return ari_val(x.val / y.val);
  //   case '%':
  //     if (y.val == 0)
  //     {
  //       return ari_err(MOD_ZERO);
  //     }
  //     return ari_val(fmodl(x.val, y.val));
  //   }
  //   return ari_err(BAD_OP);
  // }

  // // This is the evaluator of the parser
  // ari eval(mpc_ast_t * t)
  // {
  //   // If the tag is a number turn it into an a decimal and return
  //   if (strstr(t->tag, "number"))
  //   {
  //     char *endptr;
  //     return ari_val(strtod(t->contents, &endptr)); // converts a numerical
  //     string into decimal
  //   }

  //   /* if the tag has a factor and it has one children
  //    * then that child must be a number. if it has more than one children
  //    * then the second child(child at 1) must be an expression
  //    * so call the evaluate function on it again*/
  //   if (strstr(t->tag, "factor"))
  //   {
  //     if (t->children_num == 1)
  //       return eval(t->children[0]);
  //     else
  //       return eval(t->children[1]);
  //   }

  //   /* Handle the power rule for right-associativity
  //    * If it has 1 child, it's just a factor
  //    * If it has 3 children, it's factor ^ power (right-associative) */
  //   if (strstr(t->tag, "power"))
  //   {
  //     if (t->children_num == 1)
  //     {
  //       // Just a factor, no power operation
  //       return eval(t->children[0]);
  //     }
  //     else if (t->children_num == 3)
  //     {
  //       // factor ^ power (right-associative)
  //       ari base = eval(t->children[0]);     // factor
  //       ari exponent = eval(t->children[2]); // power (recursive)
  //       return ari_val(pow(base.val, exponent.val));
  //     }
  //   }

  //   /* if the tag is term or expr then the children at one will eventually be
  //   a number (or expression)
  //    * and it's second child(child at 1) if it has one will be an operator
  //    always
  //    * and third will be an expression or a number eventually*/
  //   if (strstr(t->tag, "expr") || strstr(t->tag, "term"))
  //   {
  //     ari x = eval(t->children[0]);
  //     for (int i = 1; i < t->children_num; i += 2)
  //     {
  //       char *op = t->children[i]->contents;
  //       x = eval_op(x, op, eval(t->children[i + 1]));
  //     }
  //     return x;
  //   }

  //   return ari_err(BAD_NUM);
  // }

  int main(int argc, char **argv) {

    // Following I have described the parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Expression = mpc_new("expr");
    mpc_parser_t *Term = mpc_new("term");
    mpc_parser_t *Factor = mpc_new("factor");
    mpc_parser_t *Power = mpc_new("power");
    mpc_parser_t *Sfactor = mpc_new("sfactor");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *CroxParser = mpc_new("croxParser");

    /*
     * This here defines my language
     * main parser is croxParser which starts and takes only expr as an input
     * and ends expr takes either one term or a term with either + or - and
     * another term. i.e., the expr contains either 1 term or even number of
     * terms with +/- between them A term contains higher precedence operators
     * than + or - the term in turn contains either one factor or even number of
     * factors with these higher operators a factor is simiply a number or an
     * expression with paranthesis around
     *
     * Why was this decision to separate term and factor was taken?
     *     to maintain bodmas and precedence and remove the dependency on ().
     */

    mpca_lang(MPCA_LANG_DEFAULT,
              "number     : /-?[0-9]+(\\.[0-9]+)?/ ;"
              "symbol     : '+' | '-' | '*' | '/' | '%' | '^' | \"log\" |  "
              "\"list\" | \"head\" | \"tail\"| \"join\" | \"eval\" | \"cons\" "
              "| \"len\" | \"init\" ;"
              "sexpr      : '[' <symbol> <sfactor>* ']' ;"
              "qexpr      : '{' <symbol>? <sfactor>* '}' ;"
              "sfactor    : <number> | <sexpr> | <qexpr> | <expr> ;"
              "expr       : <term> (('+' | '-') <term>)* ;"
              "term       : <power> (('*' | '/' | '%') <power>)* ;"
              "power      : <factor> ('^' <power>)? ;"
              "factor     : <number> | <sexpr> | <qexpr> | '(' <expr> ')' ;"
              "croxParser : /^/ <expr> /$/ ;",
              Number, Symbol, Sexpr, Qexpr, Sfactor, Expression, Term, Power,
              Factor, CroxParser);

    puts("Welcome! You are using Crox.");
    puts("A terribly useless language to solve none of your problems.");
    puts("You are using version: 0.0.0.4");
    // printf("sizeof(ari) = %zu\n", sizeof(ari));
    // printf("sizeof(long double) = %zu\n", sizeof(long double));

    while (1) {
      char *input = readline("\033[m\033[31mcrox -> \033[0m");
      add_history(input); // function to remember last input given

      mpc_result_t r;

      if (mpc_parse("<stdin>", input, CroxParser,
                    &r)) // fucntion ot parse our result
      {

        mpc_ast_t *a =
            r.output; // storing the output of the parsed input in an AST

        // Evaluation of the ast using eval function
        // ari result = eval(a->children[1]);
        // print(result);
        // ari *result = ari_read(r.output);
        // ari_println(result);
        // ari_del(result);

        ari *result = ari_eval(ari_read(r.output));
        ari_println(result);
        ari_del(result);

        /*
          We can use thr fullowing function to print the ast tree.
           mpc_ast_print(a);
        */

        // mpc_ast_print(a);
        mpc_ast_delete(a);
      } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }

      free(input);
    }

    // Clear up the parsers after use
    mpc_cleanup(10, Number, Symbol, Sexpr, Qexpr, Sfactor, Expression, Term,
                Power, Factor, CroxParser);

    return 0;
  }

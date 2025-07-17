#include "../include/io.h"
#include "../include/ari.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ari *ari_read_num(mpc_ast_t *t) {
  errno = 0;
  char *end;
  long double x = strtod(t->contents, &end);

  if (errno == ERANGE)
    return ari_err("number out of range");
  if (end == t->contents || *end != '\0')
    return ari_err("invalid number");

  return ari_val(x);
}
ari *ari_read(mpc_ast_t *t) {
  // Add safety checks
  if (!t) {
    return ari_err("Null AST node");
  }

  if (!t->tag) {
    return ari_err("AST node has no tag");
  }

  // If the node is a number, return a numeric ari
  if (strstr(t->tag, "number")) {
    return ari_read_num(t);
  }

  // If the node is a symbol, return a symbol ari
  if (strstr(t->tag, "symbol")) {
    return ari_sym(t->contents);
  }

  // Handle Q-expressions
  if (strstr(t->tag, "qexpr")) {
    ari *v = ari_qexpr();
    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;
      if (t->children[i]->contents &&
          (strcmp(t->children[i]->contents, "{") == 0 ||
           strcmp(t->children[i]->contents, "}") == 0 ||
           strcmp(t->children[i]->contents, "[") == 0 ||
           strcmp(t->children[i]->contents, "]") == 0)) {
        continue;
      }
      ari *child_result = ari_read(t->children[i]);
      if (child_result) {
        v = ari_add(v, child_result);
      }
    }
    return v;
  }

  // Handle S-expressions (Polish notation)
  if (strstr(t->tag, "sexpr")) {
    ari *v = ari_sexpr();
    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;
      if (t->children[i]->contents &&
          (strcmp(t->children[i]->contents, "[") == 0 ||
           strcmp(t->children[i]->contents, "]") == 0)) {
        continue;
      }
      ari *child_result = ari_read(t->children[i]);
      if (child_result) {
        v = ari_add(v, child_result);
      }
    }
    return v;
  }

  // Handle infix expressions - convert to S-expressions
  if (strstr(t->tag, "expr") || strstr(t->tag, "term") || strstr(t->tag, "power")) {
    ari *result = NULL;
    ari *left = NULL;
    char *operator = NULL;

    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;

      if (strcmp(t->children[i]->tag, "char") == 0) {
        operator = t->children[i]->contents;
        continue;
      }

      if (!operator) {
        left = ari_read(t->children[i]);
      } else {
        ari *right = ari_read(t->children[i]);
        if (!result) {
          result = ari_sexpr();
          result = ari_add(result, ari_sym(operator));
          result = ari_add(result, left);
          result = ari_add(result, right);
        } else {
          ari *new_result = ari_sexpr();
          new_result = ari_add(new_result, ari_sym(operator));
          new_result = ari_add(new_result, result);
          new_result = ari_add(new_result, right);
          result = new_result;
        }
        left = NULL;
        operator = NULL;
      }
    }

    return result ? result : left;
  }

  // Handle statement rule - just unwrap to the actual content
  if (strstr(t->tag, "statement")) {
    int symbol_index = -1;
    int arg_count = 0;

    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;
      if (strstr(t->children[i]->tag, "symbol")) {
        symbol_index = i;
      } else {
        arg_count++;
      }
    }

    if (symbol_index != -1 && arg_count > 0) {
      ari *v = ari_sexpr();
      v = ari_add(v, ari_sym(t->children[symbol_index]->contents));
      for (int i = 0; i < t->children_num; i++) {
        if (!t->children[i] || !t->children[i]->tag)
          continue;
        if (strcmp(t->children[i]->tag, "regex") == 0)
          continue;
        if (i == symbol_index)
          continue;
        ari *arg = ari_read(t->children[i]);
        if (arg) {
          v = ari_add(v, arg);
        }
      }
      return v;
    }

    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;
      return ari_read(t->children[i]);
    }
  }

  // Handle factors - mostly just unwrap parentheses
  if (strstr(t->tag, "factor")) {
    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") == 0)
        continue;
      if (t->children[i]->contents &&
          (strcmp(t->children[i]->contents, "(") == 0 ||
           strcmp(t->children[i]->contents, ")") == 0)) {
        continue;
      }
      return ari_read(t->children[i]);
    }
  }

  // Special case: unwrap top-level '>' if it has exactly one child that isn’t noise
  if (strcmp(t->tag, ">") == 0) {
    int real_children = 0;
    int child_index = -1;

    for (int i = 0; i < t->children_num; i++) {
      if (!t->children[i] || !t->children[i]->tag)
        continue;
      if (strcmp(t->children[i]->tag, "regex") != 0 &&
          strcmp(t->children[i]->tag, "char") != 0) {
        real_children++;
        child_index = i;
      }
    }

    if (real_children == 1) {
      return ari_read(t->children[child_index]);
    }
  }

  // Fallback - shouldn’t reach here with proper grammar
  return ari_err("Unknown AST node type");
}


// ari *ari_read(mpc_ast_t *t) {
//   // Add safety checks
//   if (!t) {
//     return ari_err("Null AST node");
//   }

//   if (!t->tag) {
//     return ari_err("AST node has no tag");
//   }

//   // If the node is a number, return a numeric ari
//   if (strstr(t->tag, "number")) {
//     return ari_read_num(t);
//   }

//   // If the node is a symbol, return a symbol ari
//   if (strstr(t->tag, "symbol")) {
//     return ari_sym(t->contents);
//   }

//   // Handle Q-expressions
//   if (strstr(t->tag, "qexpr")) {
//     ari *v = ari_qexpr();
//     for (int i = 0; i < t->children_num; i++) {
//       // Add null checks
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;

//       // Skip regex first
//       if (strcmp(t->children[i]->tag, "regex") == 0) {
//         continue;
//       }

//       // Skip brackets
//       if (t->children[i]->contents &&
//           (strcmp(t->children[i]->contents, "{") == 0 ||
//            strcmp(t->children[i]->contents, "}") == 0 ||
//            strcmp(t->children[i]->contents, "[") == 0 ||
//            strcmp(t->children[i]->contents, "]") == 0)) {
//         continue;
//       }

//       ari *child_result = ari_read(t->children[i]);
//       if (child_result) {
//         v = ari_add(v, child_result);
//       }
//     }
//     return v;
//   }

//   // Handle S-expressions (Polish notation)
//   if (strstr(t->tag, "sexpr")) {
//     ari *v = ari_sexpr();
//     for (int i = 0; i < t->children_num; i++) {
//       // Add null checks
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;

//       // Skip regex first
//       if (strcmp(t->children[i]->tag, "regex") == 0) {
//         continue;
//       }

//       // Skip brackets
//       if (t->children[i]->contents &&
//           (strcmp(t->children[i]->contents, "[") == 0 ||
//            strcmp(t->children[i]->contents, "]") == 0)) {
//         continue;
//       }

//       ari *child_result = ari_read(t->children[i]);
//       if (child_result) {
//         v = ari_add(v, child_result);
//       }
//     }
//     return v;
//   }

//   // Handle statement rule - just unwrap to the actual content
//   if (strstr(t->tag, "statement")) {
//     // Check if this is a function call (symbol followed by arguments)
//     int symbol_index = -1;
//     int arg_count = 0;

//     for (int i = 0; i < t->children_num; i++) {
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;
//       if (strcmp(t->children[i]->tag, "regex") == 0)
//         continue;

//       if (strstr(t->children[i]->tag, "symbol")) {
//         symbol_index = i;
//       } else {
//         arg_count++;
//       }
//     }

//     // If we have a symbol and arguments, treat as function call
//     if (symbol_index != -1 && arg_count > 0) {
//       ari *v = ari_sexpr();

//       // Add the function symbol
//       v = ari_add(v, ari_sym(t->children[symbol_index]->contents));

//       // Add all arguments
//       for (int i = 0; i < t->children_num; i++) {
//         if (!t->children[i] || !t->children[i]->tag)
//           continue;
//         if (strcmp(t->children[i]->tag, "regex") == 0)
//           continue;
//         if (i == symbol_index)
//           continue;

//         ari *arg = ari_read(t->children[i]);
//         if (arg) {
//           v = ari_add(v, arg);
//         }
//       }

//       return v;
//     }

//     // If not a function call, delegate to the single meaningful child
//     for (int i = 0; i < t->children_num; i++) {
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;
//       if (strcmp(t->children[i]->tag, "regex") == 0)
//         continue;

//       return ari_read(t->children[i]);
//     }
//   }

//   // Handle infix expressions - convert to S-expressions
//   if (strstr(t->tag, "expr") || strstr(t->tag, "term") ||
//       strstr(t->tag, "power")) {
//     ari *result = NULL;
//     ari *left = NULL;
//     char *operator = NULL;

//     for (int i = 0; i < t->children_num; i++) {
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;

//       if (strcmp(t->children[i]->tag, "regex") == 0) {
//         continue;
//       }

//       // If it's a char node, it's an operator
//       if (strcmp(t->children[i]->tag, "char") == 0) {
//         operator = t->children[i]->contents;
//         continue;
//       }

//       // If we haven't found an operator yet, this is the left operand
//       if (!operator) {
//         left = ari_read(t->children[i]);
//       } else {
//         // We have an operator, this is the right operand
//         ari *right = ari_read(t->children[i]);

//         // Create S-expression: (operator left right)
//         if (!result) {
//           result = ari_sexpr();
//           result = ari_add(result, ari_sym(operator));
//           result = ari_add(result, left);
//           result = ari_add(result, right);
//         } else {
//           // For expressions like "1 + 2 + 3", handle left-associativity
//           // Convert to (+ (+ 1 2) 3)
//           ari *new_result = ari_sexpr();
//           new_result = ari_add(new_result, ari_sym(operator));
//           new_result = ari_add(new_result, result);
//           new_result = ari_add(new_result, right);
//           result = new_result;
//         }

//         left = NULL; // Reset for next iteration
//         operator = NULL;
//       }
//     }

//     return result ? result : left;
//   }

//   // Handle factors - mostly just unwrap parentheses
//   if (strstr(t->tag, "factor")) {
//     for (int i = 0; i < t->children_num; i++) {
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;

//       // Skip regex first
//       if (strcmp(t->children[i]->tag, "regex") == 0) {
//         continue;
//       }

//       // Skip parentheses
//       if (t->children[i]->contents &&
//           (strcmp(t->children[i]->contents, "(") == 0 ||
//            strcmp(t->children[i]->contents, ")") == 0)) {
//         continue;
//       }

//       return ari_read(t->children[i]);
//     }
//   }

//   // Special case: unwrap top-level '>' if it has exactly one child that
//   isn't
//   // noise
//   if (strcmp(t->tag, ">") == 0) {
//     int real_children = 0;
//     int child_index = -1;

//     for (int i = 0; i < t->children_num; i++) {
//       if (!t->children[i] || !t->children[i]->tag)
//         continue;

//       if (strcmp(t->children[i]->tag, "regex") != 0 &&
//           strcmp(t->children[i]->tag, "char") != 0) {
//         real_children++;
//         child_index = i;
//       }
//     }

//     // If there's only one real child, return it directly
//     if (real_children == 1) {
//       return ari_read(t->children[child_index]);
//     }
//   }

//   // Fallback - shouldn't reach here with proper grammar
//   return ari_err("Unknown AST node type");
// }

void ari_print(ari *v);
void ari_exp_print(ari *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    ari_print(v->cell[i]);

    if (i != v->count - 1)
      putchar(' ');
  }
  putchar(close);
}

// to print
void ari_print(ari *n) {
  switch (n->type) {
  case ARI_VAL:
    if (floorl(n->val) == n->val) // if the result is integer
    {
      printf("%.0f", n->val);
    } else // if the result is decimal
    {
      printf("%g", n->val);
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

void ari_println(ari *n) {
  ari_print(n);
  putchar('\n');
}
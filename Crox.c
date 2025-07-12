#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32

static char buff[2048];

char *readline(char *prompt)
{
  fputs("\033[m\033[31mcrox -> \033[0m", stdout);
  fgets(buff, 2048, stdin);
  char *cpy = malloc(strlen(buff) + 1); // Adjust size for actual length of the input
  if (cpy == NULL)
  {
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
  enum errTypes
  {
    DIV_ZERO,
    MOD_ZERO,
    BAD_OP,
    BAD_NUM
  };

  // Defined the types for num data type
  enum typeOfNum
  {
    NUM_VAL,
    NUM_ERR
  };

  // declared my num data type
  typedef struct
  {
    int type;
    union
    {
      long double val;
      int err;
    };
  } num;

  // function to declare the num data type
  num num_val(long double x)
  {
    num n;
    n.type = NUM_VAL;
    n.val = x;
    return n;
  };

  // function to declate num errored type
  num num_err(int x)
  {
    num n;
    n.type = NUM_ERR;
    n.val = x;
    return n;
  }

  // to print
  void print(num n)
  {
    switch (n.type)
    {
    case NUM_VAL:
      if (floorl(n.val) == n.val) // if the result is integer
      {
        printf("%.0Lf\n", n.val);
      }
      else // if the result is decimal
      {
        printf("%Lg\n", n.val);
      }
      break;
    case NUM_ERR:
      if (n.err == DIV_ZERO)
      {
        printf("Error: Division By Zero!");
      }
      if (n.err == BAD_OP)
      {
        printf("Error: Invalid Operator!");
      }
      if (n.err == BAD_NUM)
      {
        printf("Error: Invalid Number!");
      }
      break;
    }
  }

  void println(num n)
  {
    print(n);
    putchar('\n');
  }

  // The following function finds the no of leaves
  int numNodes(mpc_ast_t * t)
  {
    // If there are no children and the size of the root has some size
    if (t->children_num == 0 && strlen(t->contents) > 0) // i don't think i need && part
      return 1;
    int total = 1;
    for (int i = 0; i < t->children_num; i++)
    {
      total += numNodes(t->children[i]);
    }
    return total;
  }

  // the following function describes the behaviour of operators
  num eval_op(num x, char *op, num y)
  {

    if(x.type == NUM_ERR) return x;
    if(y.type == NUM_ERR) return y;
    switch (op[0])
    {
    case 'l':
      return num_val(log(y.val) / log(x.val));
    case '^':
      return num_val(powl(x.val, y.val));
    case '+':
      return num_val(x.val + y.val);
    case '-':
      return num_val(x.val - y.val);
    case '*':
      return num_val(x.val * y.val);
    case '/':
      if (y.val == 0)
      {
        return num_err(DIV_ZERO);
      }
      return num_val(x.val / y.val);
    case '%':
     if (y.val == 0)
      {
        return num_err(MOD_ZERO);
      }
      return num_val(fmodl(x.val, y.val));
    }
    return num_err(BAD_OP);
  }

  // This is the evaluator of the parser
  num eval(mpc_ast_t * t)
  {
    // If the tag is a number turn it into an a decimal and return
    if (strstr(t->tag, "number"))
    {
      char *endptr;
      return num_val(strtod(t->contents, &endptr)); // converts a numerical string into decimal
    }

    /* if the tag has a factor and it has one children
     * then that child must be a number. if it has more than one children
     * then the second child(child at 1) must be an expression
     * so call the evaluate function on it again*/
    if (strstr(t->tag, "factor"))
    {
      if (t->children_num == 1)
        return eval(t->children[0]);
      else
        return eval(t->children[1]);
    }

    /* Handle the power rule for right-associativity
     * If it has 1 child, it's just a factor
     * If it has 3 children, it's factor ^ power (right-associative) */
    if (strstr(t->tag, "power"))
    {
      if (t->children_num == 1)
      {
        // Just a factor, no power operation
        return eval(t->children[0]);
      }
      else if (t->children_num == 3)
      {
        // factor ^ power (right-associative)
        num base = eval(t->children[0]);     // factor
        num exponent = eval(t->children[2]); // power (recursive)
        return num_val(pow(base.val, exponent.val));
      }
    }

    /* if the tag is term or expr then the children at one will eventually be a number (or expression)
     * and it's second child(child at 1) if it has one will be an operator always
     * and third will be an expression or a number eventually*/
    if (strstr(t->tag, "expr") || strstr(t->tag, "term"))
    {
      num x = eval(t->children[0]);
      for (int i = 1; i < t->children_num; i += 2)
      {
        char *op = t->children[i]->contents;
        x = eval_op(x, op, eval(t->children[i + 1]));
      }
      return x;
    }

    return num_err(BAD_NUM);
  }

  int main(int argc, char **argv)
  {

    // Following I have described the parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Expression = mpc_new("expr");
    mpc_parser_t *Term = mpc_new("term");
    mpc_parser_t *Factor = mpc_new("factor");
    mpc_parser_t *Power = mpc_new("power");
    mpc_parser_t *CroxParser = mpc_new("croxParser");

    /*
     * This here defines my language
     * main parser is croxParser which starts and takes only expr as an input and ends
     * expr takes either one term or a term with either + or - and another term.
     * i.e., the expr contains either 1 term or even number of terms with +/- between them
     * A term contains higher precedence operators than + or -
     * the term in turn contains either one factor or even number of factors with these higher operators
     * a factor is simiply a number or an expression with paranthesis around
     *
     * Why was this decision to separate term and factor was taken?
     *     to maintain bodmas and precedence and remove the dependency on ().
     */

    mpca_lang(MPCA_LANG_DEFAULT,
              "number     : /-?[0-9]+(\\.[0-9]+)?/ ;"                        // takes numbers and decimals
              "expr       : <term> (('+' | '-') <term>)* ;"                  // handles +/- using bodmas
              "term       : <power> ((\"log\" |'*' | '/' | '%') <power>)* ;" // handles * / % using bodmas
              "power      : <factor> ('^' <power>)? ;"                       // RIGHT-ASSOCIATIVE power operator
              "factor     : <number> | '(' <expr> ')' ;"                     // handles ()
              "croxParser : /^/ <expr> /$/ ;",                               // parses the expressions
              Number, Expression, Power, Term, Power, Factor, CroxParser);

    puts("Welcome! You are using Crox.");
    puts("A terribly useless language to solve none of your problems.");
    puts("You are using version: 0.0.0.3");

    while (1)
    {
      char *input = readline("\033[m\033[31mcrox -> \033[0m");
      add_history(input); // function to remember last input given

      mpc_result_t r;

      if (mpc_parse("<stdin>", input, CroxParser, &r)) // fucntion ot parse our result
      {

        mpc_ast_t *a = r.output; // storing the output of the parsed input in an AST

        // Evaluation of the ast using eval function
        num result = eval(a->children[1]);
        print(result);

        /*
          We can use thr fullowing function to print the ast tree.
          //  mpc_ast_print(a);
        */
       
        mpc_ast_delete(a);
      }
      else
      {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }

      free(input);
    }

    // Clear up the parsers after use
    mpc_cleanup(6, Number, Expression, Power, Term, Factor, CroxParser);

    return 0;
  }

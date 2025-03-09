#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32

static char buff[2048];

char* readline(char* prompt){
  fputs("\033[m\033[31mcrox -> \033[0m", stdout);
  fgets(buff, 2048, stdin);
  char* cpy = malloc(strlen(buff)+1);  // Adjust size for actual length of the input
  if (cpy == NULL) {
      perror("malloc failed");
      exit(1);
  strcpy(cpy, buff);
  cpy[strlen(cpy)-1] = '\0';  // Corrected null terminator
  return cpy;
}

void add_history(char* unused){}

#else
#include <readline/history.h>
#include <readline/readline.h>
#endif

// The following function finds the no of leaves
int numNodes(mpc_ast_t* t){
  if(t->children_num == 0 && strlen(t->contents)>0) return 1;
  int total = 0;
  for(int i = 0; i< t->children_num; i++){
    total+=numNodes(t->children[i]);
  }
  return total;
}

// the following function describes the behaviour of operators
long double eval_op(long double x, char* op, long double y){
  switch(op[0]){
    case '^':
      return powl(x, y);
    case '+' :
      return x+y;
    case '-':
      return x-y;
    case '*':
      return x*y;
    case '/':
      if(y==0){
        printf("Can't divide by zero you dummy!");
        break;
      }
      return x/y;
    case '%':
      if (y == 0) {
        printf("Can't modulo by zero you dummy!\n");
        return 0;
      }
      return fmodl(x, y);
  }
  return 0;
}

// This is the evaluator of the parser
long double eval(mpc_ast_t* t) {
  // If the tag is a number turn it into an a decimal and return
  if (strstr(t->tag, "number")) {
    char* endptr;
    return strtod(t->contents, &endptr);
  }

  /* if the tag has a factor and it has one children
   * then that child must be a number. if it has more than one children
   * then the second child(child at 1) must be an expression
   * so call the evaluate function on it again*/ 
   if (strstr(t->tag, "factor")) {
      if (t->children_num == 1) 
        return eval(t->children[0]);
      else 
        return eval(t->children[1]);  
    }

  /* if the tag is term or expr then the children at one will eventually be a number (or expression)
   * and it's second child(child at 1) if it has one will be an operator always
   * and third will be an expression or a number eventually*/
  if(strstr(t->tag, "expr")||strstr(t->tag, "term")){
    long double x = eval(t->children[0]);
    for(int i = 1; i< t->children_num; i+=2){
      char* op = t->children[i]->contents;
      x = eval_op(x, op, eval(t->children[i+1]));
    }
    return x;
  }


  return 0;  
}


int main(int argc, char** argv){
  
  // Following I have described the parsers
  mpc_parser_t* Number     = mpc_new("number");
  mpc_parser_t* Expression = mpc_new("expr");
  mpc_parser_t* Term       = mpc_new("term");
  mpc_parser_t* Factor     = mpc_new("factor");
  mpc_parser_t* CroxParser = mpc_new("croxParser");

  mpca_lang(MPCA_LANG_DEFAULT,
    "number     : /-?[0-9]+(\\.[0-9]+)?/ ;" // takes numbers and decimals
    "expr       : <term> (('+' | '-') <term>)* ;" // handles +/- using bodmas
    "term       : <factor> (('^'|'*' | '/' | '%') <factor>)* ;" // handles * / % usind bodmas
    "factor     : <number> | '(' <expr> ')' ;" // handles ()
    "croxParser : /^/ <expr> /$/ ;", // parses the expressions
    Number, Expression, Term, Factor, CroxParser);

  puts("Welcome! You are using Crox.");
  puts("A terribly useless language to solve none of your problems.");
  puts("You are using version: 0.0.0.3");

  while(1){

    char * input = readline("\033[m\033[31mcrox -> \033[0m");
    add_history(input);

    mpc_result_t r;
    if(mpc_parse("<stdin>", input, CroxParser, &r)){
       mpc_ast_t* a = r.output;
      long double result = eval(a->children[1]);
      printf("%.2Lf\n", result);
      mpc_ast_delete(a);
    }else{
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(5, Number, Expression, Term, Factor, CroxParser);

  return 0;
}

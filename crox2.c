
#include "mpc.h"

#ifdef _WIN32

static char buff[2048];

char* readline(char* prompt){
  fputs("\033[m\033[31mcrox -> \033[0m", stdout);
  fgets(buff, 2048, stdin);
  char* cpy = malloc(strlen(buff)+1);  // Adjust size for actual length of the input
  if (cpy == NULL) {
      perror("malloc failed");
      exit(1);
  }
  strcpy(cpy, buff);
  cpy[strlen(cpy)-1] = '\0';  // Corrected null terminator
  return cpy;
}

void add_history(char* unused){}

#else
#include <readline/history.h>
#include <readline/readline.h>
#endif

int main(int argc, char** argv){
  
  mpc_parser_t* Number     = mpc_new("number");
  mpc_parser_t* Expression = mpc_new("expr");
  mpc_parser_t* Term       = mpc_new("term");
  mpc_parser_t* Factor     = mpc_new("factor");
  mpc_parser_t* CroxParser = mpc_new("croxParser");

  mpca_lang(MPCA_LANG_DEFAULT,
    "number     : /-?[0-9]+(\\.[0-9]+)?/ ;"
    "expr       : <term> (('+' | '-'|) <term>)* ;"
    "term       : <factor> (('*' | '/' | '%') <factor>)* ;"
    "factor     : <number> | '(' <expr> ')' ;"
    "croxParser : /^/ <expr> /$/ ;",
    Number, Expression, Term, Factor, CroxParser);

  // Version Information of crox
  puts("Welcome! You are using Crox.");
  puts("A terribly useless language to solve none of your problems.");
  puts("You are using version: 0.0.0.1");

  while(1){

    char * input = readline("\033[m\033[31mcrox -> \033[0m");
    add_history(input);

    mpc_result_t r;
    if(mpc_parse("<stdin>", input, CroxParser, &r)){
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    }else{
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(5, Number, Expression, Term, Factor, CroxParser);

  return 0;
}

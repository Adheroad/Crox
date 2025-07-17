#include "../include/parser.h"

// Define parser pointers
mpc_parser_t *Number;
mpc_parser_t *Symbol;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Sfactor;
mpc_parser_t *Expression;
mpc_parser_t *Term;
mpc_parser_t *Power;
mpc_parser_t *Factor;
mpc_parser_t *Statement;
mpc_parser_t *CroxParser;

// Initialize the grammar
void parser_init(void) {
  Number = mpc_new("number");
  Symbol = mpc_new("symbol");
  Sexpr = mpc_new("sexpr");
  Qexpr = mpc_new("qexpr");
  Sfactor = mpc_new("sfactor");
  Expression = mpc_new("expr");
  Term = mpc_new("term");
  Power = mpc_new("power");
  Factor = mpc_new("factor");
  Statement = mpc_new("statement");
  CroxParser = mpc_new("croxParser");

  /*
   * This here defines my language
   * main parser is croxParser which starts and takes only expr as an input
   * and ends expr takes either one term or a term with either + or - and
   * another term. i.e., the expr contains either 1 term or even number of
   * terms with +/- between them A term contains higher precedence operators
   * than + or - the term in turn contains either one factor or even number
   * of factors with these higher operators a factor is simiply a number or
   * an expression with paranthesis around
   *
   * Why was this decision to separate term and factor was taken?
   *     to maintain bodmas and precedence and remove the dependency on ().
   */

  mpca_lang(
      MPCA_LANG_DEFAULT,
      "number     : /-?[0-9]+(\\.[0-9]+)?/ ;"
      "symbol     : '+' | '-' | '*' | '/' | '%' | '^' | \"log\" |  "
      "\"list\" | \"head\" | \"unhead\" | \"tail\"| \"untail\" | \"join\" "
      "| \"eval\" | \"cons\" | \"conlast\" | \"len\" | \"init\" | \"uninit\" ;"
      "sexpr      : '[' <symbol> <sfactor>* ']' ;"
      "qexpr      : '{' <symbol>? <sfactor>* '}' ;"
      "sfactor    : <number> | <sexpr> | <qexpr> | <expr> ;"
      "expr       : <term> (('+' | '-') <term>)* ;"
      "term       : <power> (('*' | '/' | '%') <power>)* ;"
      "power      : <factor> ('^' <power>)? ;"
      "factor     : <number> | <sexpr> | <qexpr> | '(' <expr> ')' ;"
      "statement  : <expr> | <sexpr> | <qexpr> | <symbol> <sfactor>* ;"
      "croxParser : /^/ <statement> /$/ ;",
      Number, Symbol, Sexpr, Qexpr, Sfactor, Expression, Term, Power, Factor,
      Statement, CroxParser);
}

// Cleanup
void parser_cleanup(void) {
  mpc_cleanup(11, Number, Symbol, Sexpr, Qexpr, Sfactor, Expression, Term,
              Power, Factor, Statement, CroxParser);
}

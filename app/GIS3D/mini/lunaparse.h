// (c) by Stefan Roettger

/*

This module parses the RPN-style language LUNA according to the following EBNF grammar:

LUNA        ::= { include | declaration }
include     ::= "include" <string>
declaration ::= var_decl | array_decl | ref_decl | func_decl [ ";" ]
var_decl    ::= "var" <var-id> [ "=" | ":=" expression ]
par_decl    ::= "par" <var-id>
array_decl  ::= "array" [ "byte" ] <array-id> [ "[" expression "]" ]
ref_decl    ::= "ref" [ "byte" ] <ref-id>
list_decl   ::= par_decl | ref_decl { "," par_decl | ref_decl }
func_decl   ::= "main" | ( "func" <func-id> ) "(" [ list_decl ] ")" "{" { statement } "}"
statement   ::= ( <var-id> ( "=" | ":=" expression ) | "++" | "--" ) |
                ( <array-id> | <ref-id> "[" expression "]" ( "=" | ":=" expression ) | "++" | "--" ) |
                ( <func-id> "(" [ expression { [ "," ] expression } ] ")" ) |
                ( "{" { declaration | statement } "}" ) |
                ( "if" "(" expression ")" statement [ "else" statement ] ) |
                ( "while" "(" expression ")" statement ) |
                ( "repeat" statement "until" "(" expression ")" ) |
                ( "for" "(" statement "," expression "," statement ")" statement ) |
                ( "return" [ "(" expression ")" ] ) |
                ( "warn" ) [ ";" ]
expression  ::= ( ["-"]<float-val> ) |
                ( <var-id> ) |
                ( <array-id> | <ref-id> [ "[" expression "]" ] ) |
                ( <func-id> | alpha-op "(" [ expression { [ "," ] expression } ] ")" ) |
                ( "(" [ operator ] { expression } ")" ) |
                ( "size" "(" array-id | ref-id ")" ) |
                ( "true" | "false" )
operator    ::= "+" | "-" | "*" | "/" | "%" |
                "=" | "<>" | "<" | ">" | "<=" | ">=" |
                "&" | "|" | "!"
alpha-op    ::= "min" | "max" | "abs" |
                "sqr" | "sqrt" | "exp" | "log" | "pow" |
                "sin" | "cos" | "tan" | "atan" | "atan2"

*/

#ifndef LUNAPARSE_H
#define LUNAPARSE_H

#include "minibase.h"

#include "lunascan.h"
#include "lunacode.h"

class lunaparse
   {
   public:

   //! default constructor
   lunaparse();

   //! destructor
   ~lunaparse();

   void setLUNAcode(const char *code) {SCANNER.setcode(code);}
   void setLUNAcode(const char *code,int bytes) {SCANNER.setcode(code,bytes);}
   void parseLUNA();

   void print();
   void printtokens();
   void printtoken();

   lunascan *getscanner() {return(&SCANNER);}
   lunacode *getcode() {return(&CODE);}

   protected:

   enum
      {
      LUNA_INCLUDE=0,
      LUNA_VAR=1,
      LUNA_PAR=2,
      LUNA_ARRAY=3,
      LUNA_REF=4,
      LUNA_BYTE=5,
      LUNA_MAIN=6,
      LUNA_FUNC=7,
      LUNA_IF=8,
      LUNA_ELSE=9,
      LUNA_WHILE=10,
      LUNA_REPEAT=11,
      LUNA_UNTIL=12,
      LUNA_FOR=13,
      LUNA_RETURN=14,
      LUNA_WARN=15,
      LUNA_NULL=16,
      LUNA_PARENLEFT=17,
      LUNA_PARENRIGHT=18,
      LUNA_BRACELEFT=19,
      LUNA_BRACERIGHT=20,
      LUNA_BRACKETLEFT=21,
      LUNA_BRACKETRIGHT=22,
      LUNA_ASSIGN=23,
      LUNA_COMMA=24,
      LUNA_INC=25,
      LUNA_DEC=26,
      LUNA_SIZE=27,
      LUNA_TRUE=28,
      LUNA_FALSE=29,
      LUNA_ADD=30,
      LUNA_SUB=31,
      LUNA_MUL=32,
      LUNA_DIV=33,
      LUNA_MOD=34,
      LUNA_EQ=35,
      LUNA_NEQ=36,
      LUNA_LT=37,
      LUNA_GT=38,
      LUNA_LE=39,
      LUNA_GE=40,
      LUNA_AND=41,
      LUNA_OR=42,
      LUNA_NOT=43,
      LUNA_MIN=44,
      LUNA_MAX=45,
      LUNA_ABS=46,
      LUNA_SQR=47,
      LUNA_SQRT=48,
      LUNA_EXP=49,
      LUNA_LOG=50,
      LUNA_POW=51,
      LUNA_SIN=52,
      LUNA_COS=53,
      LUNA_TAN=54,
      LUNA_ATAN=55,
      LUNA_ATAN2=56,
      LUNA_VAR_GLB=57,
      LUNA_VAR_LOC=58,
      LUNA_ARRAY_GLB=59,
      LUNA_ARRAY_LOC=60,
      LUNA_REF_GLB=61,
      LUNA_REF_LOC=62,
      LUNA_FUNCTION=63
      };

   lunascan SCANNER;
   lunacode CODE;

   void addLUNAtokens();

   private:

   int MAIN;

   int VAR_NUM;

   void parse_include();
   int parse_var_decl(BOOLINT loc,BOOLINT par,BOOLINT array,BOOLINT ref,BOOLINT stat,int *VAR_LOC_NUM=NULL);
   void parse_par_decl(int *PAR_LOC_NUM);
   void parse_func_decl(BOOLINT main);
   void parse_statement(int *VAR_LOC_NUM,int RET_ADDR);
   void parse_statement(BOOLINT index,int code_assign,int code_inc,int code_dec,int code_assign_idx,int code_inc_idx,int code_dec_idx);
   void parse_expression(BOOLINT comma=FALSE);
   void parse_expression(int push,int push_idx);
   void PARSERMSG(const char *msg,BOOLINT after=FALSE);
   };

#endif

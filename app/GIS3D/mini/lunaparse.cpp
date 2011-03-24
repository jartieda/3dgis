// (c) by Stefan Roettger

#include "miniio.h"

#include "lunaparse.h"

// default constructor
lunaparse::lunaparse()
   {
   addLUNAtokens();

   VAR_NUM=0;
   }

// destructor
lunaparse::~lunaparse() {}

void lunaparse::addLUNAtokens()
   {
   SCANNER.addtoken("include",LUNA_INCLUDE);
   SCANNER.addtoken("var",LUNA_VAR);
   SCANNER.addtoken("par",LUNA_PAR);
   SCANNER.addtoken("array",LUNA_ARRAY);
   SCANNER.addtoken("ref",LUNA_REF);
   SCANNER.addtoken("byte",LUNA_BYTE);
   SCANNER.addtoken("main",LUNA_MAIN);
   SCANNER.addtoken("func",LUNA_FUNC);
   SCANNER.addtoken("if",LUNA_IF);
   SCANNER.addtoken("else",LUNA_ELSE);
   SCANNER.addtoken("while",LUNA_WHILE);
   SCANNER.addtoken("repeat",LUNA_REPEAT);
   SCANNER.addtoken("until",LUNA_UNTIL);
   SCANNER.addtoken("for",LUNA_FOR);
   SCANNER.addtoken("return",LUNA_RETURN);
   SCANNER.addtoken("warn",LUNA_WARN);
   SCANNER.addtoken(";",LUNA_NULL);
   SCANNER.addtoken("(",LUNA_PARENLEFT);
   SCANNER.addtoken(")",LUNA_PARENRIGHT);
   SCANNER.addtoken("{",LUNA_BRACELEFT);
   SCANNER.addtoken("}",LUNA_BRACERIGHT);
   SCANNER.addtoken("[",LUNA_BRACKETLEFT);
   SCANNER.addtoken("]",LUNA_BRACKETRIGHT);
   SCANNER.addtoken(":=",LUNA_ASSIGN);
   SCANNER.addtoken(",",LUNA_COMMA);
   SCANNER.addtoken("++",LUNA_INC);
   SCANNER.addtoken("--",LUNA_DEC);
   SCANNER.addtoken("size",LUNA_SIZE);
   SCANNER.addtoken("true",LUNA_TRUE);
   SCANNER.addtoken("false",LUNA_FALSE);
   SCANNER.addtoken("+",LUNA_ADD);
   SCANNER.addtoken("-",LUNA_SUB);
   SCANNER.addtoken("*",LUNA_MUL);
   SCANNER.addtoken("/",LUNA_DIV);
   SCANNER.addtoken("%",LUNA_MOD);
   SCANNER.addtoken("=",LUNA_EQ);
   SCANNER.addtoken("<>",LUNA_NEQ);
   SCANNER.addtoken("<",LUNA_LT);
   SCANNER.addtoken(">",LUNA_GT);
   SCANNER.addtoken("<=",LUNA_LE);
   SCANNER.addtoken(">=",LUNA_GE);
   SCANNER.addtoken("&",LUNA_AND);
   SCANNER.addtoken("|",LUNA_OR);
   SCANNER.addtoken("!",LUNA_NOT);
   SCANNER.addtoken("min",LUNA_MIN);
   SCANNER.addtoken("max",LUNA_MAX);
   SCANNER.addtoken("abs",LUNA_ABS);
   SCANNER.addtoken("sqr",LUNA_SQR);
   SCANNER.addtoken("sqrt",LUNA_SQRT);
   SCANNER.addtoken("exp",LUNA_EXP);
   SCANNER.addtoken("log",LUNA_LOG);
   SCANNER.addtoken("pow",LUNA_POW);
   SCANNER.addtoken("sin",LUNA_SIN);
   SCANNER.addtoken("cos",LUNA_COS);
   SCANNER.addtoken("tan",LUNA_TAN);
   SCANNER.addtoken("atan",LUNA_ATAN);
   SCANNER.addtoken("atan2",LUNA_ATAN2);
   }

void lunaparse::parseLUNA()
   {
   int addr;

   MAIN=-1;

   addr=CODE.getaddr();
   CODE.addcode(lunacode::CODE_RESERVE_VAR,lunacode::MODE_INT,CODE.getaddr());

   while (SCANNER.gettoken()!=lunascan::LUNA_END)
      if (SCANNER.gettoken()==LUNA_INCLUDE) parse_include();
      else
         {
         if (SCANNER.gettoken()==LUNA_VAR) parse_var_decl(FALSE,FALSE,FALSE,FALSE,TRUE);
         else if (SCANNER.gettoken()==LUNA_ARRAY) parse_var_decl(FALSE,FALSE,TRUE,FALSE,FALSE);
         else if (SCANNER.gettoken()==LUNA_REF) parse_var_decl(FALSE,FALSE,FALSE,TRUE,FALSE);
         else if (SCANNER.gettoken()==LUNA_MAIN) parse_func_decl(TRUE);
         else if (SCANNER.gettoken()==LUNA_FUNC) parse_func_decl(FALSE);
         else
            {
            PARSERMSG("expected declaration");
            SCANNER.next();
            }

         if (SCANNER.gettoken()==LUNA_NULL) SCANNER.next();
         }

   CODE.addcodeat(addr,lunacode::CODE_RESERVE_VAR,lunacode::MODE_INT,VAR_NUM);

   if (MAIN!=-1) CODE.addcode(lunacode::CODE_JSR,lunacode::MODE_ANY,MAIN);

   SCANNER.freecode();
   }

void lunaparse::parse_include()
   {
   char *filename;

   char *code;
   unsigned int bytes;

   SCANNER.next();

   if (SCANNER.gettoken()!=lunascan::LUNA_STRING) PARSERMSG("expected string");

   filename=SCANNER.getstring();
   code=(char *)readfile(filename,&bytes);

   if (code==NULL)
      {
      PARSERMSG("file not found");
      SCANNER.next();
      }
   else
      {
      if ((code=(char *)realloc(code,bytes+1))==NULL) ERRORMSG();
      code[bytes]='\0';

      SCANNER.pushcode(code);
      free(code);
      }
   }

int lunaparse::parse_var_decl(BOOLINT loc,BOOLINT par,BOOLINT array,BOOLINT ref,BOOLINT stat,int *VAR_LOC_NUM)
   {
   BOOLINT byte;

   int var_num=0;
   int var_loc_num=0;

   int pop_num;

   SCANNER.next();

   byte=FALSE;

   if (array || ref)
      if (SCANNER.gettoken()==LUNA_BYTE)
         {
         byte=TRUE;
         SCANNER.next();
         }

   if (SCANNER.gettoken()!=lunascan::LUNA_UNKNOWN) PARSERMSG("identifier already defined");
   else if (SCANNER.check_special()) PARSERMSG("bad identifier");
   else
      if (!array && !ref)
         if (!loc && !par)
            {
            var_num=VAR_NUM++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_VAR_GLB,var_num);
            }
         else
            {
            var_loc_num=(*VAR_LOC_NUM)++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_VAR_LOC,var_loc_num);
            }
      else if (!ref)
         if (!loc)
            {
            var_num=VAR_NUM++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_ARRAY_GLB,var_num);
            }
         else
            {
            var_loc_num=(*VAR_LOC_NUM)++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_ARRAY_LOC,var_loc_num);
            }
      else
         if (!loc)
            {
            var_num=VAR_NUM++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_REF_GLB,var_num);
            }
         else
            {
            var_loc_num=(*VAR_LOC_NUM)++;
            SCANNER.addtoken(SCANNER.getstring(),LUNA_REF_LOC,var_loc_num);
            }

   SCANNER.next();

   pop_num=-1;

   if (!array && !ref)
      {
      if (!par)
         if (SCANNER.gettoken()==LUNA_EQ || SCANNER.gettoken()==LUNA_ASSIGN)
            {
            SCANNER.next();

            parse_expression();

            if (!loc)
               if (!stat)
                  CODE.addcode(lunacode::CODE_POP_VAR,lunacode::MODE_ANY,var_num);
               else
                  CODE.addcode(lunacode::CODE_POP_VAR_STAT,lunacode::MODE_ANY,var_num);
            else
               CODE.addcode(lunacode::CODE_POP_VAR_LOC,lunacode::MODE_ANY,var_loc_num);
            }

      if (par) pop_num=var_loc_num;
      }
   else if (!ref)
      if (SCANNER.gettoken()==LUNA_BRACKETLEFT)
         {
         SCANNER.next();

         parse_expression();

         if (!loc)
            if (!byte)
               CODE.addcode(lunacode::CODE_ALLOC_ARRAY,lunacode::MODE_ANY,var_num);
            else
               CODE.addcode(lunacode::CODE_ALLOC_ARRAY_BYT,lunacode::MODE_ANY,var_num);
         else
            if (!byte)
               CODE.addcode(lunacode::CODE_ALLOC_ARRAY_LOC,lunacode::MODE_ANY,var_loc_num);
            else
               CODE.addcode(lunacode::CODE_ALLOC_ARRAY_BYT_LOC,lunacode::MODE_ANY,var_loc_num);

         if (SCANNER.gettoken()!=LUNA_BRACKETRIGHT) PARSERMSG("expected matching bracket");
         SCANNER.next();
         }
      else
         if (!loc)
            if (!byte)
               CODE.addcode(lunacode::CODE_INIT_ARRAY,lunacode::MODE_ANY,var_num);
            else
               CODE.addcode(lunacode::CODE_INIT_ARRAY_BYT,lunacode::MODE_ANY,var_num);
         else
            if (!byte)
               CODE.addcode(lunacode::CODE_INIT_ARRAY_LOC,lunacode::MODE_ANY,var_loc_num);
            else
               CODE.addcode(lunacode::CODE_INIT_ARRAY_BYT_LOC,lunacode::MODE_ANY,var_loc_num);
   else
      {
      if (!par)
         if (!loc)
            if (!byte)
               CODE.addcode(lunacode::CODE_INIT_REF,lunacode::MODE_ANY,var_num);
            else
               CODE.addcode(lunacode::CODE_INIT_REF_BYT,lunacode::MODE_ANY,var_num);
         else
            if (!byte)
               CODE.addcode(lunacode::CODE_INIT_REF_LOC,lunacode::MODE_ANY,var_loc_num);
            else
               CODE.addcode(lunacode::CODE_INIT_REF_BYT_LOC,lunacode::MODE_ANY,var_loc_num);

      if (par)
         {
         if (!byte)
            CODE.addcode(lunacode::CODE_INIT_REF_LOC,lunacode::MODE_ANY,var_loc_num);
         else
            CODE.addcode(lunacode::CODE_INIT_REF_BYT_LOC,lunacode::MODE_ANY,var_loc_num);

         pop_num=var_loc_num;
         }
      }

   return(pop_num);
   }

void lunaparse::parse_par_decl(int *PAR_LOC_NUM)
   {
   int pop_var_loc_num;
   int pop_ref_loc_num;

   pop_var_loc_num=pop_ref_loc_num=-1;

   if (SCANNER.gettoken()==LUNA_PAR) pop_var_loc_num=parse_var_decl(TRUE,TRUE,FALSE,FALSE,FALSE,PAR_LOC_NUM);
   else if (SCANNER.gettoken()==LUNA_REF) pop_ref_loc_num=parse_var_decl(TRUE,TRUE,FALSE,TRUE,FALSE,PAR_LOC_NUM);
   else
      {
      PARSERMSG("expected parameter declaration");
      SCANNER.next();
      }

   if (SCANNER.gettoken()==LUNA_COMMA)
      {
      SCANNER.next();

      parse_par_decl(PAR_LOC_NUM);
      }

   if (pop_var_loc_num!=-1) CODE.addcode(lunacode::CODE_POP_VAR_LOC,lunacode::MODE_ANY,pop_var_loc_num);
   else if (pop_ref_loc_num!=-1) CODE.addcode(lunacode::CODE_POP_REF_LOC,lunacode::MODE_ANY,pop_ref_loc_num);
   }

void lunaparse::parse_func_decl(BOOLINT main)
   {
   int i;

   int VAR_LOC_NUM;

   int addr1,addr2,addr3;

   int serials;

   VAR_LOC_NUM=0;

   SCANNER.next();

   addr1=CODE.getaddr();
   CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

   addr2=CODE.getaddr();
   CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

   if (main)
      if (MAIN!=-1) PARSERMSG("main already defined");
      else MAIN=CODE.getaddr();
   else
      {
      if (SCANNER.gettoken()!=lunascan::LUNA_UNKNOWN) PARSERMSG("identifier already defined");
      else if (SCANNER.check_special()) PARSERMSG("bad identifier");
      else SCANNER.addtoken(SCANNER.getstring(),LUNA_FUNCTION,CODE.getaddr());

      SCANNER.next();
      }

   SCANNER.push();

   addr3=CODE.getaddr();
   CODE.addcode(lunacode::CODE_RESERVE_VAR_LOC,lunacode::MODE_INT,0);

   if (SCANNER.gettoken()!=LUNA_PARENLEFT) PARSERMSG("expected parameter list");
   SCANNER.next();

   if (SCANNER.gettoken()==LUNA_PARENRIGHT) SCANNER.next();
   else
      {
      parse_par_decl(&VAR_LOC_NUM);

      if (SCANNER.gettoken()!=LUNA_PARENRIGHT) PARSERMSG("expected matching paren");
      SCANNER.next();
      }

   if (SCANNER.gettoken()!=LUNA_BRACELEFT) PARSERMSG("expected function body");
   SCANNER.next();

   while (SCANNER.gettoken()!=LUNA_BRACERIGHT)
      {
      if (SCANNER.gettoken()==lunascan::LUNA_END)
         {
         PARSERMSG("brace mismatch",TRUE);
         break;
         }

      if (SCANNER.gettoken()==LUNA_VAR) parse_var_decl(TRUE,FALSE,FALSE,FALSE,FALSE,&VAR_LOC_NUM);
      else if (SCANNER.gettoken()==LUNA_ARRAY) parse_var_decl(TRUE,FALSE,TRUE,FALSE,FALSE,&VAR_LOC_NUM);
      else if (SCANNER.gettoken()==LUNA_REF) parse_var_decl(TRUE,FALSE,FALSE,TRUE,FALSE,&VAR_LOC_NUM);
      else if (SCANNER.gettoken()==LUNA_FUNC) parse_func_decl(FALSE);
      else parse_statement(&VAR_LOC_NUM,addr2);

      if (SCANNER.gettoken()==LUNA_NULL) SCANNER.next();
      }

   SCANNER.next();

   CODE.addcodeat(addr3,lunacode::CODE_RESERVE_VAR_LOC,lunacode::MODE_INT,VAR_LOC_NUM);

   CODE.addcodeat(addr2,lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

   serials=SCANNER.getserials();

   for (i=SCANNER.popserials(); i<serials; i++)
      if (SCANNER.gettoken(i)==LUNA_ARRAY_LOC)
         CODE.addcode(lunacode::CODE_FREE_ARRAY_LOC,lunacode::MODE_ANY,SCANNER.getinfo(i));

   CODE.addcode(lunacode::CODE_REMOVE_VAR_LOC,lunacode::MODE_INT,VAR_LOC_NUM);

   CODE.addcode(lunacode::CODE_RSR,lunacode::MODE_NONE);

   SCANNER.pop();

   CODE.addcodeat(addr1,lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());
   }

void lunaparse::parse_statement(int *VAR_LOC_NUM,int RET_ADDR)
   {
   int info;

   int addr1,addr2,addr3,addr4;

   if (SCANNER.gettoken()==LUNA_VAR_GLB)
      parse_statement(FALSE,
                      lunacode::CODE_POP_VAR,lunacode::CODE_INC_VAR,lunacode::CODE_DEC_VAR,
                      lunacode::CODE_NOP,lunacode::CODE_NOP,lunacode::CODE_NOP);
   else if (SCANNER.gettoken()==LUNA_VAR_LOC)
      parse_statement(FALSE,
                      lunacode::CODE_POP_VAR_LOC,lunacode::CODE_INC_VAR_LOC,lunacode::CODE_DEC_VAR_LOC,
                      lunacode::CODE_NOP,lunacode::CODE_NOP,lunacode::CODE_NOP);
   else if (SCANNER.gettoken()==LUNA_ARRAY_GLB)
      parse_statement(TRUE,
                      lunacode::CODE_NOP,lunacode::CODE_NOP,lunacode::CODE_NOP,
                      lunacode::CODE_POP_ARRAY_IDX,lunacode::CODE_INC_ARRAY_IDX,lunacode::CODE_DEC_ARRAY_IDX);
   else if (SCANNER.gettoken()==LUNA_ARRAY_LOC)
      parse_statement(TRUE,
                      lunacode::CODE_NOP,lunacode::CODE_NOP,lunacode::CODE_NOP,
                      lunacode::CODE_POP_ARRAY_LOC_IDX,lunacode::CODE_INC_ARRAY_LOC_IDX,lunacode::CODE_DEC_ARRAY_LOC_IDX);
   else if (SCANNER.gettoken()==LUNA_REF_GLB)
      parse_statement(TRUE,
                      lunacode::CODE_POP_REF,lunacode::CODE_NOP,lunacode::CODE_NOP,
                      lunacode::CODE_POP_REF_IDX,lunacode::CODE_INC_REF_IDX,lunacode::CODE_DEC_REF_IDX);
   else if (SCANNER.gettoken()==LUNA_REF_LOC)
      parse_statement(TRUE,
                      lunacode::CODE_POP_REF_LOC,lunacode::CODE_NOP,lunacode::CODE_NOP,
                      lunacode::CODE_POP_REF_LOC_IDX,lunacode::CODE_INC_REF_LOC_IDX,lunacode::CODE_DEC_REF_LOC_IDX);
   else if (SCANNER.gettoken()==LUNA_FUNCTION)
      {
      info=SCANNER.getinfo();

      SCANNER.next();

      if (SCANNER.gettoken()==LUNA_PARENLEFT)
         {
         parse_expression(TRUE);

         CODE.addcode(lunacode::CODE_JSR,lunacode::MODE_ANY,info);
         }
      else
         {
         PARSERMSG("expected parameter list");
         SCANNER.next();
         }
      }
   else if (SCANNER.gettoken()==LUNA_BRACELEFT)
      {
      SCANNER.next();

      SCANNER.push();

      while (SCANNER.gettoken()!=LUNA_BRACERIGHT)
         {
         if (SCANNER.gettoken()==lunascan::LUNA_END)
            {
            PARSERMSG("brace mismatch",TRUE);
            break;
            }

         if (SCANNER.gettoken()==LUNA_VAR) parse_var_decl(TRUE,FALSE,FALSE,FALSE,FALSE,VAR_LOC_NUM);
         else if (SCANNER.gettoken()==LUNA_REF) parse_var_decl(TRUE,FALSE,FALSE,TRUE,FALSE,VAR_LOC_NUM);
         else if (SCANNER.gettoken()==LUNA_FUNC) parse_func_decl(FALSE);
         else parse_statement(VAR_LOC_NUM,RET_ADDR);

         if (SCANNER.gettoken()==LUNA_NULL) SCANNER.next();
         }

      SCANNER.pop();

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_IF)
      {
      SCANNER.next();

      if (SCANNER.gettoken()==LUNA_PARENLEFT) parse_expression();
      else
         {
         PARSERMSG("expected expression");
         SCANNER.next();
         }

      addr1=CODE.getaddr();
      CODE.addcode(lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      if (SCANNER.gettoken()==LUNA_ELSE)
         {
         addr2=CODE.getaddr();
         CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

         CODE.addcodeat(addr1,lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());

         SCANNER.next();

         parse_statement(VAR_LOC_NUM,RET_ADDR);

         CODE.addcodeat(addr2,lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());
         }
      else
         CODE.addcodeat(addr1,lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());
      }
   else if (SCANNER.gettoken()==LUNA_WHILE)
      {
      SCANNER.next();

      addr1=CODE.getaddr();

      if (SCANNER.gettoken()==LUNA_PARENLEFT) parse_expression();
      else
         {
         PARSERMSG("expected expression");
         SCANNER.next();
         }

      addr2=CODE.getaddr();
      CODE.addcode(lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_ANY,addr1);

      CODE.addcodeat(addr2,lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());
      }
   else if (SCANNER.gettoken()==LUNA_REPEAT)
      {
      SCANNER.next();

      addr1=CODE.getaddr();

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      if (SCANNER.gettoken()!=LUNA_UNTIL) PARSERMSG("expected matching UNTIL");
      SCANNER.next();

      if (SCANNER.gettoken()==LUNA_PARENLEFT) parse_expression();
      else
         {
         PARSERMSG("expected expression");
         SCANNER.next();
         }

      CODE.addcode(lunacode::CODE_JIF,lunacode::MODE_ANY,addr1);
      }
   else if (SCANNER.gettoken()==LUNA_FOR)
      {
      SCANNER.next();

      if (SCANNER.gettoken()!=LUNA_PARENLEFT) PARSERMSG("expected parameter list");
      SCANNER.next();

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      if (SCANNER.gettoken()!=LUNA_COMMA) PARSERMSG("expected comma");
      SCANNER.next();

      addr1=CODE.getaddr();

      parse_expression();

      addr2=CODE.getaddr();
      CODE.addcode(lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());

      addr3=CODE.getaddr();
      CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

      if (SCANNER.gettoken()!=LUNA_COMMA) PARSERMSG("expected comma");
      SCANNER.next();

      addr4=CODE.getaddr();

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_ANY,addr1);

      if (SCANNER.gettoken()!=LUNA_PARENRIGHT) PARSERMSG("expected matching paren");
      SCANNER.next();

      CODE.addcodeat(addr3,lunacode::CODE_JMP,lunacode::MODE_INT,CODE.getaddr());

      parse_statement(VAR_LOC_NUM,RET_ADDR);

      CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_ANY,addr4);

      CODE.addcodeat(addr2,lunacode::CODE_JIF,lunacode::MODE_INT,CODE.getaddr());
      }
   else if (SCANNER.gettoken()==LUNA_RETURN)
      {
      SCANNER.next();
      if (SCANNER.gettoken()==LUNA_PARENLEFT) parse_expression();

      CODE.addcode(lunacode::CODE_JMP,lunacode::MODE_ANY,RET_ADDR);
      }
   else if (SCANNER.gettoken()==LUNA_WARN)
      {
      CODE.addcode(lunacode::CODE_WARN,lunacode::MODE_ANY,SCANNER.getline());

      SCANNER.next();
      }
   else
      {
      PARSERMSG("expected statement");
      SCANNER.next();
      }

   if (SCANNER.gettoken()==LUNA_NULL) SCANNER.next();
   }

void lunaparse::parse_statement(BOOLINT index,
                                int code_assign,int code_inc,int code_dec,
                                int code_assign_idx,int code_inc_idx,int code_dec_idx)
   {
   int info;

   info=SCANNER.getinfo();

   SCANNER.next();

   if (index)
      if (SCANNER.gettoken()==LUNA_BRACKETLEFT)
         {
         SCANNER.next();

         parse_expression();

         if (SCANNER.gettoken()!=LUNA_BRACKETRIGHT) PARSERMSG("expected matching bracket");
         SCANNER.next();
         }
      else index=FALSE;

   if (SCANNER.gettoken()==LUNA_EQ || SCANNER.gettoken()==LUNA_ASSIGN)
      {
      SCANNER.next();

      parse_expression();

      if (!index)
         if (code_assign==lunacode::CODE_NOP) PARSERMSG("invalid assignment");
         else CODE.addcode(code_assign,lunacode::MODE_ANY,info);
      else
         if (code_assign_idx==lunacode::CODE_NOP) PARSERMSG("invalid assignment");
         else CODE.addcode(code_assign_idx,lunacode::MODE_ANY,info);
      }
   else if (SCANNER.gettoken()==LUNA_INC)
      {
      if (!index)
         if (code_inc==lunacode::CODE_NOP) PARSERMSG("invalid operation");
         else CODE.addcode(code_inc,lunacode::MODE_ANY,info);
      else
         if (code_inc_idx==lunacode::CODE_NOP) PARSERMSG("invalid operation");
         else CODE.addcode(code_inc_idx,lunacode::MODE_ANY,info);

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_DEC)
      {
      if (!index)
         if (code_dec==lunacode::CODE_NOP) PARSERMSG("invalid operation");
         else CODE.addcode(code_dec,lunacode::MODE_ANY,info);
      else
         if (code_dec_idx==lunacode::CODE_NOP) PARSERMSG("invalid operation");
         else CODE.addcode(code_dec_idx,lunacode::MODE_ANY,info);

      SCANNER.next();
      }
   else
      {
      PARSERMSG("expected statement");
      SCANNER.next();
      }
   }

void lunaparse::parse_expression(BOOLINT comma)
   {
   int info;

   int op;
   int args;

   if (SCANNER.gettoken()==lunascan::LUNA_VALUE)
      {
      CODE.addcode(lunacode::CODE_PUSH,lunacode::MODE_FLOAT,0,SCANNER.getvalue());

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_VAR_GLB)
      {
      CODE.addcode(lunacode::CODE_PUSH_VAR,lunacode::MODE_ANY,SCANNER.getinfo());

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_VAR_LOC)
      {
      CODE.addcode(lunacode::CODE_PUSH_VAR_LOC,lunacode::MODE_ANY,SCANNER.getinfo());

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_ARRAY_GLB) parse_expression(lunacode::CODE_PUSH_ARRAY,lunacode::CODE_PUSH_ARRAY_IDX);
   else if (SCANNER.gettoken()==LUNA_ARRAY_LOC) parse_expression(lunacode::CODE_PUSH_ARRAY_LOC,lunacode::CODE_PUSH_ARRAY_LOC_IDX);
   else if (SCANNER.gettoken()==LUNA_REF_GLB) parse_expression(lunacode::CODE_PUSH_REF,lunacode::CODE_PUSH_REF_IDX);
   else if (SCANNER.gettoken()==LUNA_REF_LOC) parse_expression(lunacode::CODE_PUSH_REF_LOC,lunacode::CODE_PUSH_REF_LOC_IDX);
   else if (SCANNER.gettoken()==LUNA_FUNCTION)
      {
      info=SCANNER.getinfo();

      SCANNER.next();

      if (SCANNER.gettoken()==LUNA_PARENLEFT)
         {
         parse_expression(TRUE);

         CODE.addcode(lunacode::CODE_JSR,lunacode::MODE_ANY,info);
         }
      else
         {
         PARSERMSG("expected parameter list");
         SCANNER.next();
         }
      }
   else if (SCANNER.gettoken()==LUNA_PARENLEFT)
      {
      SCANNER.next();

      op=lunacode::CODE_NOP;

      switch (SCANNER.gettoken())
         {
         case LUNA_ADD: op=lunacode::CODE_ADD; break;
         case LUNA_SUB: op=lunacode::CODE_SUB; break;
         case LUNA_MUL: op=lunacode::CODE_MUL; break;
         case LUNA_DIV: op=lunacode::CODE_DIV; break;
         case LUNA_MOD: op=lunacode::CODE_MOD; break;
         case LUNA_EQ: op=lunacode::CODE_EQ; break;
         case LUNA_NEQ: op=lunacode::CODE_NEQ; break;
         case LUNA_LT: op=lunacode::CODE_LT; break;
         case LUNA_GT: op=lunacode::CODE_GT; break;
         case LUNA_LE: op=lunacode::CODE_LE; break;
         case LUNA_GE: op=lunacode::CODE_GE; break;
         case LUNA_AND: op=lunacode::CODE_AND; break;
         case LUNA_OR: op=lunacode::CODE_OR; break;
         case LUNA_NOT: op=lunacode::CODE_NOT; break;
         }

      if (op!=lunacode::CODE_NOP) SCANNER.next();

      args=0;

      while (SCANNER.gettoken()!=LUNA_PARENRIGHT)
         {
         if (comma && op==lunacode::CODE_NOP && args>0)
            if (SCANNER.gettoken()==LUNA_COMMA) SCANNER.next();

         if (SCANNER.gettoken()==lunascan::LUNA_END)
            {
            PARSERMSG("paren mismatch",TRUE);
            break;
            }

         if (op!=lunacode::CODE_NOP) parse_expression();
         else parse_expression(comma);

         args++;
         }

      if (op!=lunacode::CODE_NOP)
         CODE.addcode(op,lunacode::MODE_NONE);

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_SIZE)
      {
      SCANNER.next();

      if (SCANNER.gettoken()!=LUNA_PARENLEFT) PARSERMSG("expected parameter");
      SCANNER.next();

      if (SCANNER.gettoken()==LUNA_ARRAY_GLB)
         CODE.addcode(lunacode::CODE_SIZE_ARRAY,lunacode::MODE_ANY,SCANNER.getinfo());
      else if (SCANNER.gettoken()==LUNA_ARRAY_LOC)
         CODE.addcode(lunacode::CODE_SIZE_ARRAY_LOC,lunacode::MODE_ANY,SCANNER.getinfo());
      else if (SCANNER.gettoken()==LUNA_REF_GLB)
         CODE.addcode(lunacode::CODE_SIZE_REF,lunacode::MODE_ANY,SCANNER.getinfo());
      else if (SCANNER.gettoken()==LUNA_REF_LOC)
         CODE.addcode(lunacode::CODE_SIZE_REF_LOC,lunacode::MODE_ANY,SCANNER.getinfo());
      else PARSERMSG("expected array");

      SCANNER.next();

      if (SCANNER.gettoken()!=LUNA_PARENRIGHT) PARSERMSG("expected matching paren");
      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_TRUE)
      {
      CODE.addcode(lunacode::CODE_PUSH,lunacode::MODE_BYTE,1);

      SCANNER.next();
      }
   else if (SCANNER.gettoken()==LUNA_FALSE)
      {
      CODE.addcode(lunacode::CODE_PUSH,lunacode::MODE_BYTE,0);

      SCANNER.next();
      }
   else
      {
      op=lunacode::CODE_NOP;

      switch (SCANNER.gettoken())
         {
         case LUNA_MIN: op=lunacode::CODE_MIN; break;
         case LUNA_MAX: op=lunacode::CODE_MAX; break;
         case LUNA_ABS: op=lunacode::CODE_ABS; break;
         case LUNA_SQR: op=lunacode::CODE_SQR; break;
         case LUNA_SQRT: op=lunacode::CODE_SQRT; break;
         case LUNA_EXP: op=lunacode::CODE_EXP; break;
         case LUNA_LOG: op=lunacode::CODE_LOG; break;
         case LUNA_POW: op=lunacode::CODE_POW; break;
         case LUNA_SIN: op=lunacode::CODE_SIN; break;
         case LUNA_COS: op=lunacode::CODE_COS; break;
         case LUNA_TAN: op=lunacode::CODE_TAN; break;
         case LUNA_ATAN: op=lunacode::CODE_ATAN; break;
         case LUNA_ATAN2: op=lunacode::CODE_ATAN2; break;
         }

      if (op!=lunacode::CODE_NOP)
         {
         SCANNER.next();

         if (SCANNER.gettoken()==LUNA_PARENLEFT)
            {
            parse_expression(TRUE);

            CODE.addcode(op,lunacode::MODE_NONE);
            }
         else
            {
            PARSERMSG("expected parameter list");
            SCANNER.next();
            }
         }
      else
         {
         PARSERMSG("expected expression");
         SCANNER.next();
         }
      }
   }

void lunaparse::parse_expression(int push,int push_idx)
   {
   int info;

   info=SCANNER.getinfo();

   SCANNER.next();

   if (SCANNER.gettoken()==LUNA_BRACKETLEFT)
      {
      SCANNER.next();

      parse_expression();

      CODE.addcode(push_idx,lunacode::MODE_ANY,info);

      if (SCANNER.gettoken()!=LUNA_BRACKETRIGHT) PARSERMSG("expected matching bracket");
      SCANNER.next();
      }
   else
      CODE.addcode(push,lunacode::MODE_ANY,info);
   }

void lunaparse::print()
   {
   if (SCANNER.getcode()==NULL) return;

   printf("%s",SCANNER.getcode());
   }

void lunaparse::printtokens()
   {
   while (SCANNER.gettoken()!=lunascan::LUNA_END)
      if (SCANNER.gettoken()==LUNA_INCLUDE) parse_include();
      else
         {
         printtoken();
         SCANNER.next();
         }

   SCANNER.freecode();
   }

void lunaparse::printtoken()
   {
   switch (SCANNER.gettoken())
      {
      case lunascan::LUNA_UNKNOWN: printf("identifier: %s\n",SCANNER.getstring()); break;
      case lunascan::LUNA_VALUE: printf("value: %f\n",SCANNER.getvalue()); break;
      case lunascan::LUNA_STRING: printf("string: %s\n",SCANNER.getstring()); break;
      default: printf("token: %d #%d\n",SCANNER.gettoken(),SCANNER.getserial()); break;
      }
   }

void lunaparse::PARSERMSG(const char *msg,BOOLINT after)
   {
   if (SCANNER.getline()>0)
      if (after && SCANNER.popline()>0) fprintf(stderr,"parser error in column %d of line %d after line %d: %s\n",SCANNER.getcol(),SCANNER.getline(),SCANNER.popline(),msg);
      else fprintf(stderr,"parser error in column %d of line %d: %s\n",SCANNER.getcol(),SCANNER.getline(),msg);
   else fprintf(stderr,"parser error: %s\n",msg);
   }

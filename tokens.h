#ifndef tokens_h
#define tokens_h
/* tokens.h -- List of labelled tokens and stuff
 *
 * Generated from: cl.g
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * ANTLR Version 1.33MR33
 */
#define zzEOF_TOKEN 1
#define INPUTEND 1
#define OPENANGL 2
#define CLOSEANGL 3
#define PROGRAM 4
#define ENDPROGRAM 5
#define PROCEDURE 6
#define ENDPROCEDURE 7
#define FUNCTION 8
#define ENDFUNCTION 9
#define RETURN 10
#define VARS 11
#define ENDVARS 12
#define VAL 13
#define REF 14
#define INT 15
#define STRUCT 16
#define ENDSTRUCT 17
#define BOOL 18
#define CIERTO 19
#define FALSO 20
#define ARRAY 21
#define OF 22
#define WRITELN 23
#define WRITE 24
#define READ 25
#define IF 26
#define THEN 27
#define ELSE 28
#define ENDIF 29
#define WHILE 30
#define DO 31
#define ENDWHILE 32
#define GT 33
#define EQ 34
#define LT 35
#define AND 36
#define OR 37
#define NOT 38
#define DIV 39
#define MINUS 40
#define TIMES 41
#define PLUS 42
#define OPENPAR 43
#define CLOSEPAR 44
#define OPENKEY 45
#define CLOSEKEY 46
#define ASIG 47
#define DOT 48
#define COMMA 49
#define IDENT 50
#define INTCONST 51
#define STRING 52
#define COMMENT 53
#define WHITESPACE 54
#define NEWLINE 55
#define LEXICALERROR 56

#ifdef __USE_PROTOS
void program(AST**_root);
#else
extern void program();
#endif

#ifdef __USE_PROTOS
void dec_vars(AST**_root);
#else
extern void dec_vars();
#endif

#ifdef __USE_PROTOS
void l_dec_vars(AST**_root);
#else
extern void l_dec_vars();
#endif

#ifdef __USE_PROTOS
void dec_var(AST**_root);
#else
extern void dec_var();
#endif

#ifdef __USE_PROTOS
void l_dec_blocs(AST**_root);
#else
extern void l_dec_blocs();
#endif

#ifdef __USE_PROTOS
void dec_bloc(AST**_root);
#else
extern void dec_bloc();
#endif

#ifdef __USE_PROTOS
void func_code(AST**_root);
#else
extern void func_code();
#endif

#ifdef __USE_PROTOS
void proc_code(AST**_root);
#else
extern void proc_code();
#endif

#ifdef __USE_PROTOS
void l_param(AST**_root);
#else
extern void l_param();
#endif

#ifdef __USE_PROTOS
void dec_param(AST**_root);
#else
extern void dec_param();
#endif

#ifdef __USE_PROTOS
void constr_type(AST**_root);
#else
extern void constr_type();
#endif

#ifdef __USE_PROTOS
void field(AST**_root);
#else
extern void field();
#endif

#ifdef __USE_PROTOS
void l_instrs(AST**_root);
#else
extern void l_instrs();
#endif

#ifdef __USE_PROTOS
void l_values(AST**_root);
#else
extern void l_values();
#endif

#ifdef __USE_PROTOS
void instruction(AST**_root);
#else
extern void instruction();
#endif

#ifdef __USE_PROTOS
void ifthenelse(AST**_root);
#else
extern void ifthenelse();
#endif

#ifdef __USE_PROTOS
void whiledo(AST**_root);
#else
extern void whiledo();
#endif

#ifdef __USE_PROTOS
void expression(AST**_root);
#else
extern void expression();
#endif

#ifdef __USE_PROTOS
void expression1(AST**_root);
#else
extern void expression1();
#endif

#ifdef __USE_PROTOS
void expression2(AST**_root);
#else
extern void expression2();
#endif

#ifdef __USE_PROTOS
void expression3(AST**_root);
#else
extern void expression3();
#endif

#ifdef __USE_PROTOS
void expression4(AST**_root);
#else
extern void expression4();
#endif

#ifdef __USE_PROTOS
void expression5(AST**_root);
#else
extern void expression5();
#endif

#ifdef __USE_PROTOS
void expsimple(AST**_root);
#else
extern void expsimple();
#endif

#endif
extern SetWordType zzerr1[];
extern SetWordType zzerr2[];
extern SetWordType setwd1[];
extern SetWordType zzerr3[];
extern SetWordType zzerr4[];
extern SetWordType zzerr5[];
extern SetWordType setwd2[];
extern SetWordType zzerr6[];
extern SetWordType zzerr7[];
extern SetWordType zzerr8[];
extern SetWordType zzerr9[];
extern SetWordType zzerr10[];
extern SetWordType setwd3[];
extern SetWordType zzerr11[];
extern SetWordType zzerr12[];
extern SetWordType setwd4[];
extern SetWordType zzerr13[];
extern SetWordType zzerr14[];
extern SetWordType zzerr15[];
extern SetWordType setwd5[];
extern SetWordType setwd6[];

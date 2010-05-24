#header
<<
#include <string>
#include <iostream>

using namespace std;
typedef struct 
{
  string kind; //quin tipus de node es
  string text; //valor del token
} Attrib;
void zzcr_attr(Attrib *attr, int type, char *text);
//aquesta funcio li dona a un token, els atributs. A cada cop que crea un node del arbre, li dona la representacio del token (algo aixi). 

#define AST_FIELDS string kind; string text;
#define zzcr_ast(as, attr, ttype, textt) as = new AST;\
(as)->kind = (attr)->kind; (as)->text = (attr)->text;\
(as)->right = NULL; (as)->down = NULL;
>>
<<
#include <cstdlib>
#include <cmath>
#include <map>
map<string, int> m;

void zzcr_attr(Attrib *attr, int type, char *text) 
{
    attr->kind = text;
    attr->text = "";
    if (type == NUM) 
    {
      attr->kind = "intconst";
      attr->text = text;
    }
    if (type == ID) 
    {
      attr->kind = "id";
      attr->text = text;
    }
}
void ASTPrintIndent(AST *a, string s) 
{
    if (a == NULL) return;
    cout << s << " " << a->kind;
    if (a->text != "") cout << "(" << a->text << ")";
    cout << endl;
    ASTPrintIndent(a->down, s + " |");
    ASTPrintIndent(a->right, s);
}
void ASTPrint(AST *a) 
{
    cout << endl;
    ASTPrintIndent(a, "");
}


int evaluate(AST *a) 
{
    if (a == NULL) return 0;
    if (a->kind == "id")  return m[a->text];
    if (a->kind == "intconst") return atoi(a->text.c_str());
    if (a->kind == "+") return evaluate(a->down) + evaluate(a->down->right);
    if (a->kind == "*") return evaluate(a->down) * evaluate(a->down->right);
    if (a->kind == "/") return evaluate(a->down) / evaluate(a->down->right);
    if (a->kind == "-") return evaluate(a->down) - evaluate(a->down->right);
    if (a->kind == "==") return (evaluate(a->down) == evaluate(a->down->right));
    if (a->kind == ">") return (evaluate(a->down) > evaluate(a->down->right));
    if (a->kind == "<") return (evaluate(a->down) < evaluate(a->down->right));
    if (a->kind == ">=") return (evaluate(a->down) >= evaluate(a->down->right));
    if (a->kind == "<=") return (evaluate(a->down) <= evaluate(a->down->right));
    if (a->kind == "!=") return (evaluate(a->down) != evaluate(a->down->right));
}


void execute(AST *a) 
{
    AST *act = NULL;
    AST *first = NULL;
    if (a == NULL) return;
    if (a->kind == ":=") 
    {
	m[a->down->text] = evaluate(a->down->right);
    }
    if (a->kind == "write")
    {
	cout << evaluate(a->down) << endl;
    }
    if (a->kind == "if") 
    {
	act = a->down;
	if (evaluate(act)) 
	{
	    act = act->right;
	    execute(act);
	}
    }
    if (a->kind == "while")
    {
	act = a->down;
	first = act;
	while (evaluate(a->down)) 
	{
	    act = act->right;
	    execute(act);
	    act = a->down;
	}
    }
    execute(a->right);
}

int main() {
    AST *root = NULL;
    ANTLR(program(&root), stdin);
    ASTPrint(root);
    execute(root);
}


/*
x:=3+5
write x
y:=3+x+5
if y<17 then if x>3 then write y endif endif
*/

/*
x:=3+5
if x>5 then write x endif

*/


/*
i:=10
r:=1
while i>0 do r:=r*i  i:=i-1 endwhile
write r



*/

>>
#lexclass START
#token NUM "[0-9]+"
#token PLUS "\+"
#token MINUS "\-"
#token TIMES "\*"
#token WRITE "write"
#token ID "[a-zA-Z]"
#token ASIG ":="
#token IF "if"
#token THEN "then"
#token ENDIF "endif"
#token GT ">"
#token EQ "=="
#token LT "<"
#token GE ">="
#token LE "<="
#token DIF "!="
#token WHILE "while"
#token DO "do"
#token ENDWHILE "endwhile"
#token SPACE "[\ \n]" << zzskip();>>



var: (NUM | ID) ;
expr: term (MINUS^ term | PLUS^ term)* ;
term: var (TIMES^ var)* ;
instruction: ID ASIG^ expr | WRITE^ expr ;
cond: var (EQ^ var | GT^ var | LT^ var | GE^ var | LE^ var | DIF^ var) ;
whiledo: WHILE^ cond DO! (instruction | ifthen | whiledo)* ENDWHILE ;
ifthen: IF^ cond THEN! (instruction | ifthen | whiledo)* ENDIF ;
program: (instruction | ifthen | whiledo)* ;



//amb el gorrito defineixo qui es el pare, amb el ! defineixo si aquest token apareix o no apareix a l'arbre.

/* ===== semantic.cc ===== */
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <vector>



using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include "ptype.hh"
#include "symtab.hh"

#include "myASTnode.hh"
#include "util.hh"
#include "semantic.hh"

string itostring(int x);
int compute_size(ptype tp);


// feedback the main program with our error status
int TypeError = 0;


/// ---------- Error reporting routines --------------

void errornumparam(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": The number of parameters in the call do not match."<<endl;
}

void errorincompatibleparam(int l,int n) {
	TypeError = 1;
	cout<<"L. "<<l<<": Parameter "<<n<<" with incompatible types."<<endl;
}

void errorreferenceableparam(int l,int n) {
	TypeError = 1;
	cout<<"L. "<<l<<": Parameter "<<n<<" is expected to be referenceable but it is not."<<endl;
}

void errordeclaredident(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Identifier "<<s<<" already declared."<<endl;
}

void errornondeclaredident(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Identifier "<<s<<" is undeclared."<<endl;
}

void errornonreferenceableleft(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Left expression of assignment is not referenceable."<<endl;
}

void errorincompatibleassignment(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": Assignment with incompatible types."<<endl;
}

void errorbooleanrequired(int l,string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Instruction "<<s<<" requires a boolean condition."<<endl;
}

void errorisnotprocedure(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": Operator ( must be applied to a procedure in an instruction."<<endl;
}

void errorisnotfunction(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": Operator ( must be applied to a function in an expression."<<endl;
}

void errorincompatibleoperator(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Operator "<<s<<" with incompatible types."<<endl;
}

void errorincompatiblereturn(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": Return with incompatible type."<<endl;
}

void errorreadwriterequirebasic(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Basic type required in "<<s<<"."<<endl;
}

void errornonreferenceableexpression(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Referenceable expression required in "<<s<<"."<<endl;
}

void errornonfielddefined(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Field "<<s<<" is not defined in the struct."<<endl;
}

void errorfielddefined(int l, string s) {
	TypeError = 1;
	cout<<"L. "<<l<<": Field "<<s<<" already defined in the struct."<<endl;
}


void errornumberitemsunpack(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": The number of items in the unpacking assignment do not match."<<endl;
}

void errornonreferenceableleftunpack(int l, int n) {
	TypeError = 1;
	cout<<"L. "<<l<<": Left expression of item " << n << " in unpacking assignment is not referenceable."<<endl;
}

void errorincompatibleassignmentunpack(int l, int n) {
	TypeError = 1;
	cout<<"L. "<<l<<": Unpacking assignment with incompatible types for item " << n << "." << endl;
}

void errorunpackrequiresstruct(int l) {
	TypeError = 1;
	cout<<"L. "<<l<<": Unpacking assignment requires a struct." << endl;
}


/// ------------------------------------------------------------
/// Table to store information about program identifiers
symtab symboltable;

static void InsertintoST(int line,string kind,string id,ptype tp)
{
	infosym p;

	if (symboltable.find(id) && symboltable.jumped_scopes(id)==0) errordeclaredident(line,id);
	else {
		symboltable.createsymbol(id);
		symboltable[id].kind=kind;
		symboltable[id].tp=tp;
	}
}

/// ------------------------------------------------------------

bool isbasickind(string kind) {
	return kind=="int" || kind=="bool" || kind=="string";
}



void check_params(AST *a, ptype tp, int line, int numparam)
{
	if (a) 
	{
	TypeCheck(a->right);
	AST *aux = child(a->right,0);
	vector<int> lines;
	vector<int> numparams;
	vector<string> errorkind;
	ptype realParams = symboltable[a->text].tp->down;
	while (aux && realParams)
	{
		//Aquests dos IFs van guardant els errors que trobem durant l'analisi dels parametres en 3 vectors.
		//Un cop fora del bucle, i comprovat el nombre de params, els mostrem, si el nombre de parametres es correcte.
		if ((!aux->ref)&&(realParams->kind=="parref")) 
		{
			lines.push_back(line);
			numparams.push_back(numparam);
			errorkind.push_back("ref_error");
		}
		if ((aux->tp->kind!="error")&&(!equivalent_types(aux->tp, realParams->down))) 
		{
			lines.push_back(line);
			numparams.push_back(numparam);
			errorkind.push_back("incompatible_error");
		}

		aux = aux->right;
		realParams = realParams->right;
		numparam++;
	}
	if ((!aux && realParams) || (aux && !realParams))
	{
		errornumparam(line);
	}
	else 
	{	//Si el nombre de parametres coincideix, escrivim els errors que hem trobat de referencia i incompatibilitat.
		for (int i=0; i<lines.size(); i++)
		{
			if (errorkind[i]=="ref_error")			errorreferenceableparam(lines[i], numparams[i]);
			if (errorkind[i]=="incompatible_error") errorincompatibleparam(lines[i], numparams[i]);
		}
	}
	}
}


void insert_vars(AST *a)
{
	if (!a) return;
	TypeCheck(child(a,0));
	InsertintoST(a->line,"idvarlocal",a->text,child(a,0)->tp);
	insert_vars(a->right); 
}

void construct_struct(AST *a)
{
	AST *a1=child(a,0);
	a->tp=create_type("struct",0,0);

	while (a1!=0) {
		TypeCheck(child(a1,0));
		if (a->tp->struct_field.find(a1->text)==a->tp->struct_field.end()) {
			a->tp->struct_field[a1->text]=child(a1,0)->tp;
			a->tp->ids.push_back(a1->text);
		} else {
			errorfielddefined(a1->line,a1->text);
		}
		a1=a1->right;
	}
}

void insert_params(AST *a)
{
	if (!a) return;
	TypeCheck(child(a,1));
	InsertintoST(a->line, "idpar"+a->kind, child(a,0)->text, child(a,1)->tp);
	insert_params(a->right);

}

void create_header(AST *a)
{
	if (a->kind == "procedure")		a->tp = create_type("procedure",0,0);
	else if (a->kind == "function") a->tp = create_type("function",0,0);

	ptype type = a->tp;
	AST *param = child(child(child(a,0),0),0);		// param = punter al primer parametre de la llista
	ptype aux = create_type("aux", 0, 0);
	ptype punt = aux;
	while (param != 0)
	{
		aux->right = create_type("par"+param->kind,0,0);

		TypeCheck(child(param,1));			
		aux->right->down = child(param, 1)->tp;		//Posem a sota del Ref o Val, el tipus
		
		param = param->right;				
		aux = aux->right;
	}
	
	a->tp->down = punt->right;				//Ja hem vist tots els parametres, guardem l'arbret com a fill
	
	if (a->kind == "function")
	{
		TypeCheck(child(child(a, 0), 1));
		a->tp->right = child(child(a, 0), 1)->tp;	//Guardem el tipus de retorn al fill dret
	}
}


void insert_header(AST *a)
{
	string st_id;
	create_header(a);
	if (a->kind == "procedure") st_id = "idproc";
	else st_id = "idfunc";
	InsertintoST(child(a,0)->line, st_id, child(a,0)->text, a->tp);
}

void insert_headers(AST *a)
{
	while (a!=0) {
		a->tp = create_type(a->kind, 0, 0);
		insert_header(a);
		a=a->right;
	}
}


void TypeCheck(AST *a,string info)
{
	if (!a) {
		return;
	}

//cout<<"Starting with node \""<<a->kind<<"\""<<endl;

/**__ Program __**/
	if (a->kind=="program") 
	{
		a->sc=symboltable.push();		//creacio de l'Scope
		insert_vars(child(child(a,0),0)); 	//inserció de les variables
		insert_headers(child(child(a,1),0)); //insercio de headers del fill 0 del fill 1, que recursivament anira visitant els germans
		//symboltable.write(); //para que escriba las tablas de simbolos
		TypeCheck(child(a,1));
		TypeCheck(child(a,2),"instruction");
		
		symboltable.pop();
	} 

/**__ List __**/
	else if (a->kind=="list") 
	{
		// At this point only instruction, procedures or parameters lists are possible.
		for (AST *a1=a->down;a1!=0;a1=a1->right) 
		{
			TypeCheck(a1,info);
		}
	} 

/**__ Procedure & Function __**/
	else if (a->kind == "procedure" || a->kind == "function")
	{
		a->sc=symboltable.push();
		insert_params(child(child(child(a,0),0),0));
		insert_vars(child(child(a,1),0));
		insert_headers(child(child(a,2),0));
		//symboltable.write();
		TypeCheck(child(a,2));
		TypeCheck(child(a,3), "instruction");

		if (a->kind == "function")
		{
			TypeCheck(child(a,4), "expression");
			
			/*Comprovem que el tipus de retorn de la funcio coincideix amb el que efectivament retornem al final*/
			if (!equivalent_types(child(a,4)->tp, child(child(a,0),1)->tp)) 
			{
				errorincompatiblereturn(child(a,4)->line);
			}
		}
		symboltable.pop();
	}

/**__ Ident __**/
	else if (a->kind=="ident") 
	{
		if (!symboltable.find(a->text)) 
		{
			errornondeclaredident(a->line, a->text);
		} 
		else 
		{
			a->tp=symboltable[a->text].tp;
			if (a->tp->kind != "function" && a->tp->kind != "procedure") a->ref=1;
		}
	} 

/**__ Struct __**/
	else if (a->kind=="struct") 
	{
		construct_struct(a);
	}
	
	
/** OPENANGL **/
	else if (a->kind == "<<")
	{
		AST *aux = child(child(a,0),0);
		int i = 1;
		a->tp = create_type("struct", 0, 0);
		while(aux!=0)
		{
			TypeCheck(aux);
			
			string s = "e" + itostring(i);
			a->tp->struct_field[s]=aux->tp;
			a->tp->ids.push_back(s);
			
			aux=aux->right;
			i++;
		}
		compute_size(a->tp);
		
	}

/**__ Assignacio __**/
	else if (a->kind==":=") 
	{
		TypeCheck(child(a,0), "expression");
		TypeCheck(child(a,1), "expression");
		if (child(a,0)->kind=="<<")
		{
			AST *aux = child(child(child(a,0),0),0);
			int count = 0;
			while (aux)
			{
				count++;
				aux = aux->right;
			}
			if (child(a,1)->tp->kind!="struct")
			{
				errorunpackrequiresstruct(a->line);
			}
			
			if (child(a,1)->tp->ids.size() != count)
			{
				errornumberitemsunpack(a->line);
			}
			else
			{
				aux = child(child(child(a,0),0),0);
				ptype stp = child(a,1)->tp;
				list<string>::iterator it = stp->ids.begin();
				int i = 1;
				
				while (it != stp->ids.end())
				{
					ptype etp = stp->struct_field[*it];
					
					if (etp->kind!="error" && aux->tp->kind!="error" && (etp->kind!=aux->tp->kind))
					{
						errorincompatibleassignmentunpack(a->line, i);
					}
					if (!aux->ref)
					{
						errornonreferenceableleftunpack(a->line, i);
					}
					
					
					aux = aux->right;
					it++;
					i++;
				}
			}
	
			
		}
		else
		{
			
			if (!child(a,0)->ref) 
			{
				errornonreferenceableleft(a->line,child(a,0)->text);
			}
			else if (child(a,0)->tp->kind!="error" && child(a,1)->tp->kind!="error" &&
					 !equivalent_types(child(a,0)->tp,child(a,1)->tp)) 
			{
				errorincompatibleassignment(a->line);
			} 
			else 
			{
				a->tp=child(a,0)->tp;
			}
		}
		
	} 

/**__ Intconst __**/
	else if (a->kind=="intconst") 
	{
		a->tp=create_type("int",0,0);
	} 
	
	
/**__ Bool __**/
	else if (a->kind=="true" || a->kind=="false")
	{
		a->tp=create_type("bool", 0, 0);
	}

/**__ String __**/
	else if (a->kind=="string")
	{
		a->tp=create_type("string", 0, 0);
	}
	
	
/**__ Operacio Artimetica + - * / __**/
	else if (a->kind=="+" || (a->kind=="-" && child(a,1)!=0) || a->kind=="*" || a->kind=="/") 
	{
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") ||
		(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int")) 
		{
			errorincompatibleoperator(a->line,a->kind);
		}
		a->tp=create_type("int",0,0);
	}

/**__ Operacio Logica and or __**/
	else if (a->kind=="and" || a->kind=="or")
	{
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="bool") ||
		(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="bool")) 
		{
			errorincompatibleoperator(a->line,a->kind);
		}
		a->tp=create_type("bool",0,0);
	}  

/**__ Not __**/
	else if (a->kind=="not")
	{
		TypeCheck(child(a, 0));
		if ((child(a,0)->tp->kind!="bool"))
		{
			errorincompatibleoperator(a->line,a->kind);
		}
		a->tp=create_type("bool",0,0); 
	}  

/**__ Comparacio __**/
	else if (a->kind=="<" || a->kind==">")
	{
		TypeCheck(child(a,0));
		TypeCheck(child(a,1));
		if ((child(a,0)->tp->kind!="error" && child(a,0)->tp->kind!="int") || 
			(child(a,1)->tp->kind!="error" && child(a,1)->tp->kind!="int"))
		{
			errorincompatibleoperator(a->line,a->kind);
		}
		a->tp=create_type("bool",0,0); 

	}

/**__ Negacio __**/
	else if (a->kind=="-")
	{
		TypeCheck(child(a,0));
		if ((child(a,0)->tp->kind!="int"))
		{
			errorincompatibleoperator(a->line,a->kind);
		}
		a->tp=create_type("int",0,0); 
	}


/**__ If __**/
	else if (a->kind=="if")
	{
		TypeCheck(child(a, 0), "expression");
		if ((child(a,0)->tp->kind!="bool")){
			errorbooleanrequired(a->line,a->kind);
		}		
		TypeCheck(child(a, 1), "instruction");
		TypeCheck(child(a, 2), "instruction");
	}

/**__ While __**/
	else if (a->kind=="while")
	{
		TypeCheck(child(a, 0), "expression");
		if ((child(a,0)->tp->kind!="bool"))
		{
			errorbooleanrequired(a->line,a->kind);
		}
		TypeCheck(child(a, 1), "instruction");
	}


/**__ Igualtat __**/
	else if (a->kind=="=")
	{
		TypeCheck(child(a, 0));
		TypeCheck(child(a, 1));
		if ((child(a, 0)->tp->kind != "error" || (child(a, 1)->tp->kind != "bool" && child(a, 1)->tp->kind != "int"))
			&& (child(a, 1)->tp->kind != "error" || (child(a, 0)->tp->kind != "bool" && child(a, 0)->tp->kind != "int")) 
			&& (child(a, 0)->tp->kind != "int" || child(a, 1)->tp->kind != "int") 
			&& (child(a, 0)->tp->kind != "bool" || child(a, 1)->tp->kind != "bool"))
		{
			errorincompatibleoperator(a->line,a->kind); 
		}
		a->tp=create_type("bool",0,0); 
	} 

/**__ Array (declaracio) __**/
	else if (a->kind=="array")
	{
		TypeCheck(child(a, 0));
		TypeCheck(child(a, 1));
		a->tp=create_type("array", child(a, 1)->tp, 0);
		a->tp->numelemsarray= stringtoint(child(a,0)->text);
	}

/**__ Array (acces) __**/
	else if (a->kind=="[")
	{
		TypeCheck(child(a, 0));
		TypeCheck(child(a, 1));
		if (child(a,0)->tp->kind!="error" && child(a, 0)->tp->kind!="array")
		{
			errorincompatibleoperator(a->line, "array[]");
		}  
		else
		{
			if (child(a,0)->tp->kind!="error") a->tp = child(a,0)->tp->down;
			else a->tp = create_type("error", 0, 0);
		}

		if (child(a,1)->tp->kind!="error" && child(a, 1)->tp->kind!="int")
		{
			errorincompatibleoperator(a->line, "[]");
		}
		a->ref = child(a, 0)->ref;

	}  

/**__ Parentesi __**/
	else if (a->kind=="(")
	{
		TypeCheck(child(a,0));		//checkeamos el id
		child(a,0)->ref = 0;
		if (info == "instruction")
		{
			/*Hauria de ser un procediment pero no ho es, donem error*/
			if (child(a,0)->tp->kind!= "error" && child(a,0)->tp->kind != "procedure")
			{
				errorisnotprocedure(a->line);
				a->tp = create_type("error", 0, 0);
			}
			/*Tot i que es tracti d'una function quan hauria de ser procedure, en comprovem els parametres*/
			if ((child(a,0)->tp->kind == "function")||(child(a,0)->tp->kind == "procedure"))
			{
				check_params(child(a,0), 0, a->line, 1);
			}
		}
		else	//info = "expression"
		{
			/*Hauria de ser una funcio i no ho es, donem error*/
			if (child(a,0)->tp->kind != "error" && child(a,0)->tp->kind != "function")
			{
				errorisnotfunction(a->line);
				a->tp = create_type("error", 0,0);
			}
			/*Tot i que es tracti d'un procedure quan hauria de ser function en comprovem els parametres*/
			if (child(a,0)->tp->kind == "procedure") check_params(child(a,0), 0, a->line, 1);

			/*Si es una funcio hem d'assignar al tp del node "(" el tipus de retorn de la funcio*/
			if (child(a,0)->tp->kind == "function")
			{
				check_params(child(a,0), 0, a->line, 1);
				a->tp = symboltable[child(a,0)->text].tp->right;	//assignem el tipus de retorn de la funcio al parentesi
			}
		}	
	}
	
	
/**__ Tipus Basic __**/
	else if (isbasickind(a->kind)) 
	{
		a->tp=create_type(a->kind,0,0);
	}
	
/**__ Writeln & Write__**/
	else if ((a->kind=="writeln")||(a->kind=="write")) 
	{
		TypeCheck(child(a,0));
		if (child(a,0)->tp->kind!="error" && !isbasickind(child(a,0)->tp->kind)) {
			errorreadwriterequirebasic(a->line,a->kind);
		}
	}
	
/**__ Read __**/
	else if (a->kind == "read")
	{
		TypeCheck(child(a,0));
		if (!child(a,0)->ref)
		{
			errornonreferenceableexpression(a->line, a->kind);
		}
		else if (child(a, 0)->tp->kind != "error" && !isbasickind(child(a,0)->tp->kind))
		{
			errorreadwriterequirebasic(a->line, a->kind);
		}
	}
	

/**__ Acces a Struct __**/
	else if (a->kind==".") 
	{
		TypeCheck(child(a,0));
		a->ref=child(a,0)->ref;
		if (child(a,0)->tp->kind!="error") 
		{
			if (child(a,0)->tp->kind!="struct") 
			{
				errorincompatibleoperator(a->line,"struct.");
			}
			else if (child(a,0)->tp->struct_field.find(child(a,1)->text)==
			child(a,0)->tp->struct_field.end()) 
			{
				errornonfielddefined(a->line,child(a,1)->text);
			} 
			else 
			{
				a->tp=child(a,0)->tp->struct_field[child(a,1)->text];
			}
		}
	} 
	else 
	{
		cout<<"BIG PROBLEM No case defined for kind "<<a->kind<<endl;
	}
//cout<<"Ending with node \""<<a->kind<<"\""<<endl;
}

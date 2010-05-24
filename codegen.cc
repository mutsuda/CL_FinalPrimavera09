/* ===== semantic.c ===== */
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
#include "codegest.hh"

#include "myASTnode.hh"

#include "util.hh"
#include "semantic.hh"


#include "codegen.hh"

// symbol table with information about identifiers in the program
// declared in symtab.cc
extern symtab symboltable;

// When dealing with a list of instructions, it contains the maximum auxiliar space
// needed by any of the instructions for keeping the non-referenceable non-basic-type values.
int maxoffsetauxspace;

// When dealing with one instruction, it contains the auxiliar space
// needed for keeping non-referenceable values.
int offsetauxspace;

// For distinghishing different labels for different if's and while's.
int newLabelWhile(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}

int newLabelIf(bool inicialitzar = false){
  static int comptador = 1;
  if (inicialitzar) comptador = 0;
  return comptador++;
}

codechain GenCodSL(int nivells, int temp){
	codechain c;
	if (nivells>1) c="aload static_link t"+itostring(temp);
	
}


codechain indirections(int jumped_scopes,int t){
  codechain c;
  if (jumped_scopes==0) {
    c="aload static_link t"+itostring(t);
  }
  else {
    c="load static_link t"+itostring(t);
    for (int i=1;i<jumped_scopes;i++) {
      c=c||"load t"+itostring(t)+" t"+itostring(t);
    }
  }
  return c;
}

int compute_size(ptype tp){
  if (isbasickind(tp->kind)) {
    tp->size=4;
  }
  else if (tp->kind=="array") {
    tp->size=tp->numelemsarray*compute_size(tp->down);
  }
  else if (tp->kind=="struct") {
    tp->size=0;
    for (list<string>::iterator it=tp->ids.begin();it!=tp->ids.end();it++) {
      tp->offset[*it]=tp->size;
      tp->size+=compute_size(tp->struct_field[*it]);
    }
  }
  return tp->size;
}

void gencodevariablesandsetsizes(scope *sc,codesubroutine &cs,bool isfunction=0){
  if (isfunction) cs.parameters.push_back("returnvalue");
  for (list<string>::iterator it=sc->ids.begin();it!=sc->ids.end();it++) {
    if (sc->m[*it].kind=="idvarlocal") {
      variable_data vd;
      vd.name="_"+(*it);
      vd.size=compute_size(sc->m[*it].tp);
      cs.localvariables.push_back(vd);
    } else if (sc->m[*it].kind=="idparval" || sc->m[*it].kind=="idparref") {
      compute_size(sc->m[*it].tp);
      cs.parameters.push_back("_"+(*it));
    } else if (sc->m[*it].kind=="idfunc") {
      // Here it is assumed that in tp->right is kept the return value type
      // for the case of functions. If this is not the case you must
      // change this line conveniently in order to force the computation
      // of the size of the type returned by the function.
      compute_size(sc->m[*it].tp->right);
    } else if (sc->m[*it].kind=="idproc") {
      // Nothing to be done.
    }
  }
  cs.parameters.push_back("static_link");
}

codechain GenLeft(AST *a,int t);
codechain GenRight(AST *a,int t);


codechain CodeGenParam(AST *a, ptype paux, int t){
	if (paux->kind == "parval") return GenRight(a, t);
	else return GenLeft(a, t);
}


void CodeGenRealParams(AST *a,ptype tp,codechain &cpushparam,codechain &cremoveparam,int t){
  if (!a) return;
	if (symboltable[a->text].tp->kind=="function")
	{	
		if (isbasickind(tp->right->kind))
		{
				cpushparam="pushparam 0";
		}
		else
		{
			cpushparam="aload aux_space t"+itostring(t)+" addi t"+itostring(t)+" "+itostring(offsetauxspace)+" t"+itostring(t)+" pushparam t"+itostring(t);
            offsetauxspace+=compute_size(tp->right);
            t++;
		}
	}
	
	int numparam = 1;
	ptype paux = tp->down;
	
	for (AST *a1 = child(a->right, 0); a1!=0; numparam++, a1=a1->right, paux = paux->right)
	{
		cpushparam = cpushparam || CodeGenParam(a1, paux, t) || "pushparam t"+itostring(t);
	}
	
	cpushparam = cpushparam || indirections(symboltable.jumped_scopes(a->text), t)||
	"pushparam t"+itostring(t);
	
	cpushparam = cpushparam ||
	"call "+symboltable.idtable(a->text) + "_"+ a->text;
	
	for (int i=0;i<numparam; i++) cpushparam = cpushparam || "killparam";

	if (symboltable[a->text].tp->kind=="function")
	{
		if (isbasickind(tp->right->kind))
		{
			cpushparam = cpushparam || "popparam t"+itostring(t);
		}
		else
		{
			cpushparam=cpushparam||"killparam";
		}
		
	} 
}

// ...to be completed:
codechain GenLeft(AST *a,int t){
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->kind=="ident") 
  {
	  /*No es ta en el meu ambit*/
	  if (symboltable.jumped_scopes(a->text)>0) 
	  {
		  //Variable local o per referencia
		  if (symboltable[a->text].kind != "idparref")
		  {
			  c = indirections(symboltable.jumped_scopes(a->text), t);
			  c=c||
			  "addi t"+itostring(t)+" offset("+symboltable.idtable(a->text)+":_"+a->text+") t"+itostring(t);
		  }
		  //Parametre per referencia
		  else if (symboltable[a->text].kind=="idparref"){
			  c="load static_link t"+itostring(t)||
			  "addi t"+itostring(t)+" offset("+symboltable.idtable(a->text)+":_"+a->text+") t"+itostring(t)||
			  "load t"+itostring(t)+" t"+itostring(t);
		  }
	  }
	  else
	  {
		  if ((symboltable[a->text].kind=="idparval" )) 
		  {  
			  // Es un tipus basic
			  if(isbasickind(a->tp->kind))	c="aload _"+a->text+" t"+itostring(t);
			  // Es una variable local
			  else	c="load _"+a->text+" t"+itostring(t);
			  
			  //Es un parametre per referencia
		  }
		  else if(symboltable[a->text].kind=="idparref") 
		  {
			  c="load _"+a->text+" t"+itostring(t);
			  
			  //Es de tipus complex (carreguem direccio)
		  }
		  else 
		  {
			  c="aload _"+a->text+" t"+itostring(t);
		  }
		  
	  }

  }
  else if (a->kind==".")
  {
    c=GenLeft(child(a,0),t)||
      "addi t"+itostring(t)+" "+
      itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }
  else if (a->kind=="[")
  {
	  c = GenLeft(child(a,0),t) ||
		  GenRight(child(a,1),t+1)||
	      "muli t" + itostring(t+1) + " "+ itostring(child(a, 0)->tp->down->size) + " t" + itostring(t+1)||
	      "addi t" + itostring(t)+ " t"+itostring(t+1)+ " t" + itostring(t);
	  
  }
  else 
  {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}


// ...to be completed:
codechain GenRight(AST *a,int t){
  codechain c;

  if (!a) {
    return c;
  }

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  if (a->ref) {
    if (a->kind=="ident" && symboltable.jumped_scopes(a->text)==0 &&
	isbasickind(symboltable[a->text].tp->kind) && symboltable[a->text].kind!="idparref") {
	c="load _"+a->text+" t"+itostring(t);
    }
    else if (isbasickind(a->tp->kind)) {
      c=GenLeft(a,t)||"load t"+itostring(t)+" t"+itostring(t);
    }
    else 
	{
		int size = compute_size(a->tp);
		c=GenLeft(a,t+1)||
		"aload aux_space t"+itostring(t)||
		"addi t"+itostring(t)+" "+itostring(offsetauxspace)+ " t"+itostring(t)||
		"copy t"+itostring(t+1) + " t"+itostring(t) +" "+ itostring(size);
		offsetauxspace+=size;
    }    
  } 
	
  else if (a->kind=="<<")
  {
	  

	  c = c||"aload aux_space t"+itostring(t)||
	  "addi t"+itostring(t)+" "+itostring(offsetauxspace)+ " t"+itostring(t);
	  
	  offsetauxspace+=a->tp->size;

	  
	  AST *aux = child(child(a,0),0);
	  int i = 1;
	  int eoff=0;
	  while(aux!=0)
	  {
		
		  c = c||"aload aux_space t"+itostring(t);

		  c=c||GenRight(aux, t+1);
		  
		  string s = "e" + itostring(i);
		  ptype etp = a->tp->struct_field[s];
		  eoff = a->tp->offset[s];
		  

		  c=c||
		  "addi t"+itostring(t)+" "+itostring(eoff)+" t"+itostring(t);
		  if (isbasickind(etp->kind))
		  {
			  c=c||"stor t"+itostring(t+1)+" t"+itostring(t);
		  }
		  else
		  {
			  c=c||"copy t"+itostring(t+1)+" t"+itostring(t) + " " +itostring(etp->size);
		  }
			
		  
		  
		  
		  aux=aux->right;
		  i++;
	  }
	   c = c||"aload aux_space t"+itostring(t);
	  
	 	  
  }
	
  else if (a->kind=="intconst") {
    c="iload "+a->text+" t"+itostring(t);
  }
	
  else if (a->kind=="true")
  {
	  c="iload 1 t"+itostring(t);
  }
  else if (a->kind=="false")
  {
	  c="iload 0 t"+itostring(t);  
  }
		
  else if (a->kind=="+") 
  {
    c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "addi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="-" && child(a,1)!=0) 
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "subi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="-")
  {
	  c=GenRight(child(a,0),t)||
	  "mini t"+itostring(t)+" t"+itostring(t);
  }
  else if (a->kind=="*") 
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "muli t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="/") 
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "divi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="<")
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "lesi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind==">")
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "grti t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="=")
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "equi t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="not")
  {
	  c=GenRight(child(a,0),t)||
	  "lnot t"+itostring(t)+" t"+itostring(t);
  }
  else if (a->kind=="and")
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "land t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
  else if (a->kind=="or")
  {
	  c=GenRight(child(a,0),t)||
      GenRight(child(a,1),t+1)||
      "loor t"+itostring(t)+" t"+itostring(t+1)+" t"+itostring(t);
  }
	
  else if (a->kind=="(")
  {
	  codechain niahcedoc;
	  CodeGenRealParams(child(a,0), symboltable[child(a,0)->text].tp, c, niahcedoc, t);
  }

  else if (a->kind==".")
  {
	  ptype typ = symboltable[child(child(a,0),0)->text].tp;
	  CodeGenRealParams(child(child(a,0),0),typ,c,c,t);
	  c = c||"addi t"+itostring(t)+" "+itostring(child(a,0)->tp->offset[child(a,1)->text])+" t"+itostring(t);
  }

  else {
    cout<<"BIG PROBLEM! No case defined for kind "<<a->kind<<endl;
  }
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;
  return c;
}

// ...to be completed:
codechain CodeGenInstruction(AST *a,string info=""){
  codechain c;

  if (!a) {
    return c;
  }
  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  offsetauxspace=0;
  if (a->kind=="list") {
    for (AST *a1=a->down;a1!=0;a1=a1->right) {
      c=c||CodeGenInstruction(a1,info);
    }
  }
  else if (a->kind==":=") 
  {
	if (child(a,0)->kind=="<<")
	{
		AST * aux = child(child(child(a,0),0),0);
		c=GenRight(child(a,1), 0);
		ptype stp = child(a,1)->tp;
		
		int antOffset = 0;
		
		list<string>::iterator it = stp->ids.begin();
		int i = 1;

		while (it != stp->ids.end())
		{
			ptype etp = stp->struct_field[*it];
			
			/* Pas1: Obtener referencia a la variable adonde ira desempaaquetado*/
			c=c||GenLeft(aux,1);			
			
			/* Pas2: Calcular direccion del miembro del struct desde donde desempaquetareis*/
			c=c||"addi t0 "+itostring(antOffset)+" t0";
			if (!a->ref) c=c||"aload t0 t2";
			//else c=c||"load t0 t2";
			
			antOffset = etp->size;
			
			
			c=c||"copy t2 t1 "+itostring(etp->size);
			
			aux = aux->right;
			it++;
			i++;
		}
	}
	else
	{
		if (isbasickind(child(a,0)->tp->kind)) 
		{
			c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"stor t1 t0";
		}
		else if (child(a,1)->ref) 
		{
			c=GenLeft(child(a,0),0)||GenLeft(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
		}
		else 
		{
			c=GenLeft(child(a,0),0)||GenRight(child(a,1),1)||"copy t1 t0 "+itostring(child(a,1)->tp->size);
		}
	}
  } 
  else if (a->kind=="write" || a->kind=="writeln") {
    if (child(a,0)->kind=="string") {
      //...to be done. 
		c = "wris "+child(a,0)->text;
    } 
    else {//Exp
      c=GenRight(child(a,0),0)||"wrii t0";
    }

    if (a->kind=="writeln") {
      c=c||"wrln";
    }
  }
  else if (a->kind=="while")
  {
	  string label = itostring(newLabelWhile(false));
	  c="etiq while_"+label||
	  GenRight(child(a,0),0)||
	  "fjmp t0 endwhile_"+label||
	  CodeGenInstruction(child(a,1))||
	  "ujmp while_"+label||
	  "etiq endwhile_"+label;	  
  }
  else if (a->kind=="if")
  {
	  string label = itostring(newLabelIf(false));
	  
	  if (child(a,2)) 		  
	  {
		  /*te else*/
		  c=GenRight(child(a,0),0)||
		  "fjmp t0 else_"+label||
		  CodeGenInstruction(child(a,1))||
		  "ujmp endif_"+label||
		  "etiq else_"+label||
		  CodeGenInstruction(child(a,2))||
		  "etiq endif_"+label;  
	  }
	  else
	  {
		  /*no te else*/
		  c=GenRight(child(a,0),0)||
		  "fjmp t0 endif_"+label||
		  CodeGenInstruction(child(a,1))||
		  "etiq endif_"+label;  
	  }
  }
	else if (a->kind=="(")
	{
		codechain niahcedoc;
		CodeGenRealParams(child(a,0), symboltable[child(a,0)->text].tp, c, niahcedoc, 0);
	}
	else {
		cout<<"BIG PROBLEM! No case defined in CodeGenInstruction for kind "<<a->kind<< "("<<a->text<<") in line "<<a->line<<endl;
	}
	
	
	if (offsetauxspace > maxoffsetauxspace) 	maxoffsetauxspace = offsetauxspace;
	
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

  return c;
}

void CodeGenSubroutine(AST *a,list<codesubroutine> &l){
  codesubroutine cs;

  //cout<<"Starting with node \""<<a->kind<<"\""<<endl;
  string idtable=symboltable.idtable(child(a,0)->text);
  cs.name=idtable+"_"+child(a,0)->text;
  symboltable.push(a->sc);
  symboltable.setidtable(idtable+"_"+child(a,0)->text);
  
  gencodevariablesandsetsizes(a->sc,cs,a->kind=="function"?1:0);
	for (AST *a1=child(child(a, 2),0); a1!=0; a1=a1->right)
	{
		CodeGenSubroutine(a1, l);
	}
	
	maxoffsetauxspace=0;
	
	newLabelIf(true);
	newLabelWhile(true);
	
	codechain returncode;
	
	if (a->kind=="function")
	{
		if (isbasickind(child(a,4)->tp->kind))
		{
			returncode = GenRight(child(a, 4), 0)||
			"stor t0 returnvalue";
		}
		else
		{
			returncode = GenLeft(child(a, 4), 1)||
			"load returnvalue t0"||
			"copy t1 t0 "+itostring(child(a,4)->tp->size);
		}
	}
  	
	cs.c = CodeGenInstruction(child(a,3))||returncode||"retu";
	
	if (maxoffsetauxspace>0)
	{
		variable_data vd;
		vd.name="aux_space";
		vd.size=maxoffsetauxspace;
		cs.localvariables.push_back(vd);
	}
 	
 	
 
  symboltable.pop();
  l.push_back(cs);
  //cout<<"Ending with node \""<<a->kind<<"\""<<endl;

}

void CodeGen(AST *a,codeglobal &cg){
  initnumops();
  securemodeon();
  cg.mainsub.name="program";
  symboltable.push(a->sc);
  symboltable.setidtable("program");
  gencodevariablesandsetsizes(a->sc,cg.mainsub);
  for (AST *a1=child(child(a,1),0);a1!=0;a1=a1->right) {
    CodeGenSubroutine(a1,cg.l);
  }
  maxoffsetauxspace=0; newLabelIf(true); newLabelWhile(true);
  cg.mainsub.c=CodeGenInstruction(child(a,2))||"stop";
  if (maxoffsetauxspace>0) {
    variable_data vd;
    vd.name="aux_space";
    vd.size=maxoffsetauxspace;
    cg.mainsub.localvariables.push_back(vd);
  }
  symboltable.pop();
}


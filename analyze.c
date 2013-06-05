/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;
static void deleteProc(TreeNode *t) {
  if (t==NULL) return;
  else {
    if (t->scope < depth) {

      st_delete(t->scope);
      depth = t->scope;
    }
    return;
  }
}


/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
		      void (* preProc) (TreeNode *),
		      void (* postProc) (TreeNode *) )
{
  if (t != NULL)
    { 
      if (t->scope > depth) depth = t->scope;
      /*if ( t->nodekind == DeclK && t->kind.decl == paramK) {
	t->scope++;
	}*/
      preProc(t);
      { int i;
	for (i=0; i < MAXCHILDREN; i++) {
	  if (t->child[i] == NULL) continue;
	  if ( t->nodekind == StmtK && t->kind.stmt == CompoundK) {
	    t->child[i]->scope = t->scope + 1;
	  } 
	  else if(t->nodekind == DeclK && t->kind.decl == funK){
	    t->child[1]->scope = t->scope+1;
	  }
	  else {
	    t->child[i]->scope = t->scope;
	  }
	  traverse(t->child[i],preProc,postProc);
	}
      }
      deleteProc(t);
      postProc(t);
      //			deleteProc(t);
      if (t->sibling != NULL) t->sibling->scope = t->scope;
      traverse(t->sibling, preProc, postProc);
    }
}

/*static void deleteProc(TreeNode *t) {
  if (t==NULL) return;
  else {
  if (t->scope < depth) {
  st_delete(t->scope);
  depth = t->scope;
  }
  return;
  }
  }*/

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else {
    if (t->scope < depth) {
      st_delete(t->scope);
      depth = t->scope;
    }
    return;
  }
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
    {
    case ExpK:
      switch (t->kind.exp)
	{ case IdK:
	    if (st_lookup(t->attr.name) == -1)
	      /* not yet in table, so treat as new definition */
	      //st_insert(t->attr.name,t->lineno,location++, -1);
	      printf("Exp ERROr\n");
	    else
	      /* already in table, so ignore location, 
		 add line number of use only */ 
	      st_insert(t, 0);
	    break;
	default:
	  break;
	}
      break;
    case DeclK:
      if (t->array_size >= 0) { // if variable is not void
	if (st_advanced_lookup(t->attr.name, t->scope) == -1) {
	  st_insert(t, location++);
	} else {
	  printf("Declation Error\n"); location--;
	}
      }
      break;
    default:
      break;
    }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
/*void buildSymtab(TreeNode * syntaxTree)
  { 
  syntaxTree->scope = 0;*/
//traverse(syntaxTree,insertNode,deleteProc);
/*	traverse(syntaxTree,insertNode,checkNode);
	if (TraceAnalyze)
	{ fprintf(listing,"\nSymbol table:\n\n");
	printSymTab(listing);
	}
	}*/

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{
  BucketList l,r;
  int i,j;
  TreeNode * s;
  TreeNode * p;

  switch (t->nodekind)
    {
    case DeclK:
      switch (t->kind.decl){
      case varK:
	if(t->child[0] != NULL){
	  t->type = t->child[0]->type;
	}
	break;
      case funK:
	if(strcmp(t->attr.name,"main")==0){
	  if(t->sibling != NULL){
	    typeError(t,"main is not last function");
	    break;
	  }
	  else{
	    if(t->child[1]->type != Void){
	      typeError(t,"main has parameter");
	      break;
	    }
	  }
	}


	if(t->child[0] != NULL){
	  t->type = t->child[0]->type;
	}
	i=0;
	l = st_type_lookup(t->attr.name);
	l->type = t->type;

	if(t->child[1] == NULL){
	  l->paramnum = 0;
	  t->paramnum = 0;
	}
	else{
	  s = t->child[1];
	  while(s!=NULL){
	    if(i>0&&s->type == Void){
	      typeError(s,"Void parameter error");
	    }
	    /*if(s->type == Integer){
	      printf("param type : Integer\n");
	      }
	      else{
	      printf("param type : Void\n");
	      }*/
	    i++;
	    s = s->sibling;
	  }
	  l->paramnum = i;
	  t->paramnum = i;
	}

	//printf("%s %d\n",t->attr.name,l->paramnum);
	break;
      case paramK:
	if(t->array_size != -1){
	  t->type = t->child[0]->type;
	}
	else{

	}
	break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
	{
	case OpK:

	  break;
	case TypeK:

	  break;
	case ConstK:

	  break;
	case IdK:
	  l = st_type_lookup (t->attr.name);
	  if ( t->array_size == 0 && l->array_size > 0 ) {//array argument passing in function call
	    t->array_size = l->array_size;
	    //typeError(t,"Wrong type!");
	    //printf("Type Error\n");
	  }
	  else if(t->array_size > 0 && l->array_size == 0){//array to var err
	    typeError(t,"Wrong type!");
	  }
	  else if(l->array_size>0){
	    if(t->child[0]->nodekind == ExpK && t->child[0]->kind.exp == ConstK){
	      if(t->child[0]->attr.val < 0){
		typeError(t,"Negative Subscript Error");
	      }
	    }
	    else if(t->child[0]->type != Integer){
	      typeError(t,"Array Index Type Error");
	    }
	  }
	  else{
	    if(t->array_size == 0){

	    }
	    else{//array
	      t->attr.idx = t->child[0]->attr.val;
	    }
	  }
	  break;
	case CalcK:
	  if ((t->child[0]->type != Integer) ||
	      (t->child[2]->type != Integer))
	    typeError(t,"Op applied to non-integer");
	  // if ((t->attr.op == EQ) || (t->attr.op == LT))
	  //   t->type = Boolean;
	  else{
	    t->type = Integer;
	    if(t->child[1]->attr.op == LEQ){
	      if(t->child[0]->attr.val <= t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }
	    }
	    else if(t->child[1]->attr.op == LES){
	      if(t->child[0]->attr.val < t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }

	    }
	    else if(t->child[1]->attr.op == BIG){
	      if(t->child[0]->attr.val > t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }
	    }
	    else if(t->child[1]->attr.op == BEQ){
	      if(t->child[0]->attr.val >= t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }
	    }
	    else if(t->child[1]->attr.op == EQ){
	      if(t->child[0]->attr.val == t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }
	    }
	    else if(t->child[1]->attr.op == NEQ){
	      if(t->child[0]->attr.val != t->child[2]->attr.val){
		t->attr.val = 1;
	      }
	      else{
		t->attr.val = 0;
	      }
	    }
	    else if(t->child[1]->attr.op == PLUS){
	      t->attr.val = t->child[0]->attr.val + t->child[2]->attr.val;
	    }
	    else if(t->child[1]->attr.op == MINUS){
	      t->attr.val = t->child[0]->attr.val - t->child[2]->attr.val;
	    }
	    else if(t->child[1]->attr.op == MUL){
	      t->attr.val = t->child[0]->attr.val * t->child[2]->attr.val;
	    }
	    else if(t->child[1]->attr.op == DIV){
	      t->attr.val = t->child[0]->attr.val / t->child[2]->attr.val;
	    }
	  }
	  break;
	}
      break;
    case StmtK:
      switch (t->kind.stmt)
	{
	case CompoundK:

	  break;
	case IfK:
	  if(t->child[0]->attr.val != 0 && t->child[0]->attr.val != 1){
	    typeError(t->child[0],"if test is not Boolean");	   
	  }
	  break;
	case AssignK:
	  //BucketList l, r; 
	  l = r = NULL;
	  l = st_type_lookup(t->child[0]->attr.name);
	  if(t->child[1]->kind.stmt != ConstK){
	    r = st_type_lookup(t->child[1]->attr.name);
	  }
	  if(l == NULL){					
	    typeError(t->child[0],"invalid assignment");
	  }
	  else if(r == NULL){//expr
						
	    if(t->child[1]->type != Integer){
	      typeError(t->child[1],"invalid assignment2");
	    }
	    else{

	    }
	  }
	  else{//var to var assignment
						
	    if(l->array_size == 0  && r->array_size == 0){
	      if(l->type != Integer){
		typeError(t->child[0],"not integer");
	      }
	      else if(r->type != Integer){
		typeError(t->child[1],"not integer2");
	      }
	      else{
		t->type = Integer;

	      }
	    }
	    else if(l->array_size > 0 && r->array_size == 0){
	      if((t->child[0]->attr.idx > t->child[0]->array_size) || (t->child[0]->attr.idx < 0)){
		typeError(t->child[0],"out of array index range");
	      }
	      else{
		t->type = Integer;
	      }
	    }
	    else if(l->array_size == 0 && r->array_size > 0){
	      if((t->child[1]->attr.idx > t->child[1]->array_size) || (t->child[1]->attr.idx < 0)){
		typeError(t->child[1],"out of array index range");
	      }
	      else{
		t->type = Integer;
	      }
	    }
	    else{
	      if((t->child[1]->attr.idx > t->child[1]->array_size) || (t->child[1]->attr.idx < 0)){
		typeError(t->child[1],"out of array index range");
	      }
	      else if((t->child[0]->attr.idx > t->child[0]->array_size) || (t->child[0]->attr.idx < 0)){
		typeError(t->child[0],"out of array index range");
	      }
	      else{
		t->type = Integer;
	      }
	    }
	  }
	  break;
	case WhileK:
	  if (t->child[0]->type != Integer){
	    typeError(t->child[1],"repeat test is not Boolean");
	  }
	  else{
	    t->type = Integer;

	  }
	  break;
	case ReturnK:
	  if(t->child[0] == NULL){//void return
	    //t->type = Void;
	    typeError(t,"Function declaration has no return, but there is return stmt in function definition");
	  }
	  else{//data return
	    t->type = Integer;
	    if(t->child[0]->kind.stmt == CallK){
	      l=NULL;
	      l = st_type_lookup(t->child[0]->attr.name);
	      if(l == NULL){
		typeError(t,"unknown function name");
	      }
	      else{
		if(l->type != Integer){
		  typeError(t,"return type error2");
		}
		else{
		}
	      }
	    }
	    else{
	      if(t->child[0]->type == Void){
		typeError(t->child[0],"return type error");
	      }
	    }
	  }
	  break;
	case CallK:
	  l = st_type_lookup(t->attr.name);
	  if(l == NULL){
	    typeError(t,"unknown function name");
	  }
	  else{
	    if(l->paramnum == -1){//is not function name
	      typeError(t,"is not function name");
	    }
	    else{
	      i=0;
	      if(t->child[0] == NULL){//no argument
		if(l->paramnum == 0){

		}
		else{//need argument, but there is no argument
		  printf("%s\n",t->attr.name);
		  typeError(t,"arguments not match");
		}
	      }
	      else{//some argument
		s = t->child[0];
		while(s!=NULL){
		  i++;
		  s = s->sibling;
		}
		if(l->paramnum == i){
		  s = t->child[0];
		  p = l->tnode_p;
		  if(p->child[1] != NULL){
		    p = p->child[1];
		  }
		  while(s!=NULL){
		    /*r = NULL;
		      r = st_type_lookup(p->attr.name);
		      if(r == NULL){
		      printf("unknown %s %s\n",s->attr.name,p->attr.name);
		      typeError(s,"unknown argument");
		      }
		      else{
		      if(s->type != r->type){
		      typeError(s,"argument type is not matched");
		      }
		      else if(s->array_size != r->array_size){
		      typeError(s,"argument array size not match");
		      }
		      else{//okay

		      }
		      }*/
		    if(p == NULL){
		      typeError(s,"unknown argument");
		    }
		    else{
		      if(s->type != p->type){
			typeError(s,"argument type is not matched");
		      }
		      else if(s->array_size > 0 && p->array_size == 0){
			typeError(s,"argument type is not matched(array to var)");
		      }
		      else if(s->array_size == 0 && p->array_size > 0){
			typeError(s,"argument type is not matched(var to array)");
		      }
		      else{//okay

		      }
		    }
		    s = s->sibling;
		    p = p->sibling;
		  }
		}
		else{//number of arguments not match to number of parameters
		  typeError(t,"arguments not match2");
		}
	      }
	    }
	  }
	  break;
	default:
	  break;
	}
      break;
    default:
      break;

    }
}
/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  syntaxTree->scope = 0;
  //traverse(syntaxTree,insertNode,deleteProc);
  traverse(syntaxTree,insertNode,checkNode);
  if (TraceAnalyze)
    { fprintf(listing,"\nSymbol table:\n\n");
      printSymTab(listing);
    }
}
/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}

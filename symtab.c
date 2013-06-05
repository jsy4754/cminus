/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( TreeNode *t, int loc )
{ int h = hash(t->attr.name);
  BucketList l =  hashTable[h];
  TreeNode * s;
  int tmp;

  while ((l != NULL) && (strcmp(t->attr.name,l->name) != 0) && (l->scope != t->scope))
    l = l->next;

  if (l == NULL) /* variable not yet in table */
    { l = (BucketList) malloc(sizeof(struct BucketListRec));
      l->name = t->attr.name;
      l->lines = (LineList) malloc(sizeof(struct LineListRec));
      l->lines->lineno = t->lineno;
      l->memloc = loc;
      l->scope = t->scope;

      l->tnode_p = t;

      if(t->kind.decl == funK){
	l->type = t->child[0]->type;
	s = NULL;
	if(t->child[2] != NULL){
	  s = t->child[2];
	  while(s!=NULL){
	    if(s->nodekind == StmtK && s->kind.stmt == ReturnK){
	      //s->attr.name = copyString(t->attr.name);
	      tmp = strlen(t->attr.name);
	      s->attr.name = (char *)malloc((tmp+1)*sizeof(char));
	      strcpy(s->attr.name,t->attr.name);
	    }
	    s = s->sibling;
	  }
	}
      }
      else if(t->kind.decl == varK){
	l->type = t->child[0]->type;
      }
      else{
	l->type = t->type;
      }
      l->array_size = t->array_size;
      l->lines->next = NULL;
      l->next = hashTable[h];
      l->paramnum = -1;
      hashTable[h] = l; }
  else /* found in table, so just add line number */
    { LineList ll = l->lines;
      while (ll->next != NULL) ll = ll->next;
      ll->next = (LineList) malloc(sizeof(struct LineListRec));
      ll->next->lineno = t->lineno;
      ll->next->next = NULL;
    }
} /* st_insert */

void st_delete ( int scope ) {
  int i;
  fprintf(listing,"Scope  Variable Name  Location   Line Numbers\n");
  fprintf(listing,"-----  -------------  --------   ------------\n");

  for (i = 0; i < SIZE; i++) {
    BucketList l = hashTable[i];


    while ( (l != NULL) && (l->scope > scope) ){
      hashTable[i] = l->next;
      fprintf(listing,"%-5d  %-14s %-8d  ",l->scope,l->name,l->memloc);

      LineList t = l->lines;
      while(t != NULL) {
	LineList next = t->next;
	fprintf(listing,"%4d ",t->lineno);

	free(t);
	t = next;
      }
      fprintf(listing,"\n");
      printf("[DELETE] %s\n",l->name);
      free(l);
      l = hashTable[i];
    }
  }
  return ;
}

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ int h = hash(name);
  BucketList l =  hashTable[h];printf("[lookup] : %s\n", name);
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

/*
 */
int st_advanced_lookup ( char *name , int scope) {
  int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0) && (l->scope != scope))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

BucketList st_type_lookup ( char *name ){
  int h = hash(name);
  BucketList l =  hashTable[h];printf("[type lookup] : %s\n", name);
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;

  if (l == NULL){
    return NULL;
  }
  else{
    return l;
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Scope  Variable Name  Location   Line Numbers\n");
  fprintf(listing,"-----  -------------  --------   ------------\n");
  for (i=0;i<SIZE;++i)
    { if (hashTable[i] != NULL)
	{ BucketList l = hashTable[i];
	  while (l != NULL)
	    { LineList t = l->lines;
	      fprintf(listing,"%-5d  ",l->scope);
	      fprintf(listing,"%-14s ",l->name);
	      fprintf(listing,"%-8d  ",l->memloc);
	      while (t != NULL)
		{ fprintf(listing,"%4d ",t->lineno);
		  t = t->next;
		}
	      fprintf(listing,"\n");
	      l = l->next;
	    }
	}
    }
} /* printSymTab */

/*void printSymTab_scope(FILE * listing, int scope)
  { int i;
  fprintf(listing,"Scope  Variable Name  Location   Line Numbers\n");
  fprintf(listing,"-----  -------------  --------   ------------\n");
  for (i = 0; i < SIZE; i++) {
  BucketList l = hashTable[i];
  while ( (l != NULL) && (l->scope > scope) ){
  hashTable[i] = l->next;
  fprintf(listing,"%-5d  %-14s %-8d  ",l->scope,l->name,l->memloc);
  LineList t = l->lines;
  while(t != NULL) {
  LineList next = t->next;
  fprintf(listing,"%4d ",t->lineno);
  free(t);
  t = next;
  }
  fprintf(listing,"\n");
  printf("[DELETE] %s\n", l->name);
  free(l);
  l = hashTable[i];
  }
  }
  return ;

  }*/

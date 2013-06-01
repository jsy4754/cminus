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
    if ( t->nodekind == DeclK && t->kind.decl == paramK) {
      t->scope++;
    }
    preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++) {
	if (t->child[i] == NULL) break;
	if ( t->nodekind == StmtK && t->kind.stmt == CompoundK) {
	  t->child[i]->scope = t->scope + 1;
	} else {
	  t->child[i]->scope = t->scope;
	}
        traverse(t->child[i],preProc,postProc);
      }
    }
    postProc(t);
    if (t->sibling != NULL) t->sibling->scope = t->scope;
    traverse(t->sibling,preProc,postProc);
  }
}

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
            st_insert(t->attr.name,t->lineno,0, t->scope);
          break;
        default:
          break;
      }
      break;
    case DeclK:
      if (t->array_size >= 0) { // if variable is not void
		if (st_advanced_lookup(t->attr.name, t->scope) == -1) {
	      		st_insert(t->attr.name, t->lineno, location++, t->scope);
			printf("[INSERT] %x %s\n", t, t->attr.name);
		} else {
			printf("Declation Error\n"); location--;
	      	};
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
  traverse(syntaxTree,insertNode,deleteProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
         // if ((t->attr.op == EQ) || (t->attr.op == LT))
         //   t->type = Boolean;
          else
            t->type = Integer;
          break;
        case ConstK:
        case IdK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
        case WhileK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}

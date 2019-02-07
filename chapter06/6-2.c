/* 
 * Exercise 6-2. Write a program that reads a C program and prints in
 * alphabetical order each group variable names that are identical in the first
 * 6 characters, but different  somewhere thereafter. Don't count words within
 * string and comments. Make 6 a parameter that can be set from the command
 * line.
 * Note: getword comment detection could nbe improved 
 * By Faisal Saadatmand
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>                    /* for malloc && atoi */

#define MAXWORD 100
#define BUFSIZE 100
#define NKEYS   (sizeof keytab / sizeof keytab[0])
#define NSYMBOLS (sizeof symbol / sizeof symbol[0])

/* functions */
struct tnode *addtree(struct tnode *, char *);
void   treeprint(struct tnode *);
int    getword(char *, int);
struct tnode *talloc(void);            /* alocate memory to new tree node */
char   *strDup(char *);                /* copy string into safe place */
struct key *binsearch(char *, struct key *, int); 
void   findVariables(struct tnode *, int);

/* Globals */
char   buf[BUFSIZE];                   /* buffer from ungetch */
int    bufp = 0;                       /* next free position in buf */

struct tnode {                         /* the tree node: */
	char   *word;                      /* points to the text */
	int    count;                      /* number of occurrences */
	int    match;                      /* matching word */
	struct tnode *left;                /* left child */
	struct tnode *right;               /* right child */
};

struct key {
	char *word;
	int count;
};

struct key keytab[] ={
	{ "auto", 0 },
	{ "break", 0 },
	{ "case", 0 },
	{ "char", 0 },
	{ "const", 0 },
	{ "continue", 0 },
	{ "default", 0 },
	{ "do", 0 },
	{ "double", 0 },
	{ "else", 0 },
	{ "enum", 0 },
	{ "extern", 0 },
	{ "float", 0 },
	{ "for", 0 },
	{ "goto", 0 },
	{ "if", 0 },
	{ "int", 0 },
	{ "long", 0 },
	{ "register", 0 },
	{ "return", 0 },
	{ "short", 0 },
	{ "signed", 0 },
	{ "sizeof", 0 },
	{ "static", 0 },
	{ "struct", 0 },
	{ "switch", 0 },
	{ "typeof", 0 },
	{ "union", 0 },
	{ "unsigned", 0 },
	{ "void", 0 },
	{ "volatile", 0 },
	{ "while", 0 },
};

struct key symbol[] = {                /* array is sorted for binary search */
	{ "\"", 0 },
	{ "#", 0 },
	{ "*", 0 },
	{ "/", 0 },
	{ "\\", 0 },
	{ "_", 0 },
};

/* addtree: add a node with w, at or below p */
struct tnode *addtree(struct tnode *p, char *w)
{
	int cond;

	if (p == NULL) {                   /* a new word has arrived */
		p = talloc();                  /* make a new node */
		p->word = strDup(w);           /* copy data to it */
		p->count = 1;
		p->match = 0;                  /* initialize match */
		p->left = p->right = NULL;
	} else if ((cond = strcmp(w, p->word)) == 0)
		p->count++;                    /* repeated word */
	else if (cond < 0)                 /* less thant into left subtree */
		p->left = addtree(p->left, w);
	else 
		p->right = addtree(p->right, w);

	return p;
}

/* treeprint: in-order print of tree p */
void treeprint(struct tnode *p)
{
	if (p != NULL) {
		treeprint(p->left);
		if (p->match > 0)
			printf("%4d %4d  %s\n", p->count, p->match, p->word);
		treeprint(p->right);
	}
}

int getch(void)              /* get a (possibly pushed back) character */
{
	return (bufp > 0) ? buf[--bufp] : getchar();
}

void ungetch(int c)          /* push character back on input */
{
	if (bufp >= BUFSIZE)
		printf("ungetch: too many characters\n");
	else
		buf[bufp++] = c;
}

/* getword: get next word or character from input */
int getword(char *word, int lim)
{
	int   c, getch(void);
	void  ungetch(int);
	char  *w = word;
	struct key *p; 

	while (isspace(c = getch()))
		;
	
	if (c != EOF) {
		*w++ = c;
		*w = '\0';
	} else
		return c;

	if (!isalpha(c) && (p = binsearch(word, symbol, NSYMBOLS)) == NULL)
		return c;

	switch (c) {
	case '\\':                          /* handle escape sequences */
		c = getch();
		break;
	case '\"':                          /* skip words inside string constant */
		while ((c = getch()) != '\"')
			if (c == EOF)
				return c;
		break;
	case '#':                          /* skip preprocessor control lines */
		while ((c = getch()) != '\n')
			;
		ungetch(c);
		break;
	case '/':                          /* skip words inside C comments */
		if ((c = getch()) == '*') {
			while ((c = getch()))
				if	(c == '*' && (c = getch()) == '/')
					break; 
				else if (c == EOF)
					return c;
		} else                         /* don't skip pointer variables */
			ungetch(c);
		break;
	default:
		for ( ; --lim > 0; w++)
			if (!isalnum(*w = getch()) && *w != '_') {
				ungetch(*w);
				break;
			}
		break;
	}

	*w = '\0';
	return word[0];
}

/* talloc: make a tnode */
struct tnode *talloc(void)
{
	return (struct tnode *) malloc(sizeof(struct tnode));
}

/*strDup: make a duplicate of s */
char *strDup(char *s)
{
	char *p;

	p = (char *) malloc(strlen(s) + 1); /* +1 for '\0' */
	if (p != NULL)
		strcpy(p, s);
	return p;
}

/* binsearch: find word in tab[0]...tab[n - 1] */
struct key *binsearch(char *word, struct key *tab, int n)
{
	int cond;
	struct key *low = &tab[0];
	struct key *high = &tab[n];
	struct key *mid;

	while (low < high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(word, mid->word)) < 0)
			high = mid;
		else if (cond > 0)
			low = mid + 1;
		else
			return mid;
	}
	return NULL;
}

/* findVariables: finds matching variables in a binary search tree, using LDR
 * (inorder) traversal */
void findVariables(struct tnode *p, int n)
{
	if (p != NULL) {
		findVariables(p->left, n);
		if (p->left != NULL)
			if (strncmp(p->word, p->left->word, n) == 0)
				p->match = p->left->match = 1;
		if (p->right != NULL)
			if (strncmp(p->word, p->right->word, n) == 0)
				p->match = p->right->match = 1;
		findVariables(p->right, n);
	}
}

/* word frequency count */
int main(int argc, char *argv[])
{
	struct tnode *root;                /* root node */
	struct key *p;                     /* currently searched word */
	char   word[MAXWORD];              /* currently read word */
	int    nChar;                      /* number of characters to match */

	if (argc != 2)
		nChar = 6;
	else
		nChar = atoi(argv[1]);

	root = NULL;                       /* initialize root node */
	while (getword(word, MAXWORD) != EOF)
		if ((isalpha(word[0]) || word[0] == '_' || word[0] == '*')
				&& (int) strlen(word) > nChar)
			if ((p = binsearch(word, keytab, NKEYS)) == NULL) /* skip C */
				root = addtree(root, word);                   /* reserved words */
	findVariables(root, nChar);
	treeprint(root);
	return 0;
}

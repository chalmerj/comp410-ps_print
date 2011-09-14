#include <ansi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* #TODO
 * -- Implement Tree Functions
*/

/* Definitions */
#define psCommand "ps -afe"

/* Typedef Declarations */
struct node {
	int pid;
	struct node *parent;
	struct node *child;
	struct node *sibling;
};

struct node pstree;

/* Prototypes */
int getline(char *line, FILE *fp, int size);
int growTree(struct node *node, int pid, int ppid);
struct node *lookupNode(struct node *node, int pid);

/*
*	void printTree(Tree_t *tree, int indent);
*/


int main (int argc, char **argv)
{
    FILE *fp = 0;
    char line[128];
    int status = 0;
    int lineNumber = 0;
    int pidIndex, ppidIndex, cmdIndex = -1;
    char *token;



	memset((void*)&pstree, 0, sizeof(pstree));

    fp =(FILE*) popen(psCommand, "r");

    while (1)
    {
        status = getline(line, fp, sizeof(line));
        if (status <=0) break;

        long int pid = -1;
        int ppid = -1;
        int tokenNumber = 0;

        token = strtok(line, " \t\n");

        while (NULL != token)
        {
            if (0 == lineNumber)
            {
                if (0 == strncmp("PID", token, sizeof("PID")))      {  pidIndex = tokenNumber;}
                if (0 == strncmp("PPID", token, sizeof("PPID")))    { ppidIndex = tokenNumber;}
            }
            else
            {
                if (tokenNumber == pidIndex)
                {
                    /*printf("Token: %s\n",token);*/

                    if (NULL != strchr(token, '('))     /* Check for '(' in PID, then convert to signed long */
                    {
                        char *firstChar, *lastChar = 0;
                        firstChar = strchr(token,'(')+1;
                        lastChar = strchr(token,')');
                        pid = strtol(firstChar, (char**) lastChar, 10);
                    }
                    else
                    {
                        pid = atoi(token);
                    }
                    printf("PID: %3d\t", pid);
                }
                else if (tokenNumber == ppidIndex)
                {
                    ppid = atoi(token);
                    printf("PPID: %4d\n", ppid);
                }
            }

            token = strtok(NULL, " \t\n");
            tokenNumber++;
        }

        lineNumber++;
    }

    pclose(fp);
}
int growTree(struct node *node, int pid, int ppid)
{
	struct node *thisNode;
	struct node *parentNode;
	
	thisNode = lookupNode(node,ppid);
	
	if (thisNode) /*Node found; add either child or child's sibling */
	{
		parentNode = thisNode; /*Set parent node */
	
		if (thisNode->child) /*If this node has a child, move to child and follow siblings */
		{
			thisNode = thisNode->child;
			while(thisNode->sibling) thisNode = thisNode->sibling;
			/* malloc a new node */
			thisNode->sibling = (struct node* )malloc(sizeof(struct node));
			if (NULL == thisNode->sibling) return -1; /*malloc failed and returned NULL */
			/*Fill in node values, parented to parentNode */
			thisNode->sibling->parent = parentNode;
			thisNode->sibling->child = NULL;
			thisNode-sibling->sibling = NULL;
			thisNode->sibling->pid = pid;
		}
		
		else /*If this node doesn't have a child, create one */
		{
			thisNode->child = (struct node* )malloc(sizeof(struct node));
			if (NULL == thisNode->child) return -1; /*malloc failed and returned NULL */
			thisNode->child->parent = parentNode;
			thisNode->child->child = NULL;
			thisNode->child->sibling = NULL;
			thisNode->child->pid = pid;
		}
		
	}
	else	/*Create new node, starting with PPIDs. The children PIDs come after */
	{
		thisNode = node;
		thisNode->pid = ppid; 
		thisNode->parent = NULL;
		thisNode->child = NULL;
		thisNode->sibling = NULL;
		/*Debug Printing*/
		printf("growTree:New root:this->pid %d\n", thisTree->pid);
	}
}


struct node *lookupNode(struct node *node, int pid)
{
	struct node *thisNode = node;
	
	/* Null check: If we're passed a null input node, return 0 cast os a node. */
	if (NULL == thisNode) return (struct node*)0;
	
	while (thisNode)
	{
		if (thisNode->pid == pid) 
		{
			return thisNode; /* We found it, return this node */
		}
		
		else 
		{
			struct node *temp = lookupNode(thisNode->sibling, PID); /* Recursively check sibling nodes */
			
			if (temp) return temp; /*Found in sibling node */
		}
		
		thisNode = thisNode->child; /* Move to child node, search again */
	}
	
	return thisNode; /* Not in the tree */
}


int getline(char *line, FILE *fp, int size)
{
    int i = 0;

    memset(line, 0, size);

    for (i = 0; i < size-1; i++)
    {
        char c;

        if (1 != fread(&c, 1, 1, fp))
        {
            if (strlen(line)) return strlen(line);
            return -1;
        }

        line[i] = c;

        if ('\n' == c) return strlen(line);
    }
}

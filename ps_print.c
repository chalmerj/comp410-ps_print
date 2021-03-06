#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* #TODO
*	-- Beautify printTree function. 
*/

/* Definitions */
#ifdef MACOSX
#define psCommand "ps -face | awk ' NR >1{print $2\"\t\"$3}' | sort -n -k 2"
#else
#define psCommand "ps -afe | awk ' NR >1{print $4\"\t\"$5}' | sort -n +1"
#endif 

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
void printTree(struct node *tree, int indentSize);


int main (int argc, char **argv)
{
    FILE *fp = 0;
    char line[256];
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

        long int pid = -10;
        int ppid = -10;
        int tokenNumber = 0;

        token = strtok(line, " \t\n");

        while (NULL != token)
        {

			pidIndex = 0;
			ppidIndex = 1;
			
                if (tokenNumber == pidIndex)
                {
                    // printf("Token: %s\n",token);

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
                     // printf("PID: %3d\t", pid);
                }
                else if (tokenNumber == ppidIndex)
                {
                    ppid = atoi(token);
                     // printf("PPID: %4d\n", ppid);
                }
            
			if ((pid >= -5) && (ppid >= -5)) /*If both pid and ppid are set, grow the tree and move on */
			{
				growTree(&pstree, pid, ppid);
				break;
			}
			
            token = strtok(NULL, " \t\n");
            tokenNumber++;
        }

        lineNumber++;
    }
	printTree(&pstree, 0);
	printf("Done\n");
    pclose(fp);
}

void printTree(struct node *node, int indentSize)
{
	char indents[128];
	char *indentLine = "	";
	int i;
	
	//Zero out the indents string
	memset(indents, 0, sizeof(indents));

	// Make indent string larger up to indentSize
	for (i = 0; i < indentSize; i++)
	{
		strcat(indents, indentLine);
	}
	
	printf("%s|->%d\n",indents, node->pid);
	if (node->child) printTree(node->child, indentSize+1);
	if (node->sibling) printTree(node->sibling, indentSize);

	
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
			thisNode->sibling->sibling = NULL;
			thisNode->sibling->pid = pid;
			/*Debug Printing*/
			// printf("growTree:New sibling:parent->pid %d\t sibling->pid %d\n", 
			// 				thisNode->sibling->parent->pid, thisNode->sibling->pid);
		}
		
		else /*If this node doesn't have a child, create one */
		{
			thisNode->child = (struct node* )malloc(sizeof(struct node));
			if (NULL == thisNode->child) return -1; /*malloc failed and returned NULL */
			thisNode->child->parent = parentNode;
			thisNode->child->child = NULL;
			thisNode->child->sibling = NULL;
			thisNode->child->pid = pid;
			/*Debug Printing*/
			// printf("growTree:New child:parent->pid %d\t child->pid %d\n", 
			// 		thisNode->child->parent->pid, thisNode->child->pid);
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
		// printf("growTree:New root:this->pid %d\n", thisNode->pid);
	}
}


struct node *lookupNode(struct node *node, int pid)
{
	struct node *thisNode = node;
	
	/* Null check: If we're passed a null input node, return 0 cast os a node. */
	if (NULL == thisNode) 
	{
		// printf("lookupNode: NULL Match, returning 0\n");
		return (struct node*)0;
	}
	
	while (thisNode)
	{
		if (thisNode->pid == pid) 
		{
			return thisNode; /* We found it, return this node */
			// printf("lookupNode: PID Match; returning thisNode\n");
		}

		{
		// printf("lookupNode: recursively searching\n");
		struct node *temp = lookupNode(thisNode->sibling, pid); /* Recursively check sibling nodes */
		if (temp) return temp; /*Found in sibling node */
		}
		
		thisNode = thisNode->child; /* Move to child node, search again */
		// printf("lookupNode: moving to child\n");
	}
	
	return thisNode; /* Not in the tree */
	// printf("lookupNode: returning thisNode\n");
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

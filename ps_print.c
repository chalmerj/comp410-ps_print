#include <ansi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* #TODO
 * -- Implement Tree Functions
 * -- Handle '(' and ')' in token (remove them?)
*/

/* Definitions */
#define psCommand "ps -afe"

/* Typedef Declarations */
typedef struct tree_s
{
    struct tree_s *parent;  /* Ptr to the Parent Node */
    struct tree_s *child;   /* Ptr to the first child node */
    struct tree_s *sib;     /* Ptr to a sibling node (same parent ptr) */
    int leaf;               /*The actual PID of the process */
} Tree_t;

/* Prototypes */
int getline(char *line, FILE *fp, int size);
/*int growTree(Tree_t *tree, int pid, int ppid);
*Tree_t *lookUpLeaf(Tree_t *tree, int leaf);
*void printTree(Tree_t *tree, int indent);
*/


int main (int argc, char **argv)
{
    FILE *fp = 0;
    char line[128];
    int status = 0;
    int lineNumber = 0;
    int pidIndex, ppidIndex, cmdIndex = -1;
    char *token;

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

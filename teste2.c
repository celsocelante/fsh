#include <stdio.h>
#include <string.h>

int main(void)
{
    char abc[] = "gedit @ firefox @ ps l";
    char *tok = abc;

    while ((tok = strtok(tok, "@")) != NULL)
    {
        printf("<<%s>>\n", tok);
        tok = NULL;
    }
    return 0;
}
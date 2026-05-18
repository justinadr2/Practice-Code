/*

code for printing 'Hello'
++++++++++[>+++++++>++++++++++>+++++++++++<<<-]>++.>+.>--..+++.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{

    char tape[8] = "\x00\x00\x00\x00\x00\x00\x00\x00";

    char* ptr = &tape[0];

    char code[128];
    scanf("%127s", code);

    char* ip = &code[0];

    while (*ip != '\0')
    {
        switch (*ip)
        {
            case '>':
                if (ptr >= &tape[7])
                {
                    printf("out of bounds\n");
                    exit(1);
                }
                ptr++;
                break;
            case '<':
                if (ptr == &code[0])
                {
                    printf("out of bounds\n");
                    exit(1);
                }
                ptr--;
                break;
            case '.':
                printf("%c", *ptr);
                break;
            case '+':
                (*ptr)++;
                break;
            case '-':
                (*ptr)--;
                break;
            case '[':
                if (*ptr == 0)
                {
                    int depth = 1;
                    while (depth > 0)
                    {

                        ip++;
                        if (*ip == '\0')
                        {
                            printf("Error: unmatched '['\n");
                            exit(1);
                        }
                        if (*ip == '[')
                            depth++;
                        if (*ip == ']')
                            depth--;
                    }
                }
                break;
            case ']':
                if (*ptr != 0)
                {
                    int depth = 1;
                    while (depth > 0)
                    {
                        ip--;
                        if (ip < &code[0])
                        {
                            printf("Error: unmatched ']'\n");
                            exit(1);
                        }
                        if (*ip == ']')
                            depth++;
                        if (*ip == '[')
                            depth--;
                    }
                }
                break;
            default:
                break;
        }
        ip++;
    }


    printf("\nFinal state: %s\n", tape);


}
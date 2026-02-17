#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LEN 64
#define MAX_TOKENS    16
#define MAX_LINES     64

typedef struct 
{
    char **tokens;  
    int count;
} Line;

Line line_create(void)
{
    Line line;
    line.count = 0;
    line.tokens = malloc(sizeof(char *) * MAX_TOKENS);
    return line;
}

void line_add_token(Line *line, const char *text)
{
    if (line->count >= MAX_TOKENS)
        return;

    line->tokens[line->count] = malloc(strlen(text) + 1);
    strcpy(line->tokens[line->count], text);
    line->count++;
}

void line_free(Line *line)
{
    for (int i = 0; i < line->count; i++)
        free(line->tokens[i]);

    free(line->tokens);
}

int main(void)
{
    FILE *file = fopen("code.bin", "r");
    if (!file) 
    {
        perror("Failed to open file");
        return 1;
    }

    Line lines[MAX_LINES];
    int line_count = 0;

    lines[line_count] = line_create();

    int c;
    char token[MAX_TOKEN_LEN];
    int token_len = 0;

    while ((c = fgetc(file)) != EOF) 
    {
        if (c == ' ' || c == ';') 
        {
            if (token_len > 0) 
            {
                token[token_len] = '\0';
                line_add_token(&lines[line_count], token);
                token_len = 0;
            }

            if (c == ';') 
            {
                line_count++;
                if (line_count >= MAX_LINES)
                    break;
                lines[line_count] = line_create();
            }
        }
        else if (!isspace((unsigned char)c)) 
        {
            if (token_len < MAX_TOKEN_LEN - 1)
                token[token_len++] = (char)c;
        }
    }

    fclose(file);

    for (int i = 0; i < line_count; i++) 
    {
        printf("line %d: ", i + 1);
        for (int j = 0; j < lines[i].count; j++) 
        {
            if (j > 0) printf(", ");
            printf("%s", lines[i].tokens[j]);
        }
        printf("\n");
    }

    for (int i = 0; i < line_count; i++)
        line_free(&lines[i]);

    return 0;
}


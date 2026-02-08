#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    
    char* filename = "disas.txt";
    FILE* file = fopen(filename, "w");

    fprintf(file, "xor eax, eax\n");

    char data[] = { 0x41, 0x51, 0x44, 0x55 };

    fwrite(data, sizeof(char), sizeof(data), file);

    fclose(file);

}

// get the file offset (.exe) of a process .text address

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

int main()
{
    uint64_t address, base, size, text;
    printf("process address: ");
    scanf("%" SCNx64, &address);

    printf(".exe base address in process: ");
    scanf("%" SCNx64, &base);

    printf(".exe size in process: ");
    scanf("%" SCNx64, &size);


    printf(".text base address in file: ");
    scanf("%" SCNx64, &text);

    printf("static .text offset = 0x%" PRIX64 "\n", (address - base - size) + text);

}

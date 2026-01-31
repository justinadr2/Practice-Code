#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <windows.h>

int main(int argc, char* argv[])
{
    char input[64];    
    printf(": ");
    fgets(input, sizeof(input), stdin);
    char alpha[26 + 26 + 10 + 14] = "Na7MbL!Oc+d5@eP9#KfJg$_2QhI^RiHSjkGT&1 lmFU*noVEp3(Wq=r0XDst4)CuY6vB_Zwx8Ayz";
    char* temp = "";

    for (int i = 0; i < strlen(input) - 1; i++)
    {  
        for (int j = 0; j < strlen(alpha); j++)
        {
            if (alpha[j] == input[i])
            {   
                temp[i] = alpha[j];
                temp[i + 1] = '\0';
                printf("%s\n", temp);
                break;
            }
            
            printf("%s%c\n", temp, alpha[j]);               
            Sleep(10);          
        }
    }   
    return 0;
}



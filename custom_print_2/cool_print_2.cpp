
#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
using namespace std;

int main()
{
    char c = 'X';
    string text;
    string sub;
    string space = "";

    for (int i = 0; i < 80; i++)
        text.push_back(c);
    
    int len = text.length();
    
    int idx = 3000;
    while (idx--)
    {
        for (int i = 0; i < len; i++)
        {
            int j = (i < len / 2) ? i : ( len - 1 - i);

            string space(j, ' ');
            int sub_start = j;
            int sub_len = len - (2 * j);
            sub = text.substr(sub_start, sub_len);
            
            cout << space + sub << '\n';
        }
    }
    system("cls");
}

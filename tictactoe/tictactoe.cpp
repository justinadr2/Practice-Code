
#include <iostream>
#include <string>
#include <fstream>

#include <windows.h>
#include <random>

using namespace std;

string cell = "123456789";
void Print()
{
    
    cout << "-------------\n";
    cout << "| " << cell[0] << " | " << cell[1] << " | " << cell[2] << " |\n";
    cout << "|---+---+---|\n";
    cout << "| " << cell[3] << " | " << cell[4] << " | " << cell[5] << " |\n";
    cout << "|---+---+---|\n";
    cout << "| " << cell[6] << " | " << cell[7] << " | " << cell[8] << " |\n";
    cout << "-------------\n\n";
}

bool CheckWin()
{
    int winLines[8][3] = 
    {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };
    for (int i = 0; i < 8; i++)
    {
        int a = winLines[i][0];
        int b = winLines[i][1];
        int c = winLines[i][2];

        if (cell[a] == cell[b] && cell[b] == cell[c])
        {
            return true;   
        }
    }
    return false;
}

int i = 0;
char taken[9];
bool cell_taken(char c)
{
    for (int i = 0; i < 9; i++)
    {
        if (c == taken[i])
            return true;
    }
    return false;
}

int main()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 9);

    bool win = false;
    char input;

    Print();

    int turns = 0;
    while (!win)
    {
        cout << "Player's turn: ";
        cin >> input;

        if (cell_taken(input))
        {
            cout << "Cell taken! try again\n";
            continue;
        }

        taken[i++] = input;
        cell[input - '1'] = 'X';
        
        Print();
        
        if (CheckWin())
        {
            cout << "Player wins!\n";
            win = true;
            break;
        }
        else if (turns++ == 4)
        {
            cout << "Game ties\n";
            break;
        }


        string bot = to_string(distrib(gen));
        char c = bot[0];
        while (cell_taken(c))
        {
            bot = to_string(distrib(gen));
            c = bot[0];
        }
        taken[i++] = c;
        cell[c - '1'] = 'O';

        cout << "Bot's turn: " << c << '\n';

        Print();
        if (CheckWin())
        {
            cout << "Bot wins!\n";
            break;
        }

    }
}

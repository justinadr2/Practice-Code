#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>

#define MAX_TOKEN_LEN   64
#define MAX_TOKENS      16
#define MAX_LINES       64

struct Line 
{
    std::vector<std::string> tokens;
};

int main()
{
    std::ifstream file("code.bin");
    if (!file) 
    {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    std::vector<Line> lines;
    lines.reserve(MAX_LINES);
    lines.push_back(Line{});

    char c;
    std::string token;

    while (file.get(c)) 
    {

        if (c == ' ' || c == ';') 
        {
            if (!token.empty()) 
            {
                if (lines.back().tokens.size() < MAX_TOKENS)
                    lines.back().tokens.push_back(token);
                token.clear();
            }

            if (c == ';') 
            {
                if (lines.size() >= MAX_LINES)
                    break;
                lines.push_back(Line{});
            }
        }
        else if (!std::isspace(static_cast<unsigned char>(c))) 
        {
            if (token.size() < MAX_TOKEN_LEN - 1)
                token.push_back(c);
        }
    }

    for (size_t i = 0; i < lines.size() - 1; i++) 
    {
        std::cout << "line " << i + 1 << ": ";
        for (size_t j = 0; j < lines[i].tokens.size(); j++) 
        {
            if (j > 0) std::cout << ", ";
            std::cout << lines[i].tokens[j];
        }
        std::cout << '\n';
    }

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

typedef struct Node_
{
    struct Node_** nodes;
    char* val;
    int num_nodes;
} Node;

Node* ParseJSONToNode(cJSON* item) 
{
    if (!item) 
        return NULL;

    Node* node = malloc(sizeof(Node));
    
    cJSON* val_obj = cJSON_GetObjectItem(item, "val");
    node->val = val_obj ? _strdup(val_obj->valuestring) : _strdup("Unknown");

    cJSON* nodes_array = cJSON_GetObjectItem(item, "nodes");
    node->num_nodes = cJSON_GetArraySize(nodes_array);

    if (node->num_nodes > 0) 
    {
        node->nodes = malloc(node->num_nodes * sizeof(Node*));
        for (int i = 0; i < node->num_nodes; i++) 
        {
            cJSON* child_item = cJSON_GetArrayItem(nodes_array, i);
            node->nodes[i] = ParseJSONToNode(child_item);
        }
    } 
    else 
    {
        node->nodes = NULL;
    }

    return node;
}

char* ReadJSONFile(const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f) 
        return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(len + 1);
    fread(data, 1, len, f);
    fclose(f);
    data[len] = '\0';
    return data;
}

void PrintTree(Node* root, int depth, int is_last[])
{
    
    if (!root)
        return;
    
    for (int i = 0; i < root->num_nodes; i++)
    {
        int current_is_last = (i == root->num_nodes - 1);
        
        for (int d = 0; d < depth; d++)
        {
            if (is_last[d])
                printf("    ");
            else
                printf("│   ");
        }
        
        printf("│\n");

        for (int d = 0; d < depth; d++)
        {
            if (is_last[d])
                printf("    ");
            else
                printf("│   ");
        }

        if (current_is_last)
            printf("└── ");
        else
            printf("├── ");
        printf("%s\n", root->nodes[i]->val);

        if (root->nodes[i]->num_nodes > 0)
        {
            is_last[depth] = current_is_last;
            PrintTree(root->nodes[i], depth + 1, is_last);
        }
    }
}

void Free(Node* root)
{
    if (!root) 
        return;

    if (root->val) 
        free(root->val);

    if (root->nodes) 
    {
        for (int i = 0; i < root->num_nodes; i++) 
            Free(root->nodes[i]); 
        free(root->nodes);
    }

    free(root); 
}

int main()
{
    system("chcp 65001 > nul");

    char* json_text = ReadJSONFile("hier.json");
    
    cJSON* root_json = cJSON_Parse(json_text);

    Node* root = ParseJSONToNode(root_json);

    printf("%s\n", root->val);
    int is_last[100] = {0};
    PrintTree(root, 0, is_last);

    cJSON_Delete(root_json);
    free(json_text);
    Free(root);
    return 0;
}

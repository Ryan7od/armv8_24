#include <stdlib.h>
#include <stdio.h>
struct list {
    int *data;
    int numItems;
    int size;
};

struct SA_pair {
    char Symbol[100];
    int address;
};


void addToTable(struct list *mySymbolTable, struct SA_pair new_symbol);


int main(int argc, char **argv) {
    struct list SymbolTable;
    SymbolTable.numItems = 0;
    SymbolTable.size = 20;
    SymbolTable.data = calloc(SymbolTable.size, sizeof(struct SA_pair));

    if (SymbolTable.data == NULL) {
        printf("memory allocation failed");
        return 1;
    }

  return EXIT_SUCCESS;
}



#include <stdlib.h>
#include <stdio.h>

struct SA_pair {
    char Symbol[100];
    int address;
};


struct list {
    struct SA_pair *data;
    int numItems;
    int size;
};

typedef struct {} IR;



void addToTable(struct list *mySymbolTable, struct SA_pair new_symbol);
IR *parser(char *line, int *data);


int main(int argc, char **argv) {
    struct list SymbolTable;
    SymbolTable.numItems = 0;
    SymbolTable.size = 20;
    SymbolTable.data = malloc(SymbolTable.size * sizeof(struct SA_pair));

    if (SymbolTable.data == NULL) {
        printf("memory allocation failed");
        return 1;
    }

  return EXIT_SUCCESS;
};

void addToTable(struct list *mySymbolTable, struct SA_pair new_symbol) {
    if (mySymbolTable->numItems == mySymbolTable->size) {
        mySymbolTable->size *= 2;
        mySymbolTable->data = realloc(mySymbolTable->data, mySymbolTable->size * sizeof(struct SA_pair));
        if (mySymbolTable->data == NULL) {
            printf("Symbol table failed to be resized");
        }
    }
    mySymbolTable->data[mySymbolTable->numItems] = new_symbol;
    mySymbolTable->numItems++;
}


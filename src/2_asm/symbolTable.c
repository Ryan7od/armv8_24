#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdint.h>

#include "symbolTable.h"


void growTable(dynarray mySymbolTable) { //helper function to increase size of symbol table if needed
    if (mySymbolTable->numItems == mySymbolTable->size) {
        mySymbolTable->size *= 2;
        mySymbolTable->data = realloc(mySymbolTable->data, mySymbolTable->size * sizeof(struct SA_pair));
        if (mySymbolTable->data == NULL) {
            printf("Symbol table failed to be resized");
        }
    }
}

void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol) {
    growTable(mySymbolTable);
    mySymbolTable->data[mySymbolTable->numItems] = new_symbol;
    mySymbolTable->numItems++;
}

struct SA_pair createSA(char *label, int lineNo) { //creates a symbol table pair 
    struct SA_pair newSA;
    newSA.Symbol = label;
    int address = lineNo;
    newSA.address = address;
    return newSA;
}

void freeTable(dynarray symbolTable) {
    free(symbolTable->data);
    free(symbolTable);
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct SA_pair {
    char Symbol[100];
    int address;
};

struct list {
    struct SA_pair *data;
    int numItems;
    int size;
};
#define MaxLineLength 30
#define SymbolTableSize 20
typedef enum {instruction, directive, label} LineType;

#define MAX_OPERANDS 4
typedef struct {
    char *opcode;
    char operand[MAX_OPERANDS];
} InstructionIR;



void addToTable(struct list *mySymbolTable, struct SA_pair new_symbol);
void parser(char *line);


int main(int argc, char **argv) {
    struct list SymbolTable;
    SymbolTable.numItems = 0;
    SymbolTable.size = SymbolTableSize;
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
void fileProcessor(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        // Remove trailing newline character
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        parser(line);
    }

    free(line);
    fclose(file);
}

void tokenizer(char instruction[]) {
    char* mne = strtok(instruction, " ");
    char* ops = strtok(NULL, " ");
    printf("%s\n", mne);
    printf("%s\n", ops);
    char* operand;
    char* operands[4];
    int operand_count = 0;
    operand = strtok(ops, ",");
    while (operand != NULL && operand_count < 4) {
        operands[operand_count++] = operand;
        operand = strtok(NULL, ",");
    }
    // Print the operands
    for (int i = 0; i < operand_count; i++) {
        printf("Operand %d: %s\n", i + 1, operands[i]);
    }
}

void parser(char *line) {
    char *s = line;
    while (isspace(*s)) { s++; };
}


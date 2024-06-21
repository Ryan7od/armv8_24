#define SymbolTableSize 20

struct SA_pair {
    char *Symbol;
    int address;
};

struct dynarray {
    struct SA_pair *data;
    int numItems;
    int size;
};
typedef struct dynarray *dynarray;

extern struct SA_pair createSA(char *label, int lineNo);
extern void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol);
extern void freeTable(dynarray symbolTable);




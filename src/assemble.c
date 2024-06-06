#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MaxLineLength 30
#define SymbolTableSize 20




struct SA_pair {
    char *Symbol;
    int address;
};

struct list {
    struct SA_pair *data;
    int numItems;
    int size;
};


typedef enum {instruction, directive, label} LineType;

#define MAX_OPERANDS 4
typedef struct {
    char *opcode;
    char *operand[MAX_OPERANDS];
} InstructionIR;

typedef void (*InstructionParser)(InstructionIR, FILE *);

uint32_t data_processing_immediate_code = 1 << 28;
uint32_t data_processing_register_code = 5 << 27;

typedef struct {
    const char* mnemonic;
    InstructionParser parser;
} InstructionMapping;

typedef struct {
    const char* mnemonic;
    uint32_t opcode_bin;
} OpcodeMapping;

OpcodeMapping *opcodeMapping;
size_t opcode_map_size;


void addToTable(struct list *mySymbolTable, struct SA_pair new_symbol);




static void parser(char *line);
char* DataProcessingInstruction(InstructionIR instruction);

// Function pointer type for instruction parsers


// Function declarations
void parseBranch(InstructionIR instruction, FILE *file);
void parseLoadStore(InstructionIR instruction, FILE *file);
void parseDataProcessing(InstructionIR instruction, FILE *file);

// Parsing functions
void parseBranch(InstructionIR instruction, FILE *file) {
    printf("Parsing Branch instruction: %s\n", instruction);
}

void parseLoadStore(InstructionIR instruction, FILE *file) {
    printf("Parsing Load/Store instruction: %s\n", instruction);
}

void parseDataProcessing(InstructionIR instruction, FILE *file) {

}

static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);

static void parseTwoOperand(InstructionIR instruction, FILE *file);

static void parser(char *line);
static InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);


int main(int argc, char **argv) {
    struct list SymbolTable;
    SymbolTable.numItems = 0;
    SymbolTable.size = SymbolTableSize;
    SymbolTable.data = malloc(SymbolTable.size * sizeof(struct SA_pair));


    if (SymbolTable.data == NULL) {
        printf("memory allocation failed");
        return 1;
    }

    // Create an array of instruction mappings
    InstructionMapping mappings[] = {
            {"b", parseBranch},
            {"b.cond", parseBranch},
            {"br", parseBranch},
            {"str", parseLoadStore},
            {"ldr", parseLoadStore},
            {"add", parseDataProcessing},
            {"adds", parseDataProcessing},
            {"sub", parseDataProcessing},
            {"subs", parseDataProcessing},
            {"cmp", parseDataProcessing},
            {"cmn", parseDataProcessing},
            {"neg", parseDataProcessing},
            {"negs", parseDataProcessing},
            {"and", parseDataProcessing},
            {"ands", parseDataProcessing},
            {"bic", parseDataProcessing},
            {"bics", parseDataProcessing},
            {"eor", parseDataProcessing},
            {"orr", parseDataProcessing},
            {"eon", parseDataProcessing},
            {"orn", parseDataProcessing},
            {"tst", parseDataProcessing},
            {"movk", parseDataProcessing},
            {"movn", parseDataProcessing},
            {"movz", parseDataProcessing},
            {"mov", parseDataProcessing},
            {"mvn", parseDataProcessing},
            {"madd", parseDataProcessing},
            {"msub", parseDataProcessing},
            {"mul", parseDataProcessing},
            {"mneg", parseDataProcessing},
            // Add more mappings as needed
    };

    size_t mappingCount = sizeof(mappings) / sizeof(mappings[0]);

    // Example instruction mnemonics
    InstructionIR instruction1;
    instruction1.opcode = "mvn";
    instruction1.operand[0] = "x5";
    instruction1.operand[1] = "x6";

    InstructionParser parser1 = functionClassifier(instruction1, mappings, mappingCount);


    FILE *binaryCode = fopen("code.bin", "wb");
    if (binaryCode == NULL) {
        fprintf(binaryCode, "file unable to open");
        return EXIT_FAILURE;
    }
    parser1(instruction1, binaryCode);

    *opcodeMapping = {
            {"add", 0},
            {"adds", 1 << 29},
            {"sub", 1 << 30},
            {"subs", 3 << 30}
    };


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


static void parser(char *line) {

    char *s = line;
    if (*s == '.') {
        printf("d");
    } else {
        char *temp = line;
        char *end = line;
        while (*end != '\0') {
            temp = end;
            end++;
        }
        if (*temp == ':' && isalpha(*s)) {
            printf("lbl");
        } else {
            tokenizer(line);
        }
    }

}



static InstructionParser functionClassifier(InstructionIR instruction, InstructionMapping* mappings, size_t mapSize) {
    for (size_t i = 0; i < mapSize; i++) {
        if (strcmp(instruction.opcode, mappings[i].mnemonic) == 0) { return  mappings[i].parser; }
    }
    return NULL;
}


static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp(instructionIr.opcode, mapping[i].mnemonic) == 0) {return mapping[i].opcode_bin;}
    }
    exit(1);
}

static void parseArtihmetic(InstructionIR instruction, FILE *file) {
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 1 << 24;
    uint32_t sf = 1 << 31;
    if (*instruction.operand[0] == 'W') {
        sf = 0;
    }
    if (*instruction.operand[2] == '#') {
        char *startptr = instruction.operand[2] + 1;
        char *endptr;
        uint32_t imm12 = strtoul(startptr, &endptr, 10) << 21;
        uint32_t sl = 0;
        if (strcmp(instruction.operand[3], "lsl #12") == 0) {
            sl = 1 << 22;
        }
        uint32_t rn;
        uint32_t rd;
        uint32_t write_val = sf || opcode_bin || data_processing_immediate_code || opi || sl || imm12 || rn || rd;
        size_t written = fwrite(&write_val, sizeof (uint32_t), 1, file);
        if (written != 1) {
            printf("error writing to file");
            exit(1);
        }
    } else {
        //TODO
    }

}



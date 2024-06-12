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

struct dynarray {
    struct SA_pair *data;
    int numItems;
    int size;
};
typedef struct dynarray *dynarray;


typedef enum {instruction, directive, label} LineType;

#define MAX_OPERANDS 4
typedef struct {
    char *opcode;
    char *operand[MAX_OPERANDS];
} InstructionIR;

typedef struct {
    const char* mnemonic;
    uint32_t opcode_bin;
} OpcodeMapping;

typedef void (*InstructionParser)(InstructionIR, char *, OpcodeMapping [], size_t);
static uint32_t getN(const char *opcode);
uint32_t data_processing_immediate_code = 1 << 28;
uint32_t data_processing_register_code = 5 << 25;

typedef struct {
    const char* mnemonic;
    InstructionParser parser;
} InstructionMapping;

struct SA_pair createSA(char *label, int lineNo);





void fileProcessor(char *inputfile, char *outputfile);
void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol);
static void writeToFile(uint32_t write_val, FILE *file);
static void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);



InstructionIR parser(char *line);
char* DataProcessingInstruction(InstructionIR instruction);

// Function pointer type for instruction parsers


// Function declarations
void parseBranch(InstructionIR instruction, FILE *file);
void parseLoadStore(InstructionIR instruction, FILE *file);
void parseDataProcessing(InstructionIR instruction, FILE *file);
static uint32_t getSf(char *reg);

// Parsing functions
void parseBranch(InstructionIR instruction, FILE *file) {
    printf("x");
}

void parseLoadStore(InstructionIR instruction, FILE *file) {
    printf("x");
}

void parseDataProcessing(InstructionIR instruction, FILE *file) {
    printf("x");
}

static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);
void freeTable(dynarray symbolTable);

static void parseTwoOperand(InstructionIR instruction, FILE *file);
static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);
static uint32_t getReg(char *reg);
static void parseArtihmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
bool firstPassFlag = true;
InstructionMapping mappings[] = {
        {"b", parseBranch},
        {"b.cond", parseBranch},
        {"br", parseBranch},
        {"str", parseLoadStore},
        {"ldr", parseLoadStore},
        {"add", parseArtihmetic},
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
dynarray SymbolTable;

OpcodeMapping opcodeMapping[] = {
        {"add", 0},
        {"adds", 1 << 29},
        {"sub", 1 << 30},
        {"subs", 3 << 29},
        {"movn", 0 },
        {"movz", 1<<30},
        {"movk", 3<<29},
        {"madd", 0},
        {"msub", 0},
        {"and", 0},
        {"bic", 0},
        {"orr", 1<<29},
        {"orn", 1<<29},
        {"eor", 1<<30},
        {"eon", 1<<30},
        {"ands", 3<<29},
        {"bics", 3<<29}
};

size_t opcode_msize = sizeof(opcodeMapping) / sizeof(opcodeMapping[0]);
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect number of arguments inputted");
        return 2;
    }

    SymbolTable = malloc(sizeof(struct dynarray));
    if (SymbolTable == NULL) {
        fprintf(stderr, "memory allocation failed");
        return 1;
    }
    SymbolTable->numItems = 0;
    SymbolTable->size = SymbolTableSize;
    SymbolTable->data = malloc(SymbolTable->size * sizeof(struct SA_pair));
    if (SymbolTable->data == NULL) {
        free(SymbolTable);
        fprintf(stderr, "memory allocation failed");
        return 1;
    }


    char *inputFile = argv[0];
    char *outputFile = argv[1];
    fileProcessor(inputFile, outputFile); //Pass 1
    firstPassFlag = false;
    fileProcessor(inputFile, outputFile); //Pass 2

    freeTable(SymbolTable);

   return EXIT_SUCCESS;
};

static void growTable(dynarray mySymbolTable) {
    if (mySymbolTable->numItems == mySymbolTable->size) {
        mySymbolTable->size *= 2;
        mySymbolTable->data = realloc(mySymbolTable->data, mySymbolTable->size * sizeof(struct SA_pair));
        if (mySymbolTable->data == NULL) {
            printf("Symbol table failed to be resized");
        }
    }
}

void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol) {
    mySymbolTable->data[mySymbolTable->numItems] = new_symbol;
    mySymbolTable->numItems++;
}

struct SA_pair createSA(char *label, int lineNo) {
    struct SA_pair newSA;
    newSA.Symbol = label;
    int address = 0x0 + ((lineNo + 1) * 4);
    newSA.address = address;
    return newSA;
}

void freeTable(dynarray symbolTable) {
    free(symbolTable->data);
    free(symbolTable);
}


void fileProcessor(char *inputfile, char *outputfile) {
    FILE *input = fopen(inputfile, "r");
    if (input == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNo = -1;

    while ((read = getline(&line, &len, input)) != -1) {
        // Remove trailing newline character
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        if (isalpha(*line) && line[read - 2] == ':') {
            if (firstPassFlag) {
                struct SA_pair pair = createSA(line, lineNo);
                addToTable(SymbolTable, pair);
            }
        } else {
            if (!firstPassFlag) {
                InstructionIR instruction = parser(line);
                InstructionParser parser = functionClassifier(instruction, mappings, mappingCount);
                (*parser)(instruction, outputfile, opcodeMapping, opcode_msize);
            }
            lineNo++;
        }
    }
    free(line);
    fclose(input);
}



static InstructionIR tokenizer(char instruction[]) {
    char* mne = strtok(instruction, " ");
    char* ops = strtok(NULL, " ");
    printf("%s\n", mne);
    printf("%s\n", ops);
    char* operand;
    char* operands[4];
    InstructionIR new_instruction;
    int operand_count = 0;
    operand = strtok(ops, ",");
    new_instruction.opcode = mne;
    while (operand != NULL && operand_count < 4) {
        operands[operand_count++] = operand;
        operand = strtok(NULL, ",");
    }
    // Print the operands
    for (int i = 0; i < operand_count; i++) {
        new_instruction.operand[i] = operands[i];
        printf("Operand %d: %s\n", i + 1, operands[i]);
    }
    return new_instruction;
}


InstructionIR parser(char *line) {
    if (*line == '.') {
        printf("d");
    } else {
        return tokenizer(line);
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

static void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "wb");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 1 << 24;
    uint32_t sf = getSf(instruction.operand[0]);
    // If statement checks whether instruction is immediate or register
    if (*instruction.operand[2] == '#') {
        char *startptr = instruction.operand[2] + 1;
        printf("%s\n", startptr);
        uint32_t imm12 = (strtoul(startptr, NULL, 16)) << 10;
        printf("%u\n", imm12);
        uint32_t sl = 0;
        if (strcmp(instruction.operand[3], "lsl #12") == 0) {
            sl = 1 << 22;
        }
        uint32_t rn = getReg(instruction.operand[1]) << 5;
        uint32_t rd = getReg(instruction.operand[0]);
        uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | sl | imm12 | rn | rd;
        writeToFile(write_val, file);
        printf("%u", write_val);
    } else {
        uint32_t M = 0;
        uint32_t opr = 1 << 24;
        uint32_t rm = getReg(instruction.operand[2]) << 16;
        uint32_t rn = getReg(instruction.operand[1]) << 5;
        uint32_t rd = getReg(instruction.operand[0]);
        uint32_t operand = 0;
        if (instruction.operand[3] != NULL) {
            char *shift = malloc(3 * sizeof(char));
            strncpy(shift, instruction.operand[3], 3);
            if (strcmp(shift, "lsr") == 0) {
                opr = 10 << 21;
            } else if (strcmp(shift, "asr") == 0) {
                opr = 12 << 23;
            }
            free(shift);
            char *number = instruction.operand[3] + 5;
            operand = (strtoul(number, NULL, 10)) << 10;
            free(number);
        }
        uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | opr | rm | operand | rn | rd;
        writeToFile(write_val, file);
        printf("%u", write_val);
    }
    fclose(file);

}

static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "wb");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 5 << 23;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);
    char *startptr = instruction.operand[1] + (1* sizeof (char));
    uint32_t imm16 = (strtoul(startptr, NULL, 16)) << 5;
    printf("%u\n", imm16);
    uint32_t hw = 0;
    if (instruction.operand[2] != NULL) {
        char *numptr = instruction.operand[2] + (5 * sizeof (char ));
        hw = (strtoul(numptr, NULL, 10)) << 21;
    }
    uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | hw | imm16 | rd;
    size_t written = fwrite(&write_val, sizeof(uint32_t), 1, file);
    if (written != 1) {
        fprintf(file, "error writing to file");
        exit(1);
    }
    printf("%u", write_val);
    fclose(file);
}


static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "wb");
    uint32_t m = 1 << 28;
    uint32_t opr = 1 << 24;
    uint32_t opcode_binary = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t rm = getReg(instruction.operand[2]) << 16;
    uint32_t ra = getReg(instruction.operand[3]) << 10;
    uint32_t rn = getReg(instruction.operand[1]) << 5;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t x = 0;
    if (strcmp(instruction.opcode, "msub") == 0) {
        x = 1 << 15;
    }
    uint32_t write_val = sf | opcode_binary | m | data_processing_register_code | opr | rm | x | ra | rn | rd;
    writeToFile(write_val, file);
    fclose(file);
}

static uint32_t getReg(char *reg) {
    if (reg == NULL) {
        return 0;
    }
    uint32_t rv = strtoul((reg + (1*sizeof (char))), NULL, 10);
    return rv;
}

static uint32_t getSf(char *reg) {
    if (*reg == 'w' || *reg == 'W') {
        return 0;
    } else {
        return 1 << 31;
    }
}

static void writeToFile(uint32_t write_val, FILE *file) {
    size_t written = fwrite(&write_val, sizeof(uint32_t), 1, file);
    if (written != 1) {
        fprintf(file, "error writing to file");
        exit(1);
    }
}

static void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "wb");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t M = 0;
    uint32_t opr = 0;
    uint32_t rm = getReg(instruction.operand[2]) << 16;
    uint32_t rn = getReg(instruction.operand[1]) << 5;
    printf("%u", rn);
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t operand = 0;
    if (instruction.operand[3] != NULL) {
        char *shift = malloc(3 * sizeof(char));
        strncpy(shift, instruction.operand[3], 3);
        if (strcmp(shift, "lsr") == 0) {
            opr = 1 << 22;
        } else if (strcmp(shift, "asr") == 0) {
            opr = 1 << 23;
        }
        free(shift);
        char *number = instruction.operand[3] + 5;
        operand = (strtoul(number, NULL, 10)) << 10;
        free(number);
    }
    uint32_t N = getN(instruction.opcode) << 21;
    uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | N | opr | rm | operand | rn | rd;
    writeToFile(write_val, file);
    printf("%u", write_val);
    fclose(file);
}

static uint32_t getN(const char *opcode) {
    if (strcmp(opcode, "bic") == 0 || strcmp(opcode, "orn") == 0 || strcmp(opcode, "eon") == 0 || strcmp(opcode, "bics") == 0) {
        return 1;
    }
    return 0;
}
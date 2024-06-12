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
#define BranchMappingSize 7
typedef struct {
    char *opcode;
    char *operand[MAX_OPERANDS];
} InstructionIR;

typedef struct {
    const char* mnemonic;
    uint32_t opcode_bin;
} OpcodeMapping;

typedef struct {
    const char* mnemonic;
    uint32_t encoding;
} BranchMapping;

typedef void (*InstructionParser)(InstructionIR, FILE *);

uint32_t data_processing_immediate_code = 1 << 28;
uint32_t data_processing_register_code = 5 << 25;

typedef struct {
    const char* mnemonic;
    InstructionParser parser;
} InstructionMapping;







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
    printf("x");
}

void parseLoadStore(InstructionIR instruction, FILE *file) {
    printf("x");
}

void parseDataProcessing(InstructionIR instruction, FILE *file) {
    printf("x");
}

static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);

static void parseTwoOperand(InstructionIR instruction, FILE *file);
static void parseMultiply(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parser(char *line);
static InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);
static uint32_t getReg(char *reg);
static void parseArtihmetic(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseLoadStoreInstructions(InstructionIR instruction, FILE *file, BranchMapping branchMapping[], size_t opcode_map_size);
static void parseBranchInstructions(InstructionIR instruction, FILE *file, BranchMapping branchMapping[], size_t opcode_map_size);
static void parseMove(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size);

void printBinary(uint32_t value) {
    for (int i = 31; i >= 0; i--) {
        putchar((value & (1 << i)) ? '1' : '0');
        if (i % 4 == 0) {  // Optional: add a space every 4 bits for readability
            putchar(' ');
        }
    }
    putchar('\n');
}

int main(int argc, char **argv) {
    struct list SymbolTable;
    uint32_t bStart = 0 << 4;
    printBinary(bStart);
    uint32_t brStart = 54815 << 16;
    printBinary(brStart);
    printf("bStart in hex: 0x%08X\n", bStart);
    SymbolTable.numItems = 0;
    SymbolTable.size = SymbolTableSize;
    SymbolTable.data = malloc(SymbolTable.size * sizeof(struct SA_pair));


    if (SymbolTable.data == NULL) {
        printf("memory allocation failed");
        return 1;
    }

    OpcodeMapping opcodeMapping[] = {
            {"add", 0},
            {"adds", 1 << 29},
            {"sub", 1 << 30},
            {"subs", 3 << 29}
    };

    BranchMapping branchMapping[] = {
            {"eq", 0},
            {"ne", 1},
            {"ge", 10},
            {"lt", 11},
            {"gt", 12},
            {"le", 13},
            {"al", 14}
    };

    size_t opcode_msize = sizeof(opcodeMapping) / sizeof(opcodeMapping[0]);

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


    FILE *binaryCode = fopen("code.bin", "wb");
    if (binaryCode == NULL) {
        fprintf(binaryCode, "file unable to open");
        return EXIT_FAILURE;
    }
    InstructionIR instruction2;
    instruction2.opcode = "sub";
    instruction2.operand[0] = "w15";
    instruction2.operand[1] = "w0";
    instruction2.operand[2] = "#0x5a0";
    instruction2.operand[3] = "lsl #12";
    parseArtihmetic(instruction2, binaryCode, opcodeMapping, opcode_msize);

    InstructionIR instruction3;
    instruction3.opcode = "br";
    instruction3.operand[0] = "x15";
    parseBranchInstructions(instruction3, binaryCode, branchMapping, opcode_msize);

    InstructionIR instruction4;
    instruction4.opcode = "ldr";
    instruction4.operand[0] = "x15";
    instruction4.operand[1] = "[x3, x17]";
    parseLoadStoreInstructions(instruction4, binaryCode, branchMapping, opcode_msize);
    fclose(binaryCode);
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

static void parseArtihmetic(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 1 << 24;
    uint32_t sf = 1 << 31;
    if (*instruction.operand[0] == 'w' || *instruction.operand[0] == 'W') {
        sf = 0;
    }
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
        size_t written = fwrite(&write_val, sizeof (uint32_t), 1, file);
        if (written != 1) {
            printf("error writing to file");
            exit(1);
        }
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
        size_t written = fwrite(&write_val, sizeof (uint32_t), 1, file);
        if (written != 1) {
            printf("error writing to file");
            exit(1);
        }
        printf("%u", write_val);
    }

}
static uint32_t getEncoding(const char* mnemonic, BranchMapping branchMapping[]) {
    for (int i = 0; i < BranchMappingSize; i++) {
        if (strcmp(branchMapping[i].mnemonic, mnemonic) == 0) {
            return branchMapping[i].encoding;
        }
    }
    exit(1);
}

static void parseBranchInstructions(InstructionIR instruction, FILE *file, BranchMapping branchMapping[], size_t opcode_map_size) {
    uint32_t bStart = 5 << 26;
    uint32_t brStart = 54815 << 16;
    uint32_t bCStart = 21 << 26;
    if (strcmp(instruction.opcode, "br") == 0) {
        int regNumber;
        char *endptr;
        regNumber = strtol(&instruction.operand[0][1], &endptr, 10);
        if (*endptr != '\0') {
            printf("Conversion error occurred\n");
        }
//        printf((const char *) regNumber);
        uint32_t regBin = regNumber << 5;
        uint32_t finalVal = brStart | regBin;
        printf("\nfinal val in binary:\n");
        printBinary(finalVal);
        printf("final val in hex: 0x%08X\n", finalVal);
    } else if (instruction.opcode[1] == '.') {
        size_t length = strlen(instruction.opcode) - 2; // Subtracting 2 for 'b.' prefix
        char *suffix = (char*)malloc(length + 1); // +1 for the null terminator
        if (suffix != NULL) {
            strcpy(suffix, &instruction.opcode[2]); // Copy the substring starting from the third character
        }
        uint32_t cond = getEncoding(suffix, branchMapping);
        uint32_t finalVal = bCStart | cond;
    }
}

char** allocateRegisters(InstructionIR instruction) {
    char** registers = malloc(2 * sizeof(char*));
    if (registers == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // Initialize both pointers to NULL
    registers[0] = NULL;
    registers[1] = NULL;

    // Parse the instruction to extract register(s)
    char* token = strtok(instruction.operand[1], ", []!");
    if (token != NULL) {
        registers[0] = strdup(token); // Allocate memory and copy the string
        token = strtok(NULL, ", []"); // Move to the next token
        if (token != NULL) {
            registers[1] = strdup(token); // Allocate memory and copy the string
        }
    }
    return registers;
}

// single data transfer is str or ldr
// load literal just ldr
static void parseLoadStoreInstructions(InstructionIR instruction, FILE *file, BranchMapping branchMapping[], size_t opcode_map_size) {
    int rtNumber;
    char *endptr;
    uint32_t sf;
    rtNumber = strtol(&instruction.operand[0][1], &endptr, 10);
    if (*endptr != '\0') {
        printf("Conversion error occurred\n");
    }
    uint32_t rt = rtNumber;
    if (instruction.operand[0][0] == 'w') {
        uint32_t sf = 0;
    } else {
        uint32_t sf = 1 << 30;
    }
    if (instruction.operand[1][0] == '[') {
        uint32_t l;
        int length = strlen(instruction.operand[1]);
        uint32_t firstBit = 1 << 31;
        uint32_t otherBits = 7 << 27;
        if (instruction.opcode[0] == 'l') {
            uint32_t l = 1 << 22;
        } else {
            uint32_t l = 0 << 22;
        }
        int xnNumber;
        char *endptr2;
        char** memoryRegisters = allocateRegisters(instruction);
        xnNumber = strtol(memoryRegisters[0], &endptr2, 10);
        if (*endptr2 != '\0') {
            printf("Conversion error occurred\n");
        }
        uint32_t xn = xnNumber << 5;
        if (instruction.operand[1][length -1] == '!') {
            uint32_t u = 0;
            uint32_t i = 1 << 11;
            uint32_t endRegBit = 1 << 10;
        } else if (instruction.operand[2] != NULL){
            uint32_t u = 0;
            uint32_t endRegBit = 1 << 10;
        } else if (memoryRegisters[1] != NULL && memoryRegisters[1][0] == 'x') {
            uint32_t u = 0;
            uint32_t firstRegBit = 1 << 21;
            uint32_t regOtherBits = 13 << 11;
            int xmNumber;
            char *endptr3;
            xmNumber = strtol(memoryRegisters[1], &endptr3, 10);
            if (*endptr3 != '\0') {
                printf("Conversion error occurred\n");
            }
            uint32_t xm = xmNumber << 16;
            uint32_t finalVal = firstBit | sf | otherBits | u | l | firstRegBit | xm | regOtherBits | xn | rt;
            printf("\nfinal val 2 in binary:\n");
            printBinary(finalVal);
            printf("final val 2 in hex: 0x%08X\n", finalVal);
        } else {
            uint32_t u = 1 << 24;
        }
        free(memoryRegisters);

    } else {
        uint32_t firstBit = 0 << 31;
        uint32_t otherBits = 3 << 27;
        //when its ldr <literal> so either label or #N
    }


}


static void parseWideMove(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 5 << 23;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = 1 << 31;
    if (*instruction.operand[0] == 'w' || *instruction.operand[0] == 'W') {
        sf = 0;
    }
    char *startptr = instruction.operand[0] + (1* sizeof (char));
    uint32_t imm16 = (strtoul(startptr, NULL, 16)) << 5;
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

}



static void parseMultiply(InstructionIR instruction, FILE *file, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    //TODO
}

static uint32_t getReg(char *reg) {
    uint32_t rv = strtoul((reg + (1*sizeof (char))), NULL, 10);
    return rv;
}



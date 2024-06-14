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

typedef void (*InstructionParser)(InstructionIR, char *, OpcodeMapping [], size_t);
static uint32_t getN(const char *opcode);

uint32_t data_processing_immediate_code = 1 << 28;
uint32_t data_processing_register_code = 5 << 25;

typedef struct {
    const char* mnemonic;
    InstructionParser parser;
} InstructionMapping;

struct SA_pair createSA(char *label, int lineNo);

static char * trim_leading_spaces(char *str);



void fileProcessor(char *inputfile, char *outputfile);
void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol);
static void writeToFile(uint32_t write_val, FILE *file);
static void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static uint32_t getimmm(char *num);


InstructionIR parser(char *line);
char* DataProcessingInstruction(InstructionIR instruction);

// Function pointer type for instruction parsers



static uint32_t getSf(char *reg);



static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);
void freeTable(dynarray symbolTable);

static void parseTwoOperand(InstructionIR instruction, FILE *file);
static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);
static uint32_t getReg(char *reg);
static void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
static void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
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


static void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
bool firstPassFlag = true;
InstructionMapping mappings[] = {
        {"b",      parseBranchInstructions},
        {"b.cond", parseBranchInstructions},
        {"br",     parseBranchInstructions},
        {"str",    parseLoadStoreInstructions},
        {"ldr",    parseLoadStoreInstructions},
        {"add",    parseArithmetic},
        {"adds",   parseArithmetic},
        {"sub",    parseArithmetic},
        {"subs",   parseArithmetic},
        {"cmp",    parseArithmetic},
        {"cmn",    parseArithmetic},
        {"neg",    parseArithmetic},
        {"negs",   parseArithmetic},
        {"and",    parseLogic},
        {"ands",   parseLogic},
        {"bic",    parseLogic},
        {"bics", parseLogic},
        {"eor", parseLogic},
        {"orr", parseLogic},
        {"eon", parseLogic},
        {"orn", parseLogic},
        {"tst", parseTst},
        {"movk", parseWideMove},
        {"movn", parseWideMove},
        {"movz", parseWideMove},
        {"mov", parseLogic},
        {"mvn", parseLogic},
        {"madd", parseMultiply},
        {"msub", parseMultiply},
        {"mul", parseMultiply},
        {"mneg", parseMultiply},
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
        {"bics", 3<<29},
        {"mneg", 0},
        {"mul", 0},
        {"tst", 3<<29}

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
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "incorrect number of arguments inputted\n");
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




    char *inputFile = argv[1];
    char *outputFile = argv[2];
    FILE *file = fopen(outputFile, "wb");
    fclose(file);
    fileProcessor(inputFile, outputFile); //Pass 1
    firstPassFlag = false;
    fileProcessor(inputFile, outputFile); //Pass 2
    
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
        if (read == 1) {
            continue;
        }
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
                InstructionIR instruction = {0};
                instruction = parser(line);
                InstructionParser parser = functionClassifier(instruction, mappings, mappingCount);
                (*parser)(instruction, outputfile, opcodeMapping, opcode_msize);
            }
            lineNo++;
        }
    }
    fclose(input);
}



static InstructionIR tokenizer(char instruction[]) {
    char* mne = strtok(instruction, " ");
    char* ops = strtok(NULL, "");
    printf("%s\n", mne);
    printf("%s\n", ops);
    char* operand;
    char* operands[4];
    InstructionIR new_instruction = {0};
    int operand_count = 0;
    operand = strtok(ops, ",");
    printf("%s\n", operand);
    new_instruction.opcode = mne;
    while (operand != NULL && operand_count < 4) {
        operands[operand_count++] = operand;
        operand = strtok(NULL, ",");
    }

    // Print the operands
    for (int i = 0; i < operand_count; i++) {
        new_instruction.operand[i] = trim_leading_spaces(operands[i]);
        printf("Operand %d: %s\n", i + 1, operands[i]);
    }
    return new_instruction;
}

static char * trim_leading_spaces(char *str) {
    int index = 0;
    while(isspace(str[index])) {
        index++;
    }
    char *newstr = str + index;
    return newstr;
}


InstructionIR parser(char *line) {
    if (*line == '.') {
        printf("d");
    } else {
        printf("this is line %s\n", line);
        return tokenizer(line);
    }
}





static InstructionParser functionClassifier(InstructionIR instruction, InstructionMapping* mapping, size_t mapSize) {
    for (size_t i = 0; i < mapSize; i++) {
        if (strcmp(instruction.opcode, mappings[i].mnemonic) == 0) { return  mapping[i].parser; }
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
    FILE *file = fopen(output, "ab");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 1 << 24;
    uint32_t sf = getSf(instruction.operand[0]);
    printf("this is operand 2%s\n", (instruction.operand[2]));
    // If statement checks whether instruction
    if (*instruction.operand[2] == '#') {
        char *startptr = instruction.operand[2] + 1;
        printf("%s\n", startptr);
        uint32_t imm12 = (getimmm(startptr)) << 10;
        printf("%u\n", imm12);
        uint32_t sl = 0;
        if (instruction.operand[3] != NULL) {
            if (strcmp(instruction.operand[3], "lsl #12") == 0) {
                sl = 1 << 22;
            }
        }
        uint32_t rn = getReg(instruction.operand[1]) << 5;
        uint32_t rd = getReg(instruction.operand[0]);
        uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | sl | imm12 | rn | rd;
        writeToFile(write_val, file);
        printf("%u\n", write_val);
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
                opr = 12 << 21;
            }
            free(shift);
            char *number = instruction.operand[3] + 5;
            printf("number : %s\n", number);
            operand = getimmm(number) << 10;
            printf("%i\n", operand);
        }
        uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | opr | rm | operand | rn | rd;
        writeToFile(write_val, file);
    }
    fclose(file);
}
static uint32_t getEncoding(const char* mnemonic) {
    for (int i = 0; i < BranchMappingSize; i++) {
        if (strcmp(branchMapping[i].mnemonic, mnemonic) == 0) {
            return branchMapping[i].encoding;
        }
    }
    exit(1);
}

static void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "wb");
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
        uint32_t cond = getEncoding(suffix);
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
static void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
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

static uint32_t getimmm(char *num) {
    if (strlen(num) >= 2) {
        if (*(num+1) == 'x') {
            return strtoul(num+2, NULL, 16);
        }
    }
    return strtoul(num, NULL, 10);
}

static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 5 << 23;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);
    char *startptr = instruction.operand[1] + (1* sizeof (char));
    uint32_t imm16 = getimmm(startptr);
    printf("%u\n", imm16);
    imm16 = imm16 << 5;
    uint32_t hw = 0;
    if (instruction.operand[2] != NULL) {
        char *numptr = instruction.operand[2] + (5 * sizeof (char ));
        hw = (strtoul(numptr, NULL, 10)) / 16;
        printf("%u\n", hw);
        hw = hw << 21;
    }
    uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | hw | imm16 | rd;
    size_t written = fwrite(&write_val, sizeof(uint32_t), 1, file);
    if (written != 1) {
        fprintf(file, "error writing to file");
        exit(1);
    }
    printf("%u\n", write_val);
    fclose(file);
}



static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    uint32_t m = 1 << 28;
    uint32_t opr = 1 << 24;
    uint32_t opcode_binary = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t rm = getReg(instruction.operand[2]) << 16;
    uint32_t ra = getReg(instruction.operand[3]) << 10;
    uint32_t rn = getReg(instruction.operand[1]) << 5;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t x = 0;
    if (strcmp(instruction.opcode, "msub") == 0 || strcmp(instruction.opcode, "mneg") == 0) {
        x = 1 << 15;
    }
    uint32_t write_val = sf | opcode_binary | m | data_processing_register_code | opr | rm | x | ra | rn | rd;
    writeToFile(write_val, file);
    fclose(file);

}

static uint32_t getReg(char *reg) {
    if (reg == NULL) {
        return 31;
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
    FILE *file = fopen(output, "ab");
    if ((strcmp(instruction.opcode, "and") == 0) && (strcmp(instruction.operand[0], "x0") == 0) && (strcmp(instruction.operand[1], "x0") == 0) && (strcmp(instruction.operand[2], "x0") == 0)) {
        writeToFile(2315255808, file);
        return;
    }
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t M = 0;
    uint32_t opr = 0;
    uint32_t rm = getReg(instruction.operand[2]) << 16;
    uint32_t rn = getReg(instruction.operand[1]) << 5;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t operand = 0;
    if (instruction.operand[3] != NULL) {
        char *shift = malloc(3 * sizeof(char));
        strncpy(shift, instruction.operand[3], 3);
        if (strcmp(shift, "lsr") == 0) {
            opr = 1 << 22;
        } else if (strcmp(shift, "asr") == 0) {
            opr = 1 << 23;
        } else if (strcmp(shift, "ror") == 0) {
            opr = 3 << 22;
        }
        free(shift);
        char *number = instruction.operand[3] + 5;
        operand = getimmm(number) << 10;
    }
    uint32_t N = getN(instruction.opcode) << 21;
    uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | N | opr | rm | operand | rn | rd;
    writeToFile(write_val, file);
    printf("%u", write_val);
    fclose(file);
}

static void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t M = 0;
    uint32_t opr = 0;
    uint32_t rd = getReg(NULL); //gets the zero register 
    uint32_t rn = getReg(instruction.operand[0]) << 5; 
    uint32_t rm = getReg(instruction.operand[1]) << 16;
    uint32_t operand = 0;
    if (instruction.operand[2] != NULL) {
        char *shift = malloc(3 * sizeof(char));
        strncpy(shift, instruction.operand[2], 3);
        if (strcmp(shift, "lsr") == 0) {
            opr = 1 << 22;
        } else if (strcmp(shift, "asr") == 0) {
            opr = 1 << 23;
        } else if (strcmp(shift, "ror") == 0) {
            opr = 3 << 22;
        }
        free(shift);
        char *number = instruction.operand[2] + 5;
        operand = getimmm(number) << 10;
    }
    uint32_t write_val = sf | opcode_bin | M | data_processing_register_code  | opr | rm | operand | rn | rd;
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
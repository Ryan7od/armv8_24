#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdint.h>

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



//Function initialisation
struct SA_pair createSA(char *label, int lineNo);
static char * trim_leading_spaces(char *str);
void fileProcessor(char *inputfile, char *outputfile);
void addToTable(dynarray mySymbolTable, struct SA_pair new_symbol);
static void writeToFile(uint32_t write_val, FILE *file);
static void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static uint32_t getimmm(char *num);
InstructionIR parser(char *line);
char* DataProcessingInstruction(InstructionIR instruction);
static uint32_t getSf(char *reg);
static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size);
void freeTable(dynarray symbolTable);
static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);
static uint32_t getReg(char *reg);
static void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
static void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
static void parseDirective(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
static void growTable(dynarray mySymbolTable);



bool firstPassFlag = true;
InstructionMapping mappings[] = { //Table mapping instruction opcodes to their function
        {"b",      parseBranchInstructions},
        {"b.eq", parseBranchInstructions},
        {"b.ne", parseBranchInstructions},
        {"b.ge", parseBranchInstructions},
        {"b.lt", parseBranchInstructions},
        {"b.gt", parseBranchInstructions},
        {"b.le", parseBranchInstructions},
        {"b.al", parseBranchInstructions},
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
        {".int", parseDirective},
        // Add more mappings as needed
};
size_t mappingCount = sizeof(mappings) / sizeof(mappings[0]);
dynarray SymbolTable;

OpcodeMapping opcodeMapping[] = { //instruction mapping opcode mnemonic to number with bit mask  
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
        {"tst", 3<<29},
        {"mov", 1<<29},
        {"cmp", 3<<29},
        {"cmn", 1<<29}

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


    // Assign a value to the address field

    freeTable(SymbolTable);

   return EXIT_SUCCESS;
};

static void growTable(dynarray mySymbolTable) { //helper function to increase size of symbol table if needed
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

int PC = 0; //global program counter variable
int lineNo = 0; //global line number
void fileProcessor(char *inputfile, char *outputfile) {
    FILE *input = fopen(inputfile, "r");
    if (input == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, input)) != -1) {
        if (read == 1) {
            continue;
        }
        // Remove trailing newline character
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') {
            trimmed++;
        }
        // Check if the line is empty or is a label
        if (!(trimmed[0] == '\0' || trimmed[0] == '\n' || (strchr(trimmed, ':') != NULL))) {
            // Increment PC only if it's not an empty line or a label
            lineNo++;
        }
        
        if ((line[read - 2] == ':') || (line[read - 3] == ':')) {
            if (firstPassFlag) {
                char *colon_pos = strchr(line, ':');
                size_t lengthLabel = colon_pos - line;
                char *newLine = (char *)malloc((lengthLabel + 1) * sizeof(char));
                strncpy(newLine, line, lengthLabel);
                newLine[lengthLabel] = '\0';

                struct SA_pair pair = createSA(newLine, lineNo); 
                addToTable(SymbolTable, pair); //creates new symbol entry when a label is seen on first pass
            }
        } else {
            if (!firstPassFlag) {
                InstructionIR instruction = {0};
                instruction = parser(line); //instruction parsed into IR 
                InstructionParser parser = functionClassifier(instruction, mappings, mappingCount); //gets function for the instruction 
                (*parser)(instruction, outputfile, opcodeMapping, opcode_msize); //calls function 
                char *trimmed = line;
                while (*trimmed == ' ' || *trimmed == '\t') {
                    trimmed++;
                }

                // Check if the line is empty or is a label
                if (!(trimmed[0] == '\0' || trimmed[0] == '\n' || (strchr(trimmed, ':') != NULL))) {
                    // Increment PC only if it's not an empty line or a label
                    PC++;
                }

            }
        }
        printf("line processed: %s\n", line);
        printf("PC new 2: %d\n", PC);
        printf("lineNo: %d\n", lineNo);
    }
    fclose(input);
}


static InstructionIR tokenizer(char instruction[]) {
    char* mne = strtok(instruction, " ");
    char* ops = strtok(NULL, "");
    
    printf("Mnemonic: %s\n", mne);
    printf("Operands string: %s\n", ops);

    InstructionIR new_instruction = {0};
    new_instruction.opcode = mne;

    char* operand;
    char* operands[4];
    int operand_count = 0;

    operand = strtok(ops, ",");
    while (operand != NULL && operand_count < 4) {
        char* complete_operand = strdup(operand);
        
        if (strchr(operand, '[') != NULL && strchr(operand, ']') == NULL) {
            char* next_part = strtok(NULL, ","); 
            while (next_part != NULL && strchr(next_part, ']') == NULL) {
                complete_operand = realloc(complete_operand, strlen(complete_operand) + strlen(next_part) + 2);
                strcat(complete_operand, ",");
                strcat(complete_operand, next_part);
                next_part = strtok(NULL, ",");
            }
            if (next_part != NULL) {
                complete_operand = realloc(complete_operand, strlen(complete_operand) + strlen(next_part) + 2);
                strcat(complete_operand, ",");
                strcat(complete_operand, next_part);
            }
        }

        operands[operand_count++] = trim_leading_spaces(complete_operand); //setting operands[i] to current operand
        operand = strtok(NULL, ",");
    }

    for (int i = 0; i < operand_count; i++) {
        new_instruction.operand[i] = operands[i]; //setting operand[i] in IR to corresponding operand in list 
        printf("Operand %d: %s\n", i + 1, new_instruction.operand[i]);
    }

    return new_instruction;
}


static int getAddress(dynarray symbolTable, const char *label) { //helper function to return the address of a label 
    // Iterate through the list of symbol-address pairs
    for (int i = 0; i < symbolTable->numItems; ++i) { 
        // Compare the current symbol with the input label
        if (strcmp(symbolTable->data[i].Symbol, label) == 0) {
            // If they match, return the corresponding address
            return symbolTable->data[i].address;
        }
    }
    exit(1);
}

static char * trim_leading_spaces(char *str) {
    int index = 0;
    //iterates through list until a space character is not seen 
    while(isspace(str[index])) { 
        index++;
    }
    char *newstr = str + index;
    return newstr; //returns string without leading spaces
}



InstructionIR parser(char *line) {
    return tokenizer(line);
}


static void parseDirective(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    printf(".int parsing");
    int address = (int)strtol(instruction.operand[0], NULL, 0);
    uint32_t write_val = address;
    //writes the number on the right of the directive to the file 
    writeToFile(write_val, file);
    printf("%u\n", write_val);
    fclose(file);
}



static InstructionParser functionClassifier(InstructionIR instruction, InstructionMapping* mapping, size_t mapSize) {
    //iterates through the function map and returns the matching function to the opcode 
    for (size_t i = 0; i < mapSize; i++) {
        if (strcmp(instruction.opcode, mappings[i].mnemonic) == 0) { return  mapping[i].parser; }
    }
    return NULL;
}


static uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size) {
    //iterates through the opcode map and returns the matching opcode number to the opcode mnemonic  
    for (size_t i = 0; i < size; i++) {
        if (strcmp(instructionIr.opcode, mapping[i].mnemonic) == 0) {return mapping[i].opcode_bin;}
    }
    exit(1);
}

static void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //sets opcode, opi and sf (standard for all arithmetic instructions)
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 1 << 24;
    uint32_t sf = getSf(instruction.operand[0]);

    char *op1 = instruction.operand[0];
    char *op2 = instruction.operand[1];
    char *op3 = instruction.operand[2]; 
    char *op4 = instruction.operand[3];

    //opcodes must be shuffled if, is a cmp or cmn alias 
    if ((strcmp(instruction.opcode, "cmn") == 0) || strcmp(instruction.opcode, "cmp") == 0) {
        op1 = NULL;
        op2 = instruction.operand[0];
        op3 = instruction.operand[1];
        op4 = instruction.operand[2];
    }

    // If statement checks whether instruction is immediate 
    if (*op3 == '#') {
        //gets number after the '#'
        char *startptr = op3 + 1;
        
       
        uint32_t imm12 = (getimmm(startptr)) << 10;
       
        uint32_t sl = 0;

        if (op4 != NULL) {
            //sets sjift left bit if there is a shift left 
            if (strcmp(op4, "lsl #12") == 0) {
                sl = 1 << 22;
            }
        }


        uint32_t rn = getReg(op2) << 5;
        uint32_t rd = getReg(op1);

        uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | sl | imm12 | rn | rd;
        writeToFile(write_val, file);
    } else {
        //sets all standard bits for register arithmetic instructions 
        uint32_t M = 0;
        uint32_t opr = 1 << 24;
        uint32_t rm = getReg(op3) << 16;
        uint32_t rn = getReg(op2) << 5;
        uint32_t rd = getReg(op1);
        uint32_t operand = 0;


        //if statement checks for a shift 
        if (op4 != NULL) {
            
            char *shift = malloc(4 * sizeof(char));
            strncpy(shift, op4, 3);
            shift[3] = '\0';

            //type of shift stored in opr
            if (strcmp(shift, "lsr") == 0) {
                opr = 10 << 21;
            } else if (strcmp(shift, "asr") == 0) {
                printf("asr\n");
                opr = 12 << 21;
            }

            free(shift);
            
            //shift value stored in operand 
            char *number = op4 + 5;
            operand = getimmm(number) << 10;
        }
        uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | opr | rm | operand | rn | rd;
        writeToFile(write_val, file);
    }
    fclose(file);
}

//Helper function to get the binary encoding of conditional branches
static uint32_t getEncoding(const char* mnemonic) {
    for (int i = 0; i < BranchMappingSize; i++) {
        if (strcmp(branchMapping[i].mnemonic, mnemonic) == 0) {
            return branchMapping[i].encoding;
        }
    }
    exit(1);
}


void printBinary(uint32_t n) {
    // Define the number of bits in uint32_t
    int bits = sizeof(uint32_t) * 8;

    // Iterate through each bit
    for (int i = bits - 1; i >= 0; i--) {
        // Print the corresponding bit
        printf("%u", (n >> i) & 1);

        // Add a space every 4 bits for readability
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

static void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //set standard branch binary numbers 
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

        uint32_t regBin = regNumber << 5;
        uint32_t write_val = brStart | regBin;

        writeToFile(write_val, file);

    } else if (instruction.opcode[1] == '.') {

        size_t length = strlen(instruction.opcode) - 2; // Subtracting 2 for 'b.' prefix
        char *suffix = (char*)malloc(length + 1); // +1 for the null terminator

        if (suffix != NULL) {
            strcpy(suffix, &instruction.opcode[2]); // Copy the substring starting from the third character
        }

        uint32_t cond = getEncoding(suffix);

        int labelAddress = getAddress(SymbolTable, instruction.operand[0]);
        int offset = labelAddress - PC;

        uint32_t simm19;
        if (offset < 0) {
            simm19 = (uint32_t)(offset & 0x7FFFF) << 5;
        } else {
            simm19 = offset << 5;
        }

        uint32_t write_val = bCStart | simm19 | cond;
        writeToFile(write_val, file);

    } else {
        int labelAddress = getAddress(SymbolTable, instruction.operand[0]);
        int offset = labelAddress - PC;

        uint32_t simm26;
        if (offset < 0) {
            simm26 = (uint32_t)(offset & 0x7FFFF);
        } else {
            simm26 = offset;
        }

        uint32_t write_val = bStart | simm26;
        writeToFile(write_val, file);
    }
    fclose(file);
}

char** allocateRegisters(InstructionIR instruction) {
    char* token;
    const char *delimiters = ", []!";
    char operand[50];
    strncpy(operand, instruction.operand[1], sizeof(operand));
    operand[sizeof(operand) - 1] = '\0';

    char** registers = malloc(2 * sizeof(char*));
    if (registers == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // Initialize both pointers to NULL
    registers[0] = NULL;
    registers[1] = NULL;

    // Parse the instruction to extract register(s)
    token = strtok(operand, delimiters);
    registers[0] = malloc((strlen(token) + 1) * sizeof(char));

    if (token != NULL) {
        strcpy(registers[0],token);
        token = strtok(NULL, delimiters);// Move to the next token
        if (token != NULL) {
            registers[1] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(registers[1],token);
        }
    }


    return registers;
}



// single data transfer is str or ldr
// load literal just ldr
static void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //Initialising values
    int rtNumber;
    char *endptr;
    uint32_t sf;


    rtNumber = strtol(&instruction.operand[0][1], &endptr, 10);

    if (*endptr != '\0') {
        printf("Conversion error occurred\n");
    }

    uint32_t rt = rtNumber;
    if (instruction.operand[0][0] == 'w') {
        sf = 0;
    } else {
        sf = 1 << 30;
    }

    //Checks if ldr contains [] referencing
    if (instruction.operand[1][0] == '[') {
        uint32_t l;
        int length = strlen(instruction.operand[1]);

        uint32_t firstBit = 1 << 31;
        uint32_t otherBits = 7 << 27;

        if (instruction.opcode[0] == 'l') {
            l = 1 << 22;
        } else {
            l = 0 << 22;
        }

        int xnNumber;
        char** memoryRegisters = allocateRegisters(instruction);
        xnNumber = strtol(memoryRegisters[0] + 1, NULL, 10);
        uint32_t xn = xnNumber << 5;

        if (instruction.operand[1][length -1] == '!') {
            uint32_t u = 0;
            uint32_t i = 1 << 11;
            uint32_t endRegBit = 1 << 10;

            int signedNumber;
            signedNumber = strtol(memoryRegisters[1] + 1, NULL, 0);

            uint32_t simm9 = signedNumber << 12;
            uint32_t write_val = firstBit | sf | otherBits | u | l | simm9 | i | endRegBit | xn | rt;
            writeToFile(write_val, file);

        } else if (instruction.operand[2] != NULL && memoryRegisters[1] == NULL){
            uint32_t u = 0;
            uint32_t endRegBit = 1 << 10;

            int signedNumber;
            signedNumber = strtol(instruction.operand[2] + 1, NULL, 0);

            uint32_t simm9;
            if (signedNumber < 0) {
                simm9 = (uint32_t)(signedNumber & 0x1FF) << 12;
            } else {
                simm9 = signedNumber << 12;
            }

            uint32_t write_val = firstBit | sf | otherBits | u | l | simm9 | endRegBit | xn | rt;
            writeToFile(write_val, file);

        } else if (memoryRegisters[1] != NULL && memoryRegisters[1][0] == 'x') {
            uint32_t u = 0;
            uint32_t firstRegBit = 1 << 21;
            uint32_t regOtherBits = 13 << 11;

            int xmNumber = strtol(memoryRegisters[1] + 1, NULL, 10);
            uint32_t xm = xmNumber << 16;

            uint32_t write_val = firstBit | sf | otherBits | u | l | firstRegBit | xm | regOtherBits | xn | rt;
            writeToFile(write_val, file);
        } else {
            int offsetNum;
            if (memoryRegisters[1] != NULL) {
                int unsignedNumber;
                unsignedNumber = strtol(memoryRegisters[1] + 1, NULL, 0);
                if (sf == 0) {
                    offsetNum = unsignedNumber / 4;
                } else {
                    offsetNum = unsignedNumber /8;
                }
            } else {
                offsetNum = 0;
            }

            uint32_t offset = offsetNum << 10;
            uint32_t u = 1 << 24;

            uint32_t write_val = firstBit | sf | otherBits | u | l | offset | xn | rt;
            writeToFile(write_val, file);
        }
        free(memoryRegisters);

    } else {
        uint32_t simm19;
        uint32_t otherBits = 3 << 27;

        if (instruction.operand[1][0] == '#') {
            int signedNumber;
            signedNumber = strtol(instruction.operand[1] + 1, NULL, 0);
            simm19 = signedNumber << 5;
        } else {

            int labelAddress = getAddress(SymbolTable, instruction.operand[1]);
            int offset = labelAddress - PC;

            if (offset < 0) {
                simm19 = (uint32_t)(offset & 0x7FFFF) << 5;
            } else {
                simm19 = offset << 5;
            }
        }
        uint32_t write_val = sf | otherBits | simm19 | rt;
        writeToFile(write_val, file);
    }
    fclose(file);
}

//Helper function to get an immediate value 
static uint32_t getimmm(char *num) {
    if (strlen(num) >= 2) {
        //if statement checks whether number is hex or decimal 
        if (*(num+1) == 'x') {
            return strtoul(num+2, NULL, 16);
        }
    }
    return strtoul(num, NULL, 10);
}

static void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //getting the standard binary numbers for wide move instructions 
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t opi = 5 << 23;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);



    //setting the startptr to point to the immediate value in the instruction 
    char *startptr = instruction.operand[1] + (1* sizeof (char));
    uint32_t imm16 = getimmm(startptr);
    imm16 = imm16 << 5;


    //gets the shift value divided by 16
    uint32_t hw = 0;
    if (instruction.operand[2] != NULL) {
        char *numptr = instruction.operand[2] + (5 * sizeof (char ));
        hw = (strtoul(numptr, NULL, 10)) / 16;
        hw = hw << 21;
    }

    uint32_t write_val = sf | opcode_bin | data_processing_immediate_code | opi | hw | imm16 | rd;
    writeToFile(write_val, file);
    fclose(file);
}



static void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //setting standard bits for all multiply instructions
    uint32_t m = 1 << 28;
    uint32_t opr = 1 << 24;
    uint32_t opcode_binary = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t rm = getReg(instruction.operand[2]) << 16;
    uint32_t ra = getReg(instruction.operand[3]) << 10;
    uint32_t rn = getReg(instruction.operand[1]) << 5;
    uint32_t rd = getReg(instruction.operand[0]);
    uint32_t sf = getSf(instruction.operand[0]);

    
    //setting negative bit 
    uint32_t neg = 0;
    if (strcmp(instruction.opcode, "msub") == 0 || strcmp(instruction.opcode, "mneg") == 0) {
        neg = 1 << 15;
    }

    uint32_t write_val = sf | opcode_binary | m | data_processing_register_code | opr | rm | neg | ra | rn | rd;
    writeToFile(write_val, file);
    fclose(file);

}

//Helper function to return the register 
static uint32_t getReg(char *reg) {
    //returns 31 for the zero register
    if (reg == NULL || (strcmp(reg+1, "zr") == 0)) {
        return 31;
    }
    uint32_t rv = strtoul((reg + (1*sizeof (char))), NULL, 10);
    return rv;
}

//Helper function to return the significant bit 
static uint32_t getSf(char *reg) {
    //significant bit 0 if w register and 1 otherwise 
    if (*reg == 'w' || *reg == 'W') {
        return 0;
    } else {
        return 1 << 31;
    }
}

//helper function which writes to file
static void writeToFile(uint32_t write_val, FILE *file) {
    size_t written = fwrite(&write_val, sizeof(uint32_t), 1, file);

    //if nothing is written an error occurs
    if (written != 1) {
        fprintf(file, "error writing to file");
        exit(1);
    }
}

//helper function which gets the opr in a shift
static uint32_t getOpr(char *operand) {

    uint32_t opr = 0;
    char *shift = malloc(4 * sizeof(char));
    strncpy(shift, operand, 3);
    shift[3] = '\0'; //ensuring the string copy is closed

    if (strcmp(shift, "lsr") == 0) {
        opr = 1 << 22;
    } else if (strcmp(shift, "asr") == 0) {
        opr = 1 << 23;
    } else if (strcmp(shift, "ror") == 0) {
        opr = 3 << 22;
    }
    free(shift);

    return opr;
}


static void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    

    //if halt instruction occurs then automatically write to file 
    if ((strcmp(instruction.opcode, "and") == 0) && (strcmp(instruction.operand[0], "x0") == 0) && (strcmp(instruction.operand[1], "x0") == 0) && (strcmp(instruction.operand[2], "x0") == 0)) {
        writeToFile(2315255808, file);
        fclose(file);
        return;
    }

    //setting standard bits for logic instructions 
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t M = 0;
    uint32_t opr = 0;

    //setting register bits accounting for aliases 
    uint32_t rm = ((strcmp(instruction.opcode, "mov") == 0) ? getReg(instruction.operand[1]) : getReg(instruction.operand[2]));
    rm = rm << 16;
    uint32_t rn = ((strcmp(instruction.opcode, "mov") == 0) || strcmp(instruction.opcode, "mvn") == 0) ? getReg(NULL) : getReg(instruction.operand[1]);
    rn = rn << 5;
    uint32_t rd = getReg(instruction.operand[0]);

    //adds shift bits if there
    uint32_t operand = 0;
    char* immOp = (strcmp(instruction.opcode, "mvn") == 0) ? instruction.operand[1] : instruction.operand[3];
    if (immOp != NULL) {
        opr = getOpr(instruction.operand[3]);
        char *number = instruction.operand[3] + 5;
        operand = getimmm(number) << 10;
    }

    //gets the N bit for each logic instruction
    uint32_t N = getN(instruction.opcode) << 21;


    uint32_t write_val = sf | opcode_bin | M | data_processing_register_code | N | opr | rm | operand | rn | rd;
    writeToFile(write_val, file);
    fclose(file);
}

static void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");

    //sets standard bits for tst 
    uint32_t opcode_bin = getOpcode(instruction, opcodeMapping, opcode_map_size);
    uint32_t sf = getSf(instruction.operand[0]);
    uint32_t M = 0;
    uint32_t opr = 0;
    uint32_t rd = getReg(NULL); //gets the zero register 
    uint32_t rn = getReg(instruction.operand[0]) << 5; 
    uint32_t rm = getReg(instruction.operand[1]) << 16;
    uint32_t operand = 0;

    //checks if there is a shift and then changes the number accoringly 
    if (instruction.operand[2] != NULL) {
        opr = getOpr(instruction.operand[2]);
        char *number = instruction.operand[2] + 5;
        operand = getimmm(number) << 10;
    }


    uint32_t write_val = sf | opcode_bin | M | data_processing_register_code  | opr | rm | operand | rn | rd;
    writeToFile(write_val, file);
    fclose(file);
}


//Helper function to return the N bit for logic instructions 
static uint32_t getN(const char *opcode) {

    if (strcmp(opcode, "bic") == 0 || strcmp(opcode, "orn") == 0 || strcmp(opcode, "eon") == 0 || strcmp(opcode, "bics") == 0) {
        return 1;
    }
    return 0;
}
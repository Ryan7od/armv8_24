#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdint.h>


#include "symbolTable.h"
#include "utilities.h"

#define MaxLineLength 30



uint32_t data_processing_immediate_code = 1 << 28;
uint32_t data_processing_register_code = 5 << 25;




InstructionMapping mappings[38] = { //Table mapping instruction opcodes to their function
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
        
};

size_t mappingCount = 38;


OpcodeMapping opcodeMapping[23] = { //instruction mapping opcode mnemonic to number with bit mask  
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

BranchMapping branchMapping[7] = {
        {"eq", 0},
        {"ne", 1},
        {"ge", 10},
        {"lt", 11},
        {"gt", 12},
        {"le", 13},
        {"al", 14}
};

size_t opcode_msize = 23;


//sets first pass as true 
bool firstPassFlag = true;


dynarray SymbolTable;



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





InstructionIR parser(char *line) {
    return tokenizer(line);
}


extern void parseDirective(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
    FILE *file = fopen(output, "ab");
    printf(".int parsing");
    int address = (int)strtol(instruction.operand[0], NULL, 0);
    uint32_t write_val = address;
    //writes the number on the right of the directive to the file 
    writeToFile(write_val, file);
    printf("%u\n", write_val);
    fclose(file);
}



extern InstructionParser functionClassifier(InstructionIR instruction, InstructionMapping* mapping, size_t mapSize) {
    //iterates through the function map and returns the matching function to the opcode 
    for (size_t i = 0; i < mapSize; i++) {
        if (strcmp(instruction.opcode, mappings[i].mnemonic) == 0) { return  mapping[i].parser; }
    }
    return NULL;
}



extern void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
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



extern void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
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



// single data transfer is str or ldr
// load literal just ldr
extern void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size) {
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



extern void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
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



extern void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
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




extern void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
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

extern void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size) {
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdint.h>

#include "symbolTable.h"
#include "utilities.h"

//Helper function to return the N bit for logic instructions 
uint32_t getN(const char *opcode) {

    if (strcmp(opcode, "bic") == 0 || strcmp(opcode, "orn") == 0 || strcmp(opcode, "eon") == 0 || strcmp(opcode, "bics") == 0) {
        return 1;
    }
    return 0;
}

//helper function which gets the opr in a shift
uint32_t getOpr(char *operand) {

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

//helper function which writes to file
void writeToFile(uint32_t write_val, FILE *file) {
    size_t written = fwrite(&write_val, sizeof(uint32_t), 1, file);

    //if nothing is written an error occurs
    if (written != 1) {
        fprintf(file, "error writing to file");
        exit(1);
    }
}

//Helper function to return the significant bit 
uint32_t getSf(char *reg) {
    //significant bit 0 if w register and 1 otherwise 
    if (*reg == 'w' || *reg == 'W') {
        return 0;
    } else {
        return 1 << 31;
    }
}

//Helper function to return the register 
uint32_t getReg(char *reg) {
    //returns 31 for the zero register
    if (reg == NULL || (strcmp(reg+1, "zr") == 0)) {
        return 31;
    }
    uint32_t rv = strtoul((reg + (1*sizeof (char))), NULL, 10);
    return rv;
}


//Helper function to get an immediate value 
uint32_t getimmm(char *num) {
    if (strlen(num) >= 2) {
        //if statement checks whether number is hex or decimal 
        if (*(num+1) == 'x') {
            return strtoul(num+2, NULL, 16);
        }
    }
    return strtoul(num, NULL, 10);
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

//Helper function to get the binary encoding of conditional branches
uint32_t getEncoding(const char* mnemonic) {
    for (int i = 0; i < BranchMappingSize; i++) {
        if (strcmp(branchMapping[i].mnemonic, mnemonic) == 0) {
            return branchMapping[i].encoding;
        }
    }
    exit(1);
}


char * trim_leading_spaces(char *str) {
    int index = 0;
    //iterates through list until a space character is not seen 
    while(isspace(str[index])) { 
        index++;
    }
    char *newstr = str + index;
    return newstr; //returns string without leading spaces
}


int getAddress(dynarray symbolTable, const char *label) { //helper function to return the address of a label 
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

extern uint32_t getOpcode(InstructionIR instructionIr, OpcodeMapping mapping[], size_t size) {
        for (size_t i = 0; i < size; i++) {
        if (strcmp(instructionIr.opcode, mapping[i].mnemonic) == 0) {return mapping[i].opcode_bin;}
    }
    exit(1);
}

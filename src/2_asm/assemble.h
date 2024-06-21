
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

typedef struct {
    const char* mnemonic;
    InstructionParser parser;
} InstructionMapping;


void fileProcessor(char *inputfile, char *outputfile);
extern void parseLogic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
InstructionIR parser(char *line);
char* DataProcessingInstruction(InstructionIR instruction);
extern void parseMultiply(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
extern InstructionParser functionClassifier(InstructionIR instruction,  InstructionMapping* mappings, size_t mapSize);
extern void parseLoadStoreInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
extern void parseBranchInstructions(InstructionIR instruction, char *output, OpcodeMapping mapping[], size_t opcode_map_size);
extern void parseDirective(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
extern void parseArithmetic(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
extern void parseWideMove(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);
extern void parseTst(InstructionIR instruction, char *output, OpcodeMapping opcodeMapping[], size_t opcode_map_size);


extern InstructionMapping mappings[38];
extern size_t mappingCount;
extern OpcodeMapping opcodeMapping[23];
extern BranchMapping branchMapping[7];
extern size_t opcode_msize;
#include <stdlib.h>

extern void loadStoreHandler(uint32_t instruction);
extern void loadStoreHelperHandler(uint8_t l, uint8_t sf, uint8_t rt, uint64_t address);
extern void loadStoreLoadLiteralHandler(uint32_t instruction);
extern void loadStoreUnsignedOffsetHandler(uint32_t instruction);
extern void loadStoreRegOffHandler(uint32_t instruction);
extern void loadStorePIndexHandler(uint32_t instruction);
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utility.h"
#include "loadstore.h"



static void loadStoreLoadLiteralHandler(uint32_t instruction) {
    const uint32_t simm19 = mask32_AtoB_shifted(instruction, 23, 5);
    const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
    const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);

    uint64_t offset = simm19 << 2;
    offset = sExtend64(offset, 20);

    uint64_t address = read64(&sRegisters.PC) + offset;

    if (sf) { //64 bit
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            uint64_t mem_i = memory[address + i];
            uint64_t mem_i_shifted = mem_i << (8 * i);
            result = result + mem_i_shifted;
        }
        write64(&gRegisters[rt], result);
    } else { //32 bit
        uint32_t result = 0;
        for (int i = 0; i < 4; i++) {
            result += memory[address + i] << (8 * i);
        }
        write32(&gRegisters[rt], result);
    }
}

static void loadStoreHelperHandler(uint8_t l, uint8_t sf, uint8_t rt, uint64_t address) {
    if (l) { //Load
        if (sf) {
            uint64_t data = 0;
            for (int i = 0; i < 8; i++) {
                uint64_t i_byte = ((uint64_t)memory[address + i] << (8 * i));
                data += i_byte;
            }
            write64(&gRegisters[rt], data);
        } else {
            uint32_t data = 0;
            for (int i = 0; i < 4; i++) {
                uint32_t i_byte = ((uint32_t)memory[address + i] << (8 * i));
                data += i_byte;
            }
            write32(&gRegisters[rt], data);
        }
    } else { //Store
        if (sf) {
            uint64_t data = read64(&gRegisters[rt]);
            for (int i = 0; i < 8; i++) {
                memory[address + i] = data & 0xFF;
                data = data >> 8;
            }
        } else {
            uint32_t data = read32(&gRegisters[rt]);
            for (int i = 0; i < 4; i++) {
                memory[address + i] = data & 0xFF;
                data = data >> 8;
            }
        }
    }
}

static void loadStoreUnsignedOffsetHandler(uint32_t instruction) {
    const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
    const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
    const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
    const uint16_t imm12 = mask32_AtoB_shifted(instruction, 21, 10);
    const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);

    uint64_t uoffset;
    int msb;
    if (sf) {
        uoffset = imm12 << 3;
        msb = 14;
    } else {
        uoffset = imm12 << 2;
        msb = 13;
    }

    uint64_t address = sExtend64(uoffset, msb) + read64(&gRegisters[xn]);

    loadStoreHelperHandler(l, sf, rt, address);
}

static void loadStoreRegOffHandler(uint32_t instruction) {
    const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
    const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
    const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
    const uint16_t xm = mask32_AtoB_shifted(instruction, 20, 16);
    const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);

    uint64_t address = read64(&gRegisters[xm]) + read64(&gRegisters[xn]);

    loadStoreHelperHandler(l, sf, rt, address);
}

static void loadStorePIndexHandler(uint32_t instruction) {
    const uint8_t xn = mask32_AtoB_shifted(instruction, 9, 5);
    const uint8_t sf = mask32_AtoB_shifted(instruction, 30, 30);
    const uint8_t rt = mask32_AtoB_shifted(instruction, 4, 0);
    const uint16_t simm9 = mask32_AtoB_shifted(instruction, 20, 12);
    const uint8_t l = mask32_AtoB_shifted(instruction, 22, 22);
    const uint8_t i = mask32_AtoB_shifted(instruction, 11, 11);

    if (i) { //Pre-indexed
        uint64_t address = read64(&gRegisters[xn]) + sExtend64(simm9, 8);
        write64(&gRegisters[xn], address);
        loadStoreHelperHandler(l, sf, rt, address);
    } else { //Post-indexed
        uint64_t address = read64(&gRegisters[xn]);
        loadStoreHelperHandler(l, sf, rt, address);
        address +=  + sExtend64(simm9, 8);
        write64(&gRegisters[xn], address);
    }
}

void loadStoreHandler(uint32_t instruction) {
    const uint8_t diff = mask32_AtoB_shifted(instruction, 31, 31);

    if (!diff) { //Load literal
        loadStoreLoadLiteralHandler(instruction);
    } else {
        const uint8_t u = mask32_AtoB_shifted(instruction, 24, 24);
        if (u) { //Unsigned offset
            loadStoreUnsignedOffsetHandler(instruction);
        } else {
            const uint8_t diff2 = mask32_AtoB_shifted(instruction, 21, 21);
            if (diff2) { //Register offset
                loadStoreRegOffHandler(instruction);
            } else { // Pre/Post-index
                loadStorePIndexHandler(instruction);
            }
        }
    }
}
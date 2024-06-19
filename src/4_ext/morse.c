#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// TODO convert into

typedef enum {
    DOT,
    DASH,
    END,
    SPACE  // To indicate the end of Morse code sequence
} Morse;

#define MAX_MORSE_LENGTH 6  // Longest Morse code sequence is 6 symbols including end

// Morse code representation
Morse morseCode[27][MAX_MORSE_LENGTH] = {
    {DOT, DASH, END},             // A
    {DASH, DOT, DOT, DOT, END},   // B
    {DASH, DOT, DASH, DOT, END},  // C
    {DASH, DOT, DOT, END},        // D
    {DOT, END},                   // E
    {DOT, DOT, DASH, DOT, END},   // F
    {DASH, DASH, DOT, END},       // G
    {DOT, DOT, DOT, DOT, END},    // H
    {DOT, DOT, END},              // I
    {DOT, DASH, DASH, DASH, END}, // J
    {DASH, DOT, DASH, END},       // K
    {DOT, DASH, DOT, DOT, END},   // L
    {DASH, DASH, END},            // M
    {DASH, DOT, END},             // N
    {DASH, DASH, DASH, END},      // O
    {DOT, DASH, DASH, DOT, END},  // P
    {DASH, DASH, DOT, DASH, END}, // Q
    {DOT, DASH, DOT, END},        // R
    {DOT, DOT, DOT, END},         // S
    {DASH, END},                  // T
    {DOT, DOT, DASH, END},        // U
    {DOT, DOT, DOT, DASH, END},   // V
    {DOT, DASH, DASH, END},       // W
    {DASH, DOT, DOT, DASH, END},  // X
    {DASH, DOT, DASH, DASH, END}, // Y
    {DASH, DASH, DOT, DOT, END},   // Z
    {SPACE} //Space
};

// Function to print Morse code individual character
void printMorse(Morse *morse) {
    for (int i = 0; morse[i] != END; i++) {
        if (morse[i] == DOT) {
            printf(".");
        } else if (morse[i] == DASH) {
            printf("-");
        }
    }
    printf(" ");
}

// Function to print Morse code string
void printMorseString(const char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            printMorse(morseCode[*str - 'A']);
        } else if (*str >= 'a' && *str <= 'z') {
            printMorse(morseCode[*str - 'a']);
        } else {
            printf(" ");  // Spaces between words
        }
        str++;
    }
    printf("\n");
}

void makeMorseBlink(const char *str) {
    FILE* file = fopen("blinking_morse.s", "w");
    int count = 0;

    //Initialise assembly file
    fprintf(file, "movz w0, #0x0\n"
                  "movz w1, #0x200\n"
                  "movz w6, #0x8\n"
                  "movz w2, #0x3f20, lsl #16\n"
                  "movz w3, #0x3f20, lsl #16\n"
                  "add w3, w3, #0x1c\n"
                  "movz w4, #0x3f20, lsl #16\n"
                  "add w4, w4, #0x28\n"
                  "str w1, [w2]\n"
                  "str w0, [w2, #0x4]\n"
                  "str w0, [w2, #0x8]\n"
                  "str w0, [w2, #0xc]\n"
                  "str w0, [w2, #0x10]\n"
                  "str w0, [w2, #0x14]\n"
                  "str w0, [w3]\n"
                  "str w0, [w3, #0x4]\n"
                  "str w0, [w4]\n"
                  "str w0, [w4, #0x4]\n");

    while (*str) {
        Morse* val = NULL;
        if (*str >= 'A' && *str <= 'Z') {
            val = (morseCode[*str - 'A']);
        } else if (*str >= 'a' && *str <= 'z') {
            val = morseCode[*str - 'a'];
        } else {
            val = morseCode[26];
        }
        str++;

        while(*val) {
            if (*val == DOT) {
                fprintf(file, "str w0, [w4]\n"
                              "str w6, [w3]\n"
                              "movz w5, #50, lsl #16\n"
                              "DELAY%d:\n"
                              "sub w5, w5, #1\n"
                              "cmp w5, w0\n"
                              "b.ne DELAY%d\n"
                              "str w0, [w3]\n"
                              "str w6, [w4]\n",
                              count, count);
            } else if (*val == DASH) {
                fprintf(file, "str w0, [w4]\n"
                              "str w6, [w3]\n"
                              "movz w5, #100, lsl #16\n"
                              "DELAY%d:\n"
                              "sub w5, w5, #1\n"
                              "cmp w5, w0\n"
                              "b.ne DELAY%d\n"
                              "str w0, [w3]\n"
                              "str w6, [w4]\n",
                              count, count);
            } else if (*val == SPACE) {
                fprintf(file, "movz w5, #200, lsl #16\n"
                              "DELAY%d:\n"
                              "sub w5, w5, #1\n"
                              "cmp w5, w0\n"
                              "b.ne DELAY%d\n",
                              count, count);
            }

            val++;
            count++;

            //Space between characters
            fprintf(file, "movz w5, #75, lsl #16\n"
                          "DELAY%d:\n"
                          "sub w5, w5, #1\n"
                          "cmp w5, w0\n"
                          "b.ne DELAY%d\n",
                          count, count);

            count++;
        }
    }

}

int main(int argc, char **argv) {
    if (argc != 2 ) {
        fprintf(stderr, "Program must be run with 1 argument: English phrase to translate");
    }

    const char *string = argv[1];
    printMorseString(string);
    makeMorseBlink(string);
    return 0;
}

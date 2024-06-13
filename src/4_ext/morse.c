#include <stdio.h>

// TODO convert into

typedef enum {
    DOT,
    DASH,
    END  // To indicate the end of Morse code sequence
} Morse;

#define MAX_MORSE_LENGTH 6  // Longest Morse code sequence is 6 symbols including end

// Morse code representation
Morse morseCode[26][MAX_MORSE_LENGTH] = {
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
    {DASH, DASH, DOT, DOT, END}   // Z
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

int main() {
    const char *testString = "HELLO WORLD";
    printMorseString(testString);
    return 0;
}

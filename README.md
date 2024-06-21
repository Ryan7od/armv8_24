# armv8_24

This is group 24's source code for the Imperial CS 2024 C Group Project (CGP)

## Components

In ./src there are 4 subdirectories:
- ./1_emu

Limited Instruction Set **AArch64** Emulator
- ./2_asm

Limited Instruction Set **AArch64** Assembler
- ./3_pi

Binary program to emit a periodic pulse from RaspberryPi GPIO port 3
- ./4_ext

C Program that turns English text into Morse code, outputting it as a string and writing a binary file to convert that Morse into signal pulses from port 3


## Compilation

First, navigate to src:
```bash
cd src/
```

Then to compile using the MakeFile run:
```bash
# To compile parts 1, 2, 3, 4
make all

# To compile part 1 individually
make emulate

# To compile part 2 individually
make assemble

# To compile part 3 individually
make pi

# To compile part 4 individually
make morse

# To delete all compiled files
make clean
# make clean also deletes all object files, binary files and kernel8.img
```

All executable files are created directly in ./src and all object files are created in their respective subdirectories.

The output to parts 3 and 4 will be called "kernel8.img" (to work with the Pi) so make sure you delete one before making the other.

Alternatively for parts 1 and 2, if you have CBuild downloaded you can simply go into each directory and run:
```bash
# To compile
cb

# To delete compiled and object files
cb --clean
```

## Usage

To run part 1:
```bash
./emulate $(binary input filename) $(optional output filename)
```
To run part 2:
```bash
./assemble $(assembly input filename) $(binary output filename)
```
In both cases you must run this command in the directory in which the executable is located (./src if using the MakeFile and ./src/*{1_emu or 2_asm}* if using cb).

To manually create the kernel file for part 3 run:
```bash
./assemble ./3_pi/blink_led.s kernel8.img
```

To manually create the kernel file for part 4 run:
```bash
./morse "your text to convert"
./assemble blinking_morse.s kernel8.img
```
Alternatively, you can use the MakeFile:
```bash
make morseRun TEXT="your text to convert"
```

To run parts 3 and 4, load kernel8.img onto an SD/microSD card along with
[boocode.bin](https://github.com/raspberrypi/firmware/blob/master/boot/bootcode.bin) and
[start.elf](https://github.com/raspberrypi/firmware/blob/master/boot/start.elf).
Put this card in the Pi and turn it on and the light sequence will start.
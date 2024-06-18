movz w0, #0x0
movz w1, #0x200
movz w6, #0x8

movz w2, #0x3f20, lsl #16
movz w3, #0x3f20, lsl #16
add w3, w3, #0x1c
movz w4, #0x3f20, lsl #16
add w4, w4, #0x28

str w1, [w2]
str w0, [w2, #0x4]
str w0, [w2, #0x8]
str w0, [w2, #0xc]
str w0, [w2, #0x10]
str w0, [w2, #0x14]

str w0, [w3]
str w0, [w3, #0x4]
str w0, [w4]
str w0, [w4, #0x4]

LOOP:
str w0, [w4]
str w6, [w3]

movz w5, #50, lsl #16

DELAY_H1:
sub w5, w5, #1
cmp w5, w0
b.ne DELAY_H1

str w0, [w3]
str w6, [w4]

movz w7, #50, lsl #16

DELAY_L1:
sub w7, w7, #1
cmp w7, w0
b.ne DELAY_L1

cmp w0, w0
b.eq LOOP
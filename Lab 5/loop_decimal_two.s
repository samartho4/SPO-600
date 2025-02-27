// File: loop_decimal_two.s
.section .data
msg:      .asciz "Loop:  \n"   // Two spaces after ':' for two digits.
charbuf:  .byte 0

.section .text
.global _start

min = 0
max = 33                       // Loop from 0 to 32

_start:
    mov     x19, min           // Initialize loop counter

loop:
    // Print the fixed text "Loop:  " (8 bytes)
    adr     x0, msg
    bl      print_string

    // Load immediate 10 into x20 for division
    mov     x20, #10

    // Compute quotient and remainder for 2-digit conversion.
    udiv    x1, x19, x20       // x1 = quotient = x19 / 10
    mul     x2, x1, x20        // x2 = quotient * 10
    sub     x2, x19, x2        // x2 = remainder = x19 - (quotient * 10)

    // Convert quotient digit to ASCII and print.
    add     x1, x1, #0x30      // Convert to ASCII ('0' = 0x30)
    mov     x0, x1
    bl      print_char         // Print first digit

    // Convert remainder digit to ASCII and print.
    add     x2, x2, #0x30      // Convert remainder to ASCII.
    mov     x0, x2
    bl      print_char         // Print second digit

    // Print newline (ASCII 0x0A)
    mov     x0, #0x0A
    bl      print_char

    add     x19, x19, #1       // Increment loop counter.
    cmp     x19, max
    b.ne    loop

    // Exit the program.
    mov     x0, 0
    mov     x8, 93             // Syscall exit is number 93.
    svc     0

// -----------------------------
// Subroutine: print_string
// x0: pointer to the string to print.
print_string:
    mov     x1, #8             // "Loop:  " is 8 bytes.
    mov     x8, #64            // Syscall write is 64.
    mov     x2, x1             // x2 = number of bytes to write.
    mov     x1, x0             // x1 = pointer to the string.
    mov     x0, #1             // stdout file descriptor.
    svc     0
    ret

// -----------------------------
// Subroutine: print_char
// x0: contains the ASCII code for the character to print.
print_char:
    adrp    x1, charbuf        // Load page address of charbuf.
    add     x1, x1, :lo12:charbuf // Add offset to get full address.
    strb    w0, [x1]           // Store lower 8 bits of x0 into charbuf.
    mov     x0, #1             // stdout file descriptor.
    mov     x2, #1             // Write 1 byte.
    mov     x8, #64            // Syscall write is 64.
    svc     0
    ret

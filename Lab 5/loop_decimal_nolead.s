// File: loop_decimal_nolead.s
.section .data
msg:      .asciz "Loop: "      // Fixed text; digit(s) will follow.
newline:  .asciz "\n"          // Newline string.
charbuf:  .byte 0

.section .text
.global _start

min = 0
max = 33                       // Loop from 0 to 32

_start:
    mov     x19, min           // Initialize loop counter

loop:
    // Print "Loop: "
    adr     x0, msg
    bl      print_string_fixed  // Print the fixed text ("Loop: ")

    // Check if x19 < 10.
    cmp     x19, #10
    blt     print_single        // If less than 10, print one digit.

    // Otherwise, print two digits.
    mov     x3, #10             // Load 10 into x3 for division.
    udiv    x1, x19, x3         // x1 = quotient = x19 / 10.
    mul     x2, x1, x3          // x2 = quotient * 10.
    sub     x2, x19, x2         // x2 = remainder = x19 - (quotient*10).

    // Convert quotient to ASCII.
    add     x1, x1, #0x30       // Add 0x30 to convert to ASCII.
    mov     x0, x1
    bl      print_char

    // Convert remainder to ASCII.
    add     x2, x2, #0x30       // Add 0x30 to convert to ASCII.
    mov     x0, x2
    bl      print_char
    b       print_nl            // Go print newline.

print_single:
    // For numbers less than 10, print a single digit.
    mov     x0, x19
    add     x0, x0, #0x30       // Convert to ASCII.
    bl      print_char

print_nl:
    // Print newline.
    adr     x0, newline
    bl      print_string_newline

    add     x19, x19, #1       // Increment loop counter.
    cmp     x19, max
    b.ne    loop

    // Exit.
    mov     x0, 0
    mov     x8, 93
    svc     0

// -----------------------------
// Subroutine: print_string_fixed
// Prints the fixed string "Loop: " (6 bytes).
print_string_fixed:
    mov     x1, #6             // "Loop: " is 6 bytes.
    mov     x8, #64            // Write syscall.
    mov     x2, x1
    mov     x1, x0
    mov     x0, #1             // stdout.
    svc     0
    ret

// Subroutine: print_string_newline
// Prints a newline (assumed 1 byte).
print_string_newline:
    mov     x1, #1
    mov     x8, #64
    mov     x2, x1
    mov     x1, x0
    mov     x0, #1
    svc     0
    ret

// -----------------------------
// Subroutine: print_char
// Prints the single character contained in x0.
print_char:
    adrp    x1, charbuf
    add     x1, x1, :lo12:charbuf
    strb    w0, [x1]           // Store the lower 8 bits of x0.
    mov     x0, #1             // stdout.
    mov     x2, #1             // Write 1 byte.
    mov     x8, #64            // Syscall write.
    svc     0
    ret

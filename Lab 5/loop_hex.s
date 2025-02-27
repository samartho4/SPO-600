// File: loop_hex.s
.section .data
hexmsg:   .asciz "Hex:  "   // The message to print before the hex digits.
newline:  .asciz "\n"       // Newline string.
charbuf:  .byte 0           // One-byte buffer for a single character.

.section .text
.global _start

min = 0
max = 33                   // Loop from 0 to 32 (decimal), which is 00 to 20 in hex.

_start:
    mov     x19, min       // Initialize loop counter.

loop:
    // Print the fixed text "Hex:  "
    adr     x0, hexmsg
    bl      print_string_hex

    // Compute quotient and remainder for hex conversion.
    // We want to divide the number (x19) by 16.
    mov     x3, #16        // Load divisor 16 into x3.
    udiv    x1, x19, x3    // x1 = quotient = x19 / 16.
    mul     x2, x1, x3     // x2 = quotient * 16.
    sub     x2, x19, x2    // x2 = remainder = x19 - (quotient * 16).

    // Convert quotient (high nibble) to its ASCII hex digit.
    cmp     x1, #10
    blt     conv_q_dec
    add     x1, x1, #0x37  // For values 10-15, add 0x37 to get 'A'-'F'.
    b       conv_q_done
conv_q_dec:
    add     x1, x1, #0x30  // For values 0-9, add 0x30 to get '0'-'9'.
conv_q_done:
    mov     x0, x1
    bl      print_char

    // Convert remainder (low nibble) to its ASCII hex digit.
    cmp     x2, #10
    blt     conv_r_dec
    add     x2, x2, #0x37  // For values 10-15, add 0x37.
    b       conv_r_done
conv_r_dec:
    add     x2, x2, #0x30  // For values 0-9, add 0x30.
conv_r_done:
    mov     x0, x2
    bl      print_char

    // Print a newline.
    adr     x0, newline
    bl      print_string_newline

    // Increment loop counter.
    add     x19, x19, #1
    cmp     x19, max
    b.ne    loop

    // Exit the program.
    mov     x0, #0
    mov     x8, #93        // Syscall exit (93)
    svc     0

//-----------------------------
// Subroutine: print_string_hex
// Prints the string in x0 ("Hex:  ") with length 6.
print_string_hex:
    mov     x1, #6         // "Hex:  " is 6 bytes.
    mov     x8, #64        // Syscall write.
    mov     x2, x1         // Number of bytes.
    mov     x1, x0         // Pointer to string.
    mov     x0, #1         // File descriptor: stdout.
    svc     0
    ret

// Subroutine: print_string_newline
// Prints a newline character from x0.
print_string_newline:
    mov     x1, #1
    mov     x8, #64
    mov     x2, x1
    mov     x1, x0
    mov     x0, #1
    svc     0
    ret

//-----------------------------
// Subroutine: print_char
// Prints the single character (ASCII code in x0) by storing it into a buffer.
print_char:
    adrp    x1, charbuf
    add     x1, x1, :lo12:charbuf
    strb    w0, [x1]       // Store the lower 8 bits of x0 into charbuf.
    mov     x0, #1         // File descriptor: stdout.
    mov     x2, #1         // Write 1 byte.
    mov     x8, #64        // Syscall write.
    svc     0
    ret

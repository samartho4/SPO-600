    .section .data
times_str:    .asciz " x "           // Printed between multiplicand and multiplier.
equals_str:   .asciz " = "           // Printed before the product.
newline:      .asciz "\n"            // Newline.
sep_line:     .asciz "-------------\n"  // Separator line after each table.
charbuf:      .byte 0               // One‐byte buffer for a single character.

    .section .bss
    .align 3
intbuf:       .skip 16              // Buffer for converting an integer to a string.

    .section .text
    .global _start

_start:
    // Outer loop: multiplier j from 1 to 12 (stored in x21)
    mov     x21, #1

outer_loop:
    // Inner loop: multiplicand i from 1 to 12 (stored in x20)
    mov     x20, #1

inner_loop:
    // Print the multiplicand (i)
    mov     x0, x20
    bl      print_int

    // Print literal " x "
    adr     x0, times_str
    bl      print_str_fixed

    // Print the multiplier (j)
    mov     x0, x21
    bl      print_int

    // Print literal " = "
    adr     x0, equals_str
    bl      print_str_fixed

    // Compute product = i * j (store in x22)
    mul     x22, x20, x21
    mov     x0, x22
    bl      print_int

    // Print newline
    adr     x0, newline
    bl      print_str_fixed

    add     x20, x20, #1        // i++
    cmp     x20, #13            // while i <= 12
    blt     inner_loop

    // Print separator line after each multiplier block.
    adr     x0, sep_line
    bl      print_str_fixed

    add     x21, x21, #1        // j++
    cmp     x21, #13            // while j <= 12
    blt     outer_loop

    // Exit
    mov     x0, #0
    mov     x8, #93             // exit syscall is 93
    svc     0

////////////////////////////////////////////////////////////////////////
// Subroutine: print_str_fixed
//   Prints a null-terminated string pointed to by x0, one character at a time.
print_str_fixed:
    stp     x1, x2, [sp, #-16]!   // Save registers.
    mov     x1, x0                // x1 = pointer to string.
print_str_loop:
    ldrb    w2, [x1], #1          // Load byte; post-increment pointer.
    cbz     w2, print_str_done    // If zero, end of string.
    mov     w0, w2                // Move character code into w0.
    bl      print_char
    b       print_str_loop
print_str_done:
    ldp     x1, x2, [sp], #16     // Restore registers.
    ret

////////////////////////////////////////////////////////////////////////
// Subroutine: print_char
//   Prints a single character whose ASCII code is in w0.
print_char:
    adrp    x1, charbuf           // Get page address of charbuf.
    add     x1, x1, :lo12:charbuf  // Complete address.
    strb    w0, [x1]              // Store the byte in charbuf.
    mov     x0, #1                // stdout file descriptor.
    mov     x2, #1                // Number of bytes = 1.
    mov     x8, #64               // Syscall write (64).
    svc     0
    ret

////////////////////////////////////////////////////////////////////////
// Subroutine: print_int
//   Converts a nonnegative integer (in x0) to a decimal string and prints it.
print_int:
    cmp     x0, #0
    bne     convert_int
    // If x0 is zero, print "0"
    mov     x0, #0x30            // ASCII '0'
    bl      print_char
    ret
convert_int:
    adr     x1, intbuf           // x1 points to intbuf.
    add     x1, x1, #15          // Point to last byte.
    mov     w2, #0               // Null terminator.
    strb    w2, [x1]
    sub     x1, x1, #1          // Prepare for digit storage.
convert_int_loop:
    mov     x3, #10
    udiv    x4, x0, x3          // x4 = quotient = x0 / 10.
    mul     x5, x4, x3          // x5 = quotient * 10.
    sub     x6, x0, x5          // x6 = remainder = x0 - (quotient*10).
    add     x6, x6, #0x30       // Convert remainder to ASCII.
    strb    w6, [x1]            // Store digit.
    mov     x0, x4              // Update x0 to quotient.
    sub     x1, x1, #1          // Move pointer left.
    cmp     x0, #0
    bne     convert_int_loop
    add     x1, x1, #1          // Adjust pointer to first digit.
    mov     x0, x1              // x0 now points to the start of the string.
    bl      print_str_fixed
    ret

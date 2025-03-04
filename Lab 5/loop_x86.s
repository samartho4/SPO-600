.section .data
msg:     .asciz "Loop: "      # Message to print
newline: .asciz "\n"          # Newline character

.section .bss
buffer:  .space 2             # Reserve space for two characters

.section .text
.global _start

_start:
    mov     $0, %r15          # Counter (loop index) starts at 0
    mov     $5, %r14          # Max value (loop condition: i < 6)

loop:
    # Print "Loop: "
    mov     $1, %rax          # syscall: write
    mov     $1, %rdi          # stdout
    lea     msg(%rip), %rsi   # Load address of msg
    mov     $6, %rdx          # Length of "Loop: "
    syscall

    # Print loop index (convert number to ASCII)
    mov     %r15, %rax        # Copy loop counter
    add     $48, %rax         # Convert number to ASCII ('0' = 48)
    mov     %al, buffer       # Store in buffer

    mov     $1, %rax          # syscall: write
    mov     $1, %rdi          # stdout
    lea     buffer(%rip), %rsi # Load address of buffer
    mov     $1, %rdx          # Print one byte
    syscall

    # Print newline
    mov     $1, %rax
    mov     $1, %rdi
    lea     newline(%rip), %rsi
    mov     $1, %rdx
    syscall

    # Increment counter
    inc     %r15
    cmp     %r15, %r14        # Compare counter with max (5)
    jne     loop              # If not equal, repeat loop

    # Exit syscall
    mov     $60, %rax         # syscall: exit
    xor     %rdi, %rdi        # return 0
    syscall

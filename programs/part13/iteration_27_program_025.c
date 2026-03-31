/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Non-volatile variables with different types */
    char c1 = 'A';
    short s1 = 0x1234;
    long l1 = 0x123456789ABCDEF0L;
    float f1 = 3.14159f;
    double d1 = 2.718281828459045;
    
    /* Explicit register variables - pin to specific registers */
    register int r12_var asm ("r12") = 0xDEADBEEF;
    register int r13_var asm ("r13") = 0xCAFEBABE;
    register int r14_var asm ("r14") = 0xFACEB00C;
    
    /* Variables for addressing mode conflicts */
    int addr_var1 = 0x11111111;
    int addr_var2 = 0x22222222;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Variables that will be live across multiple blocks */
    int live1, live2, live3, live4, live5;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Force many calculations to create register pressure */
    v1 = v1 * 2 + 1;
    v2 = v2 / 3 - 1;
    v3 = v3 ^ v4;
    v4 = v4 | v5;
    v5 = v5 & v6;
    v6 = v6 << 2;
    v7 = v7 >> 1;
    v8 = ~v8;
    
    /* Complex control flow with goto to create live ranges */
    goto block1;
    
block1:
    {
        /* Inline assembly with conflicting constraints */
        /* Input in register, output in memory, clobbers many registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            : [out1] "=m" (live1)          /* Memory output */
            : [in1] "r" (v1),              /* Register input */
              [in2] "r" (v2)               /* Another register input */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        /* Use explicit register variables in C code */
        r12_var = r12_var + live1;
        r13_var = r13_var - v3;
        
        /* Force address calculation */
        asm volatile (
            "leaq %[addr], %%rax\n\t"
            "movl (%%rax), %%ebx\n\t"
            "addl %%ebx, %[out]\n\t"
            : [out] "+r" (result1)         /* Read-write register */
            : [addr] "m" (addr_var1)       /* Memory input */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
    }
    goto block2;

block2:
    {
        /* Mixed data types in inline assembly */
        /* Different modes (char, int, long) */
        unsigned char byte_result;
        asm volatile (
            "movb %[in_char], %%al\n\t"
            "addb $0x10, %%al\n\t"
            "movb %%al, %[out_char]\n\t"
            "movl %[in_int], %%ebx\n\t"
            "subl $0x100, %%ebx\n\t"
            "movl %%ebx, %[out_int]\n\t"
            : [out_char] "=r" (byte_result),  /* Register output (char mode) */
              [out_int] "=m" (live2)          /* Memory output (int mode) */
            : [in_char] "r" (c1),             /* Register input (char) */
              [in_int] "r" (v4)               /* Register input (int) */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        c1 = byte_result;  /* Keep c1 live */
        
        /* Complex addressing mode with pointer */
        asm volatile (
            "movq %[ptr], %%rax\n\t"
            "movl (%%rax), %%ecx\n\t"
            "imull %[val], %%ecx\n\t"
            "movl %%ecx, %[out]\n\t"
            : [out] "=rm" (live3)           /* Register or memory output */
            : [ptr] "rm" (addr_ptr1),       /* Register or memory input */
              [val] "rm" (v5)               /* Another register/memory input */
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "memory", "cc"
        );
    }
    goto block3;

block3:
    {
        /* Inline assembly that clobbers our explicit register variables */
        /* This should force reloads of r12, r13, r14 */
        asm volatile (
            "movl $0x12345678, %%r12d\n\t"
            "movl $0x87654321, %%r13d\n\t"
            "movl $0xABCDEF01, %%r14d\n\t"
            "xorl %%eax, %%eax\n\t"
            "addl %%r12d, %%eax\n\t"
            "addl %%r13d, %%eax\n\t"
            "addl %%r14d, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (live4)
            : /* no inputs */
            : "rax", "r12", "r13", "r14", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use the explicit register variables again - they need reloading */
        r12_var = r12_var ^ live4;
        r13_var = r13_var | v6;
        r14_var = r14_var & v7;
        
        /* Floating point mixed with integer */
        int float_as_int;
        asm volatile (
            "movss %[flt], %%xmm0\n\t"
            "cvtss2si %%xmm0, %%eax\n\t"
            "addl %[intval], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (live5)
            : [flt] "xm" (f1),              /* SSE register or memory */
              [intval] "rm" (v8)            /* Integer in register/memory */
            : "rax", "xmm0", "xmm1", "xmm2", "memory", "cc"
        );
    }
    goto block4;

block4:
    {
        /* Final complex assembly with many constraints */
        /* Early clobber to force separate registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "movl %[in2], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in3], %%ecx\n\t"
            "subl %%ecx, %[out2]\n\t"
            : [out1] "=&r" (result2),       /* Early clobber register */
              [out2] "+&r" (result3)        /* Early clobber read-write */
            : [in1] "rm" (live1),
              [in2] "rm" (live2),
              [in3] "rm" (live3)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "memory", "cc"
        );
        
        /* More arithmetic to keep variables live */
        v1 = v1 + result2;
        v2 = v2 - result3;
        v3 = v3 ^ live4;
        v4 = v4 | live5;
        
        /* Use all explicit register variables one more time */
        asm volatile (
            "addl %%r12d, %[sum]\n\t"
            "addl %%r13d, %[sum]\n\t"
            "addl %%r14d, %[sum]\n\t"
            : [sum] "+r" (result1)
            : /* inputs in registers r12, r13, r14 */
            : "cc"
        );
    }

    /* Final checksum to prevent dead code elimination */
    volatile int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                          result1 + result2 + result3 +
                          live1 + live2 + live3 + live4 + live5 +
                          r12_var + r13_var + r14_var +
                          c1 + s1 + (int)l1 + (int)f1 + (int)d1;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}

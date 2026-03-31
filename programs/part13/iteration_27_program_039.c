/* reload_test.c - Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f;
volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Mixed data types to create mode mismatches */
    char c1 = 11, c2 = 22;
    short s1 = 111, s2 = 222;
    int i1 = 1111, i2 = 2222, i3 = 3333, i4 = 4444;
    long l1 = 11111L, l2 = 22222L;
    float f5 = 5.5f, f6 = 6.6f;
    double d4 = 4.4, d5 = 5.5;
    
    /* Variables whose addresses will be taken */
    int addr_var1 = 0xAAAA, addr_var2 = 0xBBBB;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values across blocks */
    volatile int checksum = 0;
    
    /* Block 1: Create register pressure with volatile usage */
block1:
    v1 = v2 + v3;
    v4 = v5 * v6;
    f1 = f2 + f3;
    d1 = d2 * d3;
    
    /* Complex inline assembly with conflicting constraints */
    /* Inputs in registers, outputs in memory, clobbers many registers */
    asm volatile (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        : [out1] "=m" (addr_var1), 
          [out2] "=m" (addr_var2)
        : [in1] "r" (i1), 
          [in2] "r" (i2), 
          [in3] "r" (i3)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Use the results to keep them live */
    checksum += addr_var1 + addr_var2;
    
    /* Block 2: More complex assembly with address constraints */
    /* goto creates live value spans */
    if (checksum > 0) goto block2;
    goto block3;
    
block2:
    /* Force addressing mode conflicts */
    /* Variable used both by address and in register */
    asm volatile (
        "movl $0x1234, (%[addr])\n\t"
        "addl $1, %[reg]\n\t"
        "movl %[reg], %%ecx\n\t"
        : 
        : [addr] "r" (ptr1), [reg] "r" (addr_var1)
        : "rax", "rcx", "rdx", "memory"
    );
    
    /* Use explicit register variables that get clobbered */
    asm volatile (
        "mov %0, %%r12\n\t"
        "add $100, %%r12\n\t"
        "mov %%r12, %1\n\t"
        : "=r" (i4)
        : "0" (r12_var)
        : "r12", "cc"
    );
    
    /* Mixed type operations to cause mode reloads */
    asm volatile (
        "mov %w[char_in], %w[char_out]\n\t"
        "mov %[short_in], %[short_out]\n\t"
        : [char_out] "=r" (c1),
          [short_out] "=r" (s1)
        : [char_in] "r" (c2),
          [short_in] "r" (s2)
        : "cc"
    );
    
    checksum += i4 + c1 + s1;
    
block3:
    /* Block 3: Floating point with integer mix */
    /* Create more register pressure */
    v7 = v8 + v9;
    v10 = v1 * v2;
    f4 = f5 + f6;
    d4 = d5 * 2.0;
    
    /* Inline asm with multiple output constraints */
    /* Using "=&r" for early clobber to force separate registers */
    asm volatile (
        "mov %[in_a], %%eax\n\t"
        "mov %[in_b], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[out_a]\n\t"
        "imul %%eax, %%ebx\n\t"
        "mov %%ebx, %[out_b]\n\t"
        : [out_a] "=&r" (i1),  /* Early clobber */
          [out_b] "=&r" (i2)   /* Early clobber */
        : [in_a] "r" (l1), 
          [in_b] "r" (l2)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    /* Another asm with memory input and register output */
    /* This can force reloads */
    asm volatile (
        "mov (%[mem_in]), %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %[reg_out]\n\t"
        : [reg_out] "=r" (i3)
        : [mem_in] "r" (&v3)
        : "rax", "memory", "cc"
    );
    
    checksum += i1 + i2 + i3;
    
    /* Block 4: More register pressure with clobbers */
    /* Use all explicit register variables */
    r13_var = r12_var + r14_var;
    r15_var = r13_var * 2;
    
    /* Assembly that clobbers our explicit registers */
    asm volatile (
        "mov $500, %%r12\n\t"
        "mov $600, %%r13\n\t"
        "mov $700, %%r14\n\t"
        "add %%r13, %%r12\n\t"
        "add %%r14, %%r12\n\t"
        "mov %%r12, %0\n\t"
        : "=m" (addr_var1)
        : 
        : "r12", "r13", "r14", "cc"
    );
    
    /* Force the compiler to reload our register variables */
    checksum += r12_var + r13_var + r14_var + r15_var;
    
    /* Block 5: Final complex assembly with many operands */
    {
        int tmp1, tmp2, tmp3, tmp4;
        
        /* 8 operands to stress reload */
        asm volatile (
            "mov %[a1], %%eax\n\t"
            "add %[a2], %%eax\n\t"
            "mov %%eax, %[r1]\n\t"
            "mov %[a3], %%ebx\n\t"
            "add %[a4], %%ebx\n\t"
            "mov %%ebx, %[r2]\n\t"
            "mov %[a5], %%ecx\n\t"
            "add %[a6], %%ecx\n\t"
            "mov %%ecx, %[r3]\n\t"
            "mov %[a7], %%edx\n\t"
            "add %[a8], %%edx\n\t"
            "mov %%edx, %[r4]\n\t"
            : [r1] "=r" (tmp1),
              [r2] "=r" (tmp2),
              [r3] "=r" (tmp3),
              [r4] "=r" (tmp4)
            : [a1] "r" (v1), [a2] "r" (v2),
              [a3] "r" (v3), [a4] "r" (v4),
              [a5] "r" (v5), [a6] "r" (v6),
              [a7] "r" (v7), [a8] "r" (v8)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        checksum += tmp1 + tmp2 + tmp3 + tmp4;
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}

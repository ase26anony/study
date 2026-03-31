/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force the compiler to generate reloads by creating complex
   register pressure and conflicting constraints */

int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Non-volatile variables with arithmetic */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    int nv5 = 500, nv6 = 600, nv7 = 700, nv8 = 800;
    
    /* Floating point variables to increase register pressure */
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.23456, d2 = 2.34567, d3 = 3.45678;
    
    /* Explicit register variables - pin to specific registers */
    register int r12_var asm ("r12") = 0x88888888;
    register int r13_var asm ("r13") = 0x99999999;
    register int r14_var asm ("r14") = 0xAAAAAAAA;
    register int r15_var asm ("r15") = 0xBBBBBBBB;
    
    /* Variables with different sizes/types for mode mismatches */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    long long ll1 = 0x1122334455667788LL;
    
    /* Force many live values across basic blocks */
    int result1 = 0, result2 = 0, result3 = 0;
    volatile int checksum = 0;
    
    /* Block 1: Create initial register pressure */
block1:
    /* Complex arithmetic to create many live values */
    nv1 = v1 + v2;
    nv2 = v3 * v4;
    nv3 = nv1 ^ nv2;
    nv4 = (v5 << 3) | (v6 >> 2);
    
    /* Inline assembly with conflicting constraints */
    /* This asm uses many input/output operands with register constraints
       that will conflict with the live C variables */
    asm volatile (
        /* Output operands with earlyclobber (&) to force separate registers */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "imull %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        /* Use the explicit register variables */
        "addl %%r12d, %[out1]\n\t"
        "subl %%r13d, %[out2]\n\t"
        /* Memory operand to force address calculation */
        "movl %[mem1], %%ecx\n\t"
        "addl %%ecx, %[out1]\n\t"
        : [out1] "=&r" (result1),  /* Earlyclobber - can't share with inputs */
          [out2] "=&r" (result2)   /* Earlyclobber */
        : [in1] "r" (nv1),         /* Register constraint */
          [in2] "r" (nv2),
          [in3] "r" (nv3),
          [in4] "r" (nv4),
          [mem1] "m" (v7),         /* Memory constraint - forces address */
          "r" (r12_var),           /* Implicit use of r12 */
          "r" (r13_var)            /* Implicit use of r13 */
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Modify variables to keep them live */
    v1 = result1;
    v2 = result2;
    
    /* Block 2: More complex inline assembly with different constraints */
block2:
    /* More arithmetic to create new live values */
    nv5 = v3 + v4 + v5;
    nv6 = v6 * v7 / v8;
    nv7 = (nv5 << 1) + (nv6 >> 1);
    nv8 = nv7 ^ 0xFFFF;
    
    /* Another inline assembly with memory output and mixed constraints */
    /* This creates addressing mode conflicts */
    int temp_addr;
    asm volatile (
        /* Complex addressing calculation */
        "leaq %[addr_var], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl %[in5], %%ebx\n\t"
        "movl %%ebx, %[out3]\n\t"
        /* Use different sized operands */
        "movb %[char1], %%cl\n\t"
        "movw %[short1], %%dx\n\t"
        "addb %%cl, %[out3]b\n\t"
        "addw %%dx, %[out3]w\n\t"
        /* Force spill/reload by using all caller-saved regs */
        "movl $0x1234, %%r8d\n\t"
        "movl $0x5678, %%r9d\n\t"
        "movl $0x9ABC, %%r10d\n\t"
        "movl $0xDEF0, %%r11d\n\t"
        "addl %%r8d, %[out3]\n\t"
        "addl %%r9d, %[out3]\n\t"
        "addl %%r10d, %[out3]\n\t"
        "addl %%r11d, %[out3]\n\t"
        : [out3] "=r" (result3),
          [addr_var] "=m" (temp_addr)
        : [in5] "r" (nv5),
          [char1] "r" ((int)c1),
          [short1] "r" ((int)s1),
          "m" (ll1)                /* Memory operand for long long */
        : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    /* Block 3: Create control flow with many live values */
block3:
    /* Use all variables to keep them live across goto */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += nv1 + nv2 + nv3 + nv4 + nv5 + nv6 + nv7 + nv8;
    checksum += result1 + result2 + result3;
    checksum += r12_var + r13_var + r14_var + r15_var;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += c1 + c2 + s1 + s2;
    
    /* Third inline assembly - force reloads with output memory operand */
    int output_mem;
    asm volatile (
        /* This pattern often triggers reloads */
        "movl %[in6], %%eax\n\t"
        "addl %[in7], %%eax\n\t"
        "movl %%eax, %[out_mem]\n\t"
        /* Clobber many registers to force spills */
        "movl $1, %%ebx\n\t"
        "movl $2, %%ecx\n\t"
        "movl $3, %%edx\n\t"
        "movl $4, %%esi\n\t"
        "movl $5, %%edi\n\t"
        : [out_mem] "=m" (output_mem)  /* Memory output - forces store */
        : [in6] "r" (nv6),
          [in7] "r" (nv7)
        : "rax", "rbx", "rcx", "rdx", "esi", "edi", "memory", "cc"
    );
    
    checksum += output_mem;
    
    /* Conditional goto to create complex control flow with live values */
    static int counter = 0;
    if (counter++ < 3) {
        /* Many variables are live here */
        v1 += checksum;
        v2 -= checksum;
        goto block2;  /* Jump back with many live values */
    }
    
    /* Final use of all variables to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Results: %d %d %d\n", result1, result2, result3);
    printf("Register vars: %d %d %d %d\n", r12_var, r13_var, r14_var, r15_var);
    
    return checksum & 0xFF;
}

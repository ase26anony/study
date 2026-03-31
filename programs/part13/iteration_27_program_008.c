/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep all variables live */
static volatile int checksum = 0;

int main(void) {
    /* ===== VOLATILE VARIABLES FOR FORCED SPILLS ===== */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    volatile float f1 = 3.14159f;
    volatile float f2 = 2.71828f;
    volatile double d1 = 1.41421356;
    volatile double d2 = 1.73205080;
    
    /* ===== EXPLICIT REGISTER VARIABLES ===== */
    register int r12_var asm ("r12") = 0x77777777;
    register int r13_var asm ("r13") = 0x88888888;
    register int r14_var asm ("r14") = 0x99999999;
    register int r15_var asm ("r15") = 0xAAAAAAAA;
    
    /* ===== NON-VOLATILE VARIABLES WITH COMPLEX USAGE ===== */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 100, s2 = 200, s3 = 300;
    long long ll1 = 0x1122334455667788LL;
    long long ll2 = 0x8877665544332211LL;
    
    /* ===== ADDRESS TAKEN VARIABLES ===== */
    int *ptr_a = &a;
    int *ptr_b = &b;
    volatile int *volatile_ptr = &v1;
    
    /* ===== COMPLEX ARITHMETIC TO CREATE LIVE VALUES ===== */
    a = v1 + v2;
    b = v3 * v4;
    c = v5 / (v6 + 1);
    d = v7 - v8;
    e = f1 * 2.0f;
    f = d1 + 1.0;
    
    /* ===== CONTROL FLOW WITH MULTIPLE BASIC BLOCKS ===== */
    block1:
    {
        /* Complex inline assembly with conflicting constraints */
        asm volatile (
            /* Output operands with early clobber to force separate registers */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in3], %%ebx\n\t"
            "imull %[in4], %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            /* Memory operand with address taken variable */
            "movl %[mem1], %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            "movl %%ecx, %[mem1]\n\t"
            /* Use explicit register variables */
            "addl %%r12d, %%eax\n\t"
            "addl %%r13d, %%ebx\n\t"
            : [out1] "=&r" (a),        /* Early clobber - can't share reg with inputs */
              [out2] "=&r" (b),        /* Another early clobber */
              [mem1] "+m" (*ptr_a)     /* Memory operand with addressing mode */
            : [in1] "r" (c),           /* Input in register */
              [in2] "r" (d),
              [in3] "r" (e),
              [in4] "r" (f),
              "r" (r12_var),           /* Implicit use of register variable */
              "r" (r13_var)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Modify variables to keep them live */
        r12_var += a;
        r13_var += b;
        c = d + e;
        f = g * h;
    }
    
    /* Force spill by using many variables */
    v1 = a + b + c + d + e + f + g + h;
    
    block2:
    {
        /* Mixed data types in inline assembly */
        asm volatile (
            /* Byte operations */
            "movb %[char1], %%al\n\t"
            "addb %[char2], %%al\n\t"
            "movb %%al, %[char3]\n\t"
            /* Word operations */
            "movw %[short1], %%ax\n\t"
            "subw %[short2], %%ax\n\t"
            "movw %%ax, %[short3]\n\t"
            /* Long long operations */
            "movq %[ll1], %%rax\n\t"
            "xorq %[ll2], %%rax\n\t"
            "movq %%rax, %[llout]\n\t"
            /* Floating point - causes different register class usage */
            "movss %[float1], %%xmm0\n\t"
            "addss %[float2], %%xmm0\n\t"
            "movss %%xmm0, %[floatout]\n\t"
            : [char3] "=r" (c3),
              [short3] "=r" (s3),
              [llout] "=r" (ll1),
              [floatout] "=x" (f1)
            : [char1] "r" (c1),
              [char2] "r" (c2),
              [short1] "r" (s1),
              [short2] "r" (s2),
              [ll1] "r" (ll1),
              [ll2] "r" (ll2),
              [float1] "x" (f1),
              [float2] "x" (f2)
            : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "cc", "memory"
        );
        
        /* More arithmetic to keep values live */
        i = j * k;
        l = m + n;
        o = p / 2;
    }
    
    block3:
    {
        /* Inline assembly that clobbers explicit register variables */
        asm volatile (
            "movl $0xDEADBEEF, %%r12d\n\t"  /* Clobber r12 */
            "movl $0xCAFEBABE, %%r13d\n\t"  /* Clobber r13 */
            "movl $0xFACEB00C, %%r14d\n\t"  /* Clobber r14 */
            "movl $0xBAADF00D, %%r15d\n\t"  /* Clobber r15 */
            /* Now use the original values - forcing reloads */
            "addl %%r12d, %[out1]\n\t"
            "addl %%r13d, %[out2]\n\t"
            : [out1] "+r" (r12_var),   /* Output in same register that was clobbered */
              [out2] "+r" (r13_var)
            :
            : "r12", "r13", "r14", "r15", "rax", "rbx", "rcx", "rdx",
              "memory", "cc"
        );
        
        /* Complex expression with many live variables */
        v2 = (a * b) + (c * d) - (e * f) + (g * h) - (i * j) + (k * l) - (m * n) + (o * p);
    }
    
    /* ===== FINAL COMPUTATION TO PREVENT DEAD CODE ELIMINATION ===== */
    checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
               a + b + c + d + e + f + g + h +
               i + j + k + l + m + n + o + p +
               r12_var + r13_var + r14_var + r15_var +
               c1 + c2 + c3 + s1 + s2 + s3 +
               (int)f1 + (int)f2 + (int)d1 + (int)d2 +
               (int)ll1 + (int)ll2;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

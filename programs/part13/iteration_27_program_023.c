/* Test program to trigger GCC reload pass uncovered block in reload.cc */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep all variables alive */
static volatile int checksum = 0;

int main(void) {
    /* ========== PHASE 1: Declare many competing variables ========== */
    /* Volatile variables to prevent optimization */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x13579BDF;
    volatile int v4 = 0x2468ACE0;
    volatile float f1 = 3.14159f;
    volatile float f2 = 2.71828f;
    volatile double d1 = 1.41421356;
    volatile double d2 = 1.73205080;
    
    /* Non-volatile variables with different types */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 0x11111111, i2 = 0x22222222, i3 = 0x33333333;
    long l1 = 0x4444444444444444L, l2 = 0x5555555555555555L;
    
    /* Explicit register variables - pin to specific registers */
    register int r12_var asm ("r12") = 0xDEADBEEF;
    register int r13_var asm ("r13") = 0xCAFEBABE;
    register int r14_var asm ("r14") = 0xBAADF00D;
    
    /* Variables whose addresses will be taken */
    int addr_var1 = 0x66666666;
    int addr_var2 = 0x77777777;
    int *ptr1 = &addr_var1;
    int *ptr2 = &addr_var2;
    
    /* ========== PHASE 2: Create register pressure ========== */
    /* Perform arithmetic to create many live values */
    i1 = v1 + v2;
    i2 = v3 * v4;
    i3 = i1 ^ i2;
    l1 = (long)v1 * (long)v2;
    l2 = (long)v3 << 4;
    
    /* Mix floating point operations */
    f1 = f1 * f2 + 1.0f;
    d1 = d1 / d2 * 3.14159;
    
    /* Use explicit register variables */
    r12_var = r12_var ^ 0x12345678;
    r13_var = r13_var + r12_var;
    r14_var = r14_var * 2;
    
    /* ========== BLOCK 1: Complex inline assembly with conflicts ========== */
block1:
    {
        int tmp1, tmp2, tmp3;
        long tmp4;
        
        /* Inline assembly with conflicting constraints */
        /* Multiple outputs with early clobber to force separate registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[in3], %%rbx\n\t"
            "xorq %[in4], %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            "leal (%[addr1], %[in5]), %%ecx\n\t"
            "movl %%ecx, %[out3]"
            : [out1] "=&r" (tmp1),      /* Early clobber - can't share reg with inputs */
              [out2] "=&r" (tmp4),      /* Early clobber */
              [out3] "=r" (tmp3)        /* Regular output */
            : [in1] "r" (i1),           /* Input in register */
              [in2] "r" (i2),
              [in3] "r" (l1),
              [in4] "r" (l2),
              [in5] "r" (r12_var),
              [addr1] "m" (addr_var1)   /* Memory constraint - address taken */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        i3 = tmp1 + tmp3;
        l1 = tmp4;
        
        /* Create more register pressure */
        v1 = v1 ^ i3;
        v2 = v2 + (int)l1;
    }
    
    /* ========== BLOCK 2: Mixed data types and modes ========== */
    goto block2;  /* Force variables to be live across block boundary */

block2:
    {
        char out_char;
        short out_short;
        int out_int;
        float out_float;
        
        /* Inline assembly with mixed operand sizes */
        asm volatile (
            "movb %[c1], %%al\n\t"
            "addb %[c2], %%al\n\t"
            "movb %%al, %[outc]\n\t"
            "movw %[s1], %%ax\n\t"
            "imulw %[s2], %%ax\n\t"
            "movw %%ax, %[outs]\n\t"
            "movl %[i1], %%ebx\n\t"
            "addl %[i2], %%ebx\n\t"
            "movl %%ebx, %[outi]\n\t"
            "movss %[f1], %%xmm0\n\t"
            "mulss %[f2], %%xmm0\n\t"
            "movss %%xmm0, %[outf]"
            : [outc] "=r" (out_char),
              [outs] "=r" (out_short),
              [outi] "=r" (out_int),
              [outf] "=r" (out_float)
            : [c1] "r" (c1),
              [c2] "r" (c2),
              [s1] "r" (s1),
              [s2] "r" (s2),
              [i1] "r" (i1),
              [i2] "r" (i2),
              [f1] "x" (f1),
              [f2] "x" (f2)
            : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7", "cc"
        );
        
        c3 = out_char;
        s3 = out_short;
        i3 = out_int;
        f1 = out_float;
    }
    
    /* ========== BLOCK 3: Explicit register variable conflict ========== */
    goto block3;

block3:
    {
        int result1, result2;
        
        /* Clobber registers used by explicit register variables */
        asm volatile (
            "movl $0xAAAAAAAA, %%r12d\n\t"  /* Clobber r12 */
            "movl $0xBBBBBBBB, %%r13d\n\t"  /* Clobber r13 */
            "movl $0xCCCCCCCC, %%r14d\n\t"  /* Clobber r14 */
            "movl %%r12d, %[out1]\n\t"
            "movl %%r13d, %[out2]"
            : [out1] "=r" (result1),
              [out2] "=r" (result2)
            : /* no inputs */
            : "r12", "r13", "r14", "rax", "rbx", "rcx", "rdx"
        );
        
        /* Force compiler to reload the explicit register variables */
        r12_var = r12_var + result1;
        r13_var = r13_var ^ result2;
        
        /* Use both memory and register constraints for same variable */
        int temp = 0x88888888;
        asm volatile (
            "movl %[mem], %%eax\n\t"
            "addl %%eax, %[reg]\n\t"
            "movl %[reg], %%eax"
            : [reg] "+r" (temp)          /* Read-write register operand */
            : [mem] "m" (addr_var2)      /* Memory operand */
            : "rax", "cc"
        );
        addr_var2 = temp;
    }
    
    /* ========== BLOCK 4: Complex addressing modes ========== */
    {
        int index = 10;
        int base = 0x1000;
        int displacement;
        
        /* Force complex address calculation */
        asm volatile (
            "movl %[idx], %%eax\n\t"
            "leal (%[base], %%eax, 4), %%ebx\n\t"
            "movl (%%rbx), %%ecx\n\t"
            "addl %%ecx, %[disp]"
            : [disp] "+r" (displacement)
            : [idx] "r" (index),
              [base] "r" (base),
              "m" (*(int*)(uintptr_t)base)  /* Fake memory reference */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        /* More arithmetic to keep values live */
        v3 = v3 + displacement;
        v4 = v4 ^ displacement;
    }
    
    /* ========== FINAL: Aggregate results to prevent DCE ========== */
    checksum = v1 + v2 + v3 + v4 + i1 + i2 + i3 + (int)l1 + (int)l2 + 
               (int)f1 + (int)d1 + c1 + c2 + c3 + s1 + s2 + s3 +
               r12_var + r13_var + r14_var + addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}

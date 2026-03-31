/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;

/* Explicit register variables to force conflicts */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f_local1 = 5.5f, f_local2 = 6.6f;
    double d_local1 = 7.77, d_local2 = 8.88;
    
    /* Take addresses to create addressing mode conflicts */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3, *p4 = &i4;
    float *fp1 = &f_local1, *fp2 = &f_local2;
    
    /* Complex arithmetic to create many live values */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1);
    v7 = v8 ^ v9 | v10 & v1;
    f1 = f2 * f3 - f4 / 2.0f;
    d1 = d2 + d3 * d4;
    
    /* Block 1: First inline assembly with register constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        /* Complex inline assembly with conflicting constraints */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=&r" (tmp1), [out2] "=r" (tmp2)  /* Early clobber and regular output */
            : [in1] "r" (i1), [in2] "r" (i2), [in3] "rm" (i3)  /* Mixed register/memory input */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"  /* Extensive clobber list */
        );
        
        /* Use results to keep them live */
        v1 += tmp1;
        v2 += tmp2;
    }
    
    /* Modify variables to ensure they stay live across blocks */
    i1 = i2 * 2 + r12_var;
    i2 = i3 / 2 + r13_var;
    
    /* Block 2: More complex assembly with memory operands */
block2:
    {
        long result1, result2;
        int addr_temp;
        
        /* Assembly with memory addressing conflicts */
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "addq %[val1], %%rax\n\t"
            "movq %%rax, %[res1]\n\t"
            "leaq (%[addr], %[val2]), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "subq %%rcx, %%rax\n\t"
            "movq %%rax, %[res2]\n\t"
            : [res1] "=r" (result1), [res2] "=r" (result2)
            : [addr] "r" (p1), [val1] "r" (l1), [val2] "r" (sizeof(int))
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
        );
        
        l1 = result1;
        l2 = result2;
    }
    
    /* Force register variable conflicts */
    r12_var = r13_var * 2;
    r13_var = r14_var + r15_var;
    
    /* Block 3: Mixed data type assembly */
block3:
    {
        short s_result;
        char c_result;
        float f_result;
        
        /* Assembly with different data types/modes */
        asm volatile (
            "movw %[s_in], %%ax\n\t"
            "addw $100, %%ax\n\t"
            "movw %%ax, %[s_out]\n\t"
            "movb %[c_in], %%bl\n\t"
            "subb $32, %%bl\n\t"
            "movb %%bl, %[c_out]\n\t"
            "movd %[f_in], %%xmm0\n\t"
            "addss %[f_const], %%xmm0\n\t"
            "movd %%xmm0, %[f_out]\n\t"
            : [s_out] "=r" (s_result), [c_out] "=r" (c_result), [f_out] "=r" (f_result)
            : [s_in] "r" (s1), [c_in] "r" (c1), [f_in] "r" (f_local1),
              [f_const] "X" (1.5f)
            : "rax", "rbx", "xmm0", "xmm1", "memory", "cc"
        );
        
        s1 = s_result;
        c1 = c_result;
        f_local1 = f_result;
    }
    
    /* Block 4: Double precision and more conflicts */
block4:
    {
        double d_result;
        int int_result;
        
        /* Complex constraints that likely need reloads */
        asm volatile (
            "movsd %[d_in], %%xmm2\n\t"
            "mulsd %[d_mul], %%xmm2\n\t"
            "movsd %%xmm2, %[d_out]\n\t"
            "movl %[i_in], %%ebx\n\t"
            "shll $2, %%ebx\n\t"
            "addl %[r12], %%ebx\n\t"  /* Use explicit register variable */
            "movl %%ebx, %[i_out]\n\t"
            : [d_out] "=rm" (d_result), [i_out] "=r" (int_result)  /* Mixed output constraints */
            : [d_in] "rm" (d_local1), [d_mul] "rm" (2.0), 
              [i_in] "rm" (i4), [r12] "r" (r12_var)
            : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
        );
        
        d_local1 = d_result;
        i4 = int_result;
    }
    
    /* Create control flow with goto to extend live ranges */
    if (v1 > v2) {
        goto block1;
    } else if (v3 < v4) {
        goto block2;
    }
    
    /* Final block: One more complex assembly to ensure coverage */
    {
        int final_result;
        /* Assembly that uses all previously modified values */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "addl %[b], %%eax\n\t"
            "addl %[c], %%eax\n\t"
            "addl %[d], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (final_result)
            : [a] "r" (i1), [b] "r" (i2), [c] "r" (i3), [d] "r" (i4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "memory", "cc"
        );
        
        /* Use volatile checksum to prevent optimization */
        v10 = final_result + r12_var + r13_var + r14_var + r15_var;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", v10);
    
    return 0;
}

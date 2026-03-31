/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload mechanism by creating
 * inline assembly patterns that force the register allocator to generate
 * numerous reloads, including secondary reloads. The goal is to trigger
 * the initialization block in push_reload (lines 1381-1399 in reload.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
volatile int volatile_var = 100;

/* Helper functions that return values needing computation */
int compute_index(int base) {
    return (base * 3 + 7) % 16;
}

double compute_double(int i) {
    return i * 2.5;
}

void* compute_address(void* base, int offset) {
    return (char*)base + offset * sizeof(int);
}

/* Test 1: Many operands with mixed constraints to exhaust registers */
int test_many_operands(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int out1, out2, out3, out4, out5;
    double din1 = 1.1, din2 = 2.2;
    double dout1, dout2;
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = 100;
    register int reg_var2 asm ("r13") = 200;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        /* Use explicit register variables */
        "add %%r12, %[out1]\n\t"
        "sub %%r13, %[out2]\n\t"
        /* Memory operand */
        "mov (%[mem]), %[out3]\n\t"
        /* Immediate operand */
        "mov $999, %[out4]\n\t"
        /* Mixed size access */
        "movzwl (%[charptr]), %k[out5]\n\t"
        /* Floating point via integer registers (mode change) */
        "movq %[din1], %%xmm0\n\t"
        "movq %%xmm0, %[dout1]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4),
          [out5] "=r" (out5), [dout1] "=r" (dout1)
        : [in1] "r" (in1), [in2] "r" (in2), 
          [in3] "r" (in3), [in4] "r" (in4),
          [mem] "r" (&global_int),
          [charptr] "r" (global_array + 64),
          [din1] "x" (din1),
          "r" (reg_var1), "r" (reg_var2)  /* explicit register inputs */
        : "memory", "cc", "xmm0", "r12", "r13"
    );
    
    return out1 + out2 + out3 + out4 + out5 + (int)dout1;
}

/* Test 2: Nested function calls in assembly operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        /* Complex address computation */
        "mov (%[addr]), %%ebx\n\t"
        "mov %%ebx, %[res2]\n\t"
        /* Floating point result */
        "movq %[dcall], %%xmm0\n\t"
        "movq %%xmm0, %[dres]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2),
          [dres] "=r" (dresult)
        : [call1] "r" (compute_index(10)),
          [call2] "r" (compute_index(20)),
          [addr] "r" (compute_address(global_array, compute_index(5))),
          [dcall] "x" (compute_double(compute_index(8)))
        : "memory", "cc", "eax", "ebx", "xmm0"
    );
    
    /* Another volatile block with interdependent operands */
    int chain1, chain2;
    __asm__ __volatile__ (
        "mov %[prev], %%ecx\n\t"
        "add $100, %%ecx\n\t"
        "mov %%ecx, %[chain1]\n\t"
        "imul %[vol], %%ecx\n\t"
        "mov %%ecx, %[chain2]\n\t"
        : [chain1] "=r" (chain1), [chain2] "=r" (chain2)
        : [prev] "r" (result1), [vol] "r" (volatile_var)
        : "memory", "cc", "ecx"
    );
    
    return result1 + result2 + (int)dresult + chain1 + chain2;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long long ll1 = 10000000000LL, ll2 = 20000000000LL;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    char cout;
    short sout;
    int iout;
    long long llout;
    float fout;
    double dout;
    
    /* Assembly with operands of different sizes */
    __asm__ __volatile__ (
        /* Char operations */
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "mov %%al, %[cout]\n\t"
        /* Short operations */
        "mov %[s1], %%ax\n\t"
        "sub %[s2], %%ax\n\t"
        "mov %%ax, %[sout]\n\t"
        /* Integer operations */
        "mov %[i1], %%eax\n\t"
        "imul %[i2], %%eax\n\t"
        "mov %%eax, %[iout]\n\t"
        /* Long long operations */
        "mov %[ll1], %%rax\n\t"
        "add %[ll2], %%rax\n\t"
        "mov %%rax, %[llout]\n\t"
        /* Float via integer register (mode change) */
        "movd %[f1], %%xmm0\n\t"
        "movd %%xmm0, %[fout]\n\t"
        /* Double via integer register */
        "movq %[d1], %%xmm1\n\t"
        "movq %%xmm1, %[dout]\n\t"
        : [cout] "=r" (cout), [sout] "=r" (sout),
          [iout] "=r" (iout), [llout] "=r" (llout),
          [fout] "=r" (fout), [dout] "=r" (dout)
        : [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [ll1] "r" (ll1), [ll2] "r" (ll2),
          [f1] "x" (f1), [d1] "x" (d1)
        : "memory", "cc", "rax", "eax", "ax", "al",
          "xmm0", "xmm1"
    );
    
    return cout + sout + iout + (int)llout + (int)fout + (int)dout;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Try to force moves to specific registers */
    register int must_be_eax asm ("eax") = 123;
    register int must_be_ebx asm ("ebx") = 456;
    
    /* Assembly that requires specific registers */
    __asm__ __volatile__ (
        /* Force use of accumulator for multiplication */
        "mov %[val1], %%eax\n\t"
        "imul $100, %%eax\n\t"
        /* Move to another specific register */
        "mov %%eax, %%ebx\n\t"
        "add $50, %%ebx\n\t"
        /* Result in yet another register */
        "mov %%ebx, %[res]\n\t"
        : [res] "=r" (result)
        : [val1] "r" (must_be_eax),
          "a" (must_be_eax), "b" (must_be_ebx)  /* specific constraints */
        : "memory", "cc"
    );
    
    /* Another test with flag register constraints */
    int a = 100, b = 200, cmp_result;
    __asm__ __volatile__ (
        "cmp %[b], %[a]\n\t"
        "setg %[cmp]\n\t"
        : [cmp] "=r" (cmp_result)
        : [a] "r" (a), [b] "r" (b)
        : "memory", "cc"
    );
    
    /* Force memory operand with complex addressing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    int idx1 = compute_index(30);
    int idx2 = compute_index(40);
    int sum;
    
    __asm__ __volatile__ (
        "mov (%[arr], %[idx1], 4), %%eax\n\t"
        "add (%[arr], %[idx2], 4), %%eax\n\t"
        "mov %%eax, %[sum]\n\t"
        : [sum] "=r" (sum)
        : [arr] "r" (array), 
          [idx1] "r" (idx1), 
          [idx2] "r" (idx2)
        : "memory", "cc", "eax"
    );
    
    return result + cmp_result + sum;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(void) {
    /* Declare many local variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4, d5 = 5.5;
    
    int out1, out2, out3, out4, out5;
    double dout1, dout2;
    
    /* Multiple assembly blocks that use many different values */
    __asm__ __volatile__ (
        "mov %[v1], %%eax\n\t"
        "add %[v2], %%eax\n\t"
        "add %[v3], %%eax\n\t"
        "add %[v4], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        
        "mov %[v5], %%ebx\n\t"
        "imul %[v6], %%ebx\n\t"
        "add %[v7], %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        
        "movq %[d1], %%xmm0\n\t"
        "addsd %[d2], %%xmm0\n\t"
        "movq %%xmm0, %[do1]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [do1] "=r" (dout1)
        : [v1] "r" (v1), [v2] "r" (v2), [v3] "r" (v3), [v4] "r" (v4),
          [v5] "r" (v5), [v6] "r" (v6), [v7] "r" (v7),
          [d1] "x" (d1), [d2] "x" (d2)
        : "memory", "cc", "eax", "ebx", "xmm0"
    );
    
    __asm__ __volatile__ (
        "mov %[v8], %%ecx\n\t"
        "sub %[v9], %%ecx\n\t"
        "imul %[v10], %%ecx\n\t"
        "mov %%ecx, %[o3]\n\t"
        
        "mov %[v11], %%edx\n\t"
        "and %[v12], %%edx\n\t"
        "or %[v13], %%edx\n\t"
        "mov %%edx, %[o4]\n\t"
        
        "movq %[d3], %%xmm1\n\t"
        "mulsd %[d4], %%xmm1\n\t"
        "movq %%xmm1, %[do2]\n\t"
        : [o3] "=r" (out3), [o4] "=r" (out4), [do2] "=r" (dout2)
        : [v8] "r" (v8), [v9] "r" (v9), [v10] "r" (v10),
          [v11] "r" (v11), [v12] "r" (v12), [v13] "r" (v13),
          [d3] "x" (d3), [d4] "x" (d4)
        : "memory", "cc", "ecx", "edx", "xmm1"
    );
    
    /* Use all remaining variables in one more operation */
    out5 = v14 + v15 + (int)d5;
    
    return out1 + out2 + out3 + out4 + out5 + (int)dout1 + (int)dout2;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    int checksum = 0;
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_extreme_pressure();
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (checksum)
        : "memory"
    );
    
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}

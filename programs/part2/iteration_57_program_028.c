/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex addressing modes */
int global_array[100];
double global_doubles[50];
char global_chars[256];
volatile int volatile_counter = 0;

/* Function to force computation before assembly */
int compute_index(int seed) {
    volatile_counter++;
    return (seed * 37 + 12345) % 100;
}

double compute_double(int x) {
    return (double)(x * 2.71828);
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60, g = 70, h = 80;
    int out1, out2, out3, out4, out5, out6, out7, out8;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        "movl %[r0], %[out1]\n\t"
        "addl %[a], %[out1]\n\t"
        "movl %[r1], %[out2]\n\t"
        "subl %[b], %[out2]\n\t"
        "movl %[r2], %[out3]\n\t"
        "imull %[c], %[out3]\n\t"
        "movl %[r3], %[out4]\n\t"
        "andl %[d], %[out4]\n\t"
        "movl %[e], %[out5]\n\t"
        "orl %[f], %[out5]\n\t"
        "movl %[g], %[out6]\n\t"
        "xorl %[h], %[out6]\n\t"
        "leal (%[a],%[b],2), %[out7]\n\t"
        "leal (%[c],%[d],4), %[out8]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5), [out6] "=r" (out6),
          [out7] "=r" (out7), [out8] "=r" (out8)
        : [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3),
          [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4 + out5 + out6 + out7 + out8;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int idx1, idx2, idx3;
    
    /* Function calls as operands - forces evaluation before assembly */
    __asm__ __volatile__ (
        "movl %%ebx, %[res1]\n\t"
        "addl %%ecx, %[res1]\n\t"
        "movl %%edx, %[res2]\n\t"
        "subl %%esi, %[res2]\n\t"
        "movl %%edi, %[res3]\n\t"
        "imull %%eax, %[res3]"
        : [res1] "=r" (result1), [res2] "=r" (result2), [res3] "=r" (result3)
        : "b" (compute_index(1)), 
          "c" (compute_index(2)), 
          "d" (compute_index(3)),
          "S" (compute_index(4)), 
          "D" (compute_index(5)), 
          "a" (compute_index(6))
        : "memory", "cc"
    );
    
    /* Complex addressing with function calls */
    __asm__ __volatile__ (
        "movl (%[ptr1]), %[idx1]\n\t"
        "movl (%[ptr2]), %[idx2]\n\t"
        "addl (%[ptr3]), %[idx3]"
        : [idx1] "=r" (idx1), [idx2] "=r" (idx2), [idx3] "=r" (idx3)
        : [ptr1] "r" (&global_array[compute_index(10)]),
          [ptr2] "r" (&global_array[compute_index(20)]),
          [ptr3] "r" (&global_array[compute_index(30)])
        : "memory"
    );
    
    return result1 + result2 + result3 + idx1 + idx2 + idx3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 300000L, l2 = 400000L;
    float f1 = 3.14f;
    double d1 = 2.71828;
    
    int out_char, out_short, out_int, out_long;
    double out_double;
    
    /* Mixed types in same assembly - forces mode conversions */
    __asm__ __volatile__ (
        "movsbl %[ch1], %[outc]\n\t"
        "addw %[sh1], %w[outs]\n\t"
        "movl %[int1], %[outi]\n\t"
        "addq %[long1], %[outl]\n\t"
        /* Force double through integer register */
        "movq %[double1], %%rax\n\t"
        "movq %%rax, %[outd]"
        : [outc] "=r" (out_char), [outs] "=r" (out_short),
          [outi] "=r" (out_int), [outl] "=r" (out_long),
          [outd] "=r" (out_double)
        : [ch1] "r" (c1), [sh1] "r" (s1), [int1] "r" (i1),
          [long1] "r" (l1), [double1] "r" (d1)
        : "rax", "memory", "cc"
    );
    
    /* Cast between types as operands */
    __asm__ __volatile__ (
        "cvtsi2ssl %[intval], %%xmm0\n\t"
        "movd %%xmm0, %[floatout]"
        : [floatout] "=r" (f1)
        : [intval] "r" (i2)
        : "xmm0", "memory"
    );
    
    return out_char + out_short + out_int + (int)out_long + (int)out_double + (int)f1;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int value1 = 12345, value2 = 67890;
    int result1, result2, result3;
    
    /* Force specific register constraints that may need secondary reloads */
    __asm__ __volatile__ (
        "movl %[v1], %%eax\n\t"
        "addl %[v2], %%eax\n\t"
        "movl %%eax, %[r1]\n\t"
        /* Try to force spill/reload */
        "push %%rax\n\t"
        "movl $999, %%eax\n\t"
        "pop %%rax\n\t"
        "movl %%eax, %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [v1] "a" (value1), [v2] "r" (value2)
        : "memory", "cc"
    );
    
    /* Memory operand with complex addressing */
    __asm__ __volatile__ (
        "movl (%[base],%[index],4), %[r3]"
        : [r3] "=r" (result3)
        : [base] "r" (global_array), [index] "r" (compute_index(42))
        : "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int chain[10];
    int sum = 0;
    
    /* Chain of volatile assembly blocks */
    for (int i = 0; i < 10; i++) {
        int temp;
        __asm__ __volatile__ (
            "movl %[prev], %[curr]\n\t"
            "addl $1, %[curr]"
            : [curr] "=r" (chain[i])
            : [prev] "r" (i > 0 ? chain[i-1] : 0)
            : "cc"
        );
        
        /* Memory clobber between operations */
        __asm__ __volatile__ (
            "movl %[val], %[tmp]\n\t"
            "imull $2, %[tmp]"
            : [tmp] "=r" (temp)
            : [val] "r" (chain[i])
            : "memory", "cc"
        );
        
        sum += temp;
    }
    
    /* Final complex operation */
    int final;
    __asm__ __volatile__ (
        "movl %[sum], %%eax\n\t"
        "leal (%%eax,%%eax,2), %%eax\n\t"
        "movl %%eax, %[final]"
        : [final] "=r" (final)
        : [sum] "r" (sum)
        : "eax", "cc"
    );
    
    return final;
}

/* Test 6: Explicit register variables with spills */
int test_explicit_registers(void) {
    register int reg1 asm ("r8") = 111;
    register int reg2 asm ("r9") = 222;
    register int reg3 asm ("r10") = 333;
    register int reg4 asm ("r11") = 444;
    
    int array[4];
    int idx = compute_index(77);
    
    /* Force spills by using all explicit registers */
    __asm__ __volatile__ (
        "movl %[r1], %[a0]\n\t"
        "addl %[idx], %[a0]\n\t"
        "movl %[r2], %[a1]\n\t"
        "subl %[idx], %[a1]\n\t"
        "movl %[r3], %[a2]\n\t"
        "imull %[idx], %[a2]\n\t"
        "movl %[r4], %[a3]\n\t"
        "xorl %[idx], %[a3]"
        : [a0] "=m" (array[0]), [a1] "=m" (array[1]),
          [a2] "=m" (array[2]), [a3] "=m" (array[3])
        : [r1] "r" (reg1), [r2] "r" (reg2), [r3] "r" (reg3), [r4] "r" (reg4),
          [idx] "r" (idx)
        : "memory", "cc"
    );
    
    return array[0] + array[1] + array[2] + array[3];
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 256; i++) {
        global_chars[i] = i;
    }
    
    int checksum = 0;
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    checksum += test_explicit_registers();
    
    /* Final assembly barrier */
    __asm__ __volatile__ ("" : : : "memory");
    
    printf("Checksum: %d\n", checksum);
    
    /* Return deterministic value for verification */
    return (checksum % 256);
}

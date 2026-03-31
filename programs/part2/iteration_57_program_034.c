/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload mechanism by creating
 * complex inline assembly scenarios that force the register allocator
 * to generate numerous reloads, including secondary reloads.
 * 
 * The goal is to trigger the initialization block in push_reload()
 * (lines 1381-1399 in reload.cc) for many reload entries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_char_array[256];
int *global_ptr = &global_int;
volatile int volatile_global = 100;

/* Function that returns a value, forcing evaluation before assembly */
int get_value(int x) {
    return x * 2 + 1;
}

/* Another function with side effects */
double compute_double(int a, double b) {
    volatile_global++;
    return b + (double)a;
}

/* Complex addressing computation */
int* compute_address(int *base, int offset) {
    return base + offset * 2;
}

/* Test 1: Many operands to exhaust registers */
void test_many_operands(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int out1, out2, out3, out4, out5;
    double din1 = 1.1, din2 = 2.2, dout1;
    char cin1 = 'A', cout1;
    
    /* Mixed types and many operands */
    __asm__ __volatile__ (
        "mov %[i1], %[o1]\n\t"
        "add %[i2], %[o1]\n\t"
        "mov %[i3], %[o2]\n\t"
        "imul %[i4], %[o2]\n\t"
        "mov %[i5], %[o3]\n\t"
        "or %[i6], %[o3]\n\t"
        "mov %[i7], %[o4]\n\t"
        "xor %[i8], %[o4]\n\t"
        "mov %[i9], %[o5]\n\t"
        "and %[i10], %[o5]\n\t"
        /* Force some register moves for floating point */
        "mov %k[i1], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %[di1], %%xmm0\n\t"
        "movsd %%xmm0, %[do1]\n\t"
        /* Character manipulation */
        "mov %[ci1], %b[co1]\n\t"
        "addb $1, %b[co1]"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3),
          [o4] "=r" (out4), [o5] "=r" (out5),
          [do1] "=m" (dout1), [co1] "=r" (cout1)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7), [i8] "r" (in8), [i9] "r" (in9),
          [i10] "r" (in10), [di1] "m" (din1), [ci1] "r" (cin1)
        : "eax", "xmm0", "memory", "cc"
    );
}

/* Test 2: Nested function calls in operands */
void test_nested_calls(void) {
    int result1, result2;
    double dresult;
    int *ptr_result;
    
    /* Function calls that must be evaluated before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Use the result in another computation */
        "imul %[global], %%eax\n\t"
        "mov %%eax, %[out2]"
        : [out1] "=m" (result1), [out2] "=m" (result2)
        : [call1] "r" (get_value(10)), 
          [call2] "r" (get_value(20)),
          [global] "m" (global_int)
        : "eax", "memory", "cc"
    );
    
    /* More complex with double computation */
    __asm__ __volatile__ (
        "cvtsi2sd %[val], %%xmm0\n\t"
        "addsd %[dval], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]"
        : [dout] "=m" (dresult)
        : [val] "r" (get_value(5)),
          [dval] "m" (compute_double(3, 2.5))
        : "xmm0", "memory"
    );
    
    /* Pointer arithmetic forcing address computation */
    __asm__ __volatile__ (
        "mov %[addr], %[out]"
        : [out] "=r" (ptr_result)
        : [addr] "r" (compute_address(&global_int, get_value(2)))
        : "memory"
    );
}

/* Test 3: Explicit register variables with constraints */
void test_explicit_registers(void) {
    /* Explicit register variables */
    register int reg_a asm ("r12") = 100;
    register int reg_b asm ("r13") = 200;
    register int reg_c asm ("r14") = 300;
    int out1, out2, out3;
    
    /* Force moves between explicit registers and others */
    __asm__ __volatile__ (
        "mov %[a], %[o1]\n\t"
        "add %[b], %[o1]\n\t"
        "mov %[c], %[o2]\n\t"
        "sub %[a], %[o2]\n\t"
        /* Force a spill/reload by clobbering all registers */
        "mov $0, %%eax\n\t"
        "mov $0, %%ebx\n\t"
        "mov $0, %%ecx\n\t"
        "mov $0, %%edx\n\t"
        "mov $0, %%esi\n\t"
        "mov $0, %%edi\n\t"
        /* Now use the values again */
        "mov %[o1], %[o3]\n\t"
        "add %[o2], %[o3]"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3)
        : [a] "r" (reg_a), [b] "r" (reg_b), [c] "r" (reg_c)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
}

/* Test 4: Mixed data types and mode changes */
void test_mixed_types(void) {
    char c1 = 'X', c2 = 'Y';
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
    
    /* Mixed type operations forcing mode conversions */
    __asm__ __volatile__ (
        /* char operations */
        "mov %[c1], %b[co]\n\t"
        "addb %[c2], %b[co]\n\t"
        /* short operations */
        "mov %[s1], %w[so]\n\t"
        "addw %[s2], %w[so]\n\t"
        /* int operations */
        "mov %[i1], %k[io]\n\t"
        "addl %[i2], %k[io]\n\t"
        /* long long operations - requires two registers on 32-bit */
        "mov %[ll1], %%rax\n\t"
        "add %[ll2], %%rax\n\t"
        "mov %%rax, %[llo]\n\t"
        /* float operations */
        "movss %[f1], %%xmm0\n\t"
        "addss %[f2], %%xmm0\n\t"
        "movss %%xmm0, %[fo]\n\t"
        /* double operations */
        "movsd %[d1], %%xmm1\n\t"
        "addsd %[d2], %%xmm1\n\t"
        "movsd %%xmm1, %[do]"
        : [co] "=r" (cout), [so] "=r" (sout), [io] "=r" (iout),
          [llo] "=m" (llout), [fo] "=m" (fout), [do] "=m" (dout)
        : [c1] "r" (c1), [c2] "r" (c2),
          [s1] "r" (s1), [s2] "r" (s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [ll1] "r" (ll1), [ll2] "r" (ll2),
          [f1] "m" (f1), [f2] "m" (f2),
          [d1] "m" (d1), [d2] "m" (d2)
        : "rax", "xmm0", "xmm1", "memory", "cc"
    );
}

/* Test 5: Secondary reload triggers */
void test_secondary_reloads(void) {
    int value = 12345;
    int result;
    double dvalue = 123.456;
    double dresult;
    
    /* Try to force a secondary reload by using specific constraints */
    __asm__ __volatile__ (
        /* Force value into accumulator, then use it */
        "mov %[val], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out]"
        : [out] "=m" (result)
        : [val] "a" (value)  /* 'a' constraint for accumulator */
        : "memory", "cc"
    );
    
    /* Another attempt with memory constraints */
    __asm__ __volatile__ (
        "mov %[in], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %[din], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]"
        : [dout] "=m" (dresult)
        : [in] "m" (global_int),  /* Memory constraint */
          [din] "m" (dvalue)
        : "eax", "xmm0", "memory"
    );
    
    /* Complex chain of operations */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int r1, r2, r3, r4, r5;
    
    __asm__ __volatile__ (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "mov %[c], %[r2]\n\t"
        "imul %[r1], %[r2]\n\t"
        "mov %[d], %[r3]\n\t"
        "sub %[r2], %[r3]\n\t"
        "mov %[e], %[r4]\n\t"
        "xor %[r3], %[r4]\n\t"
        "mov %[r4], %[r5]\n\t"
        "neg %[r5]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3),
          [r4] "=&r" (r4), [r5] "=r" (r5)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), 
          [d] "r" (d), [e] "r" (e)
        : "memory", "cc"
    );
}

/* Test 6: Array indexing with non-constant offsets */
void test_array_indexing(void) {
    int array[100];
    int i, j, k;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Complex array accesses in assembly */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            int index = i * 10 + j;
            int temp;
            
            /* Force address computation with non-constant offset */
            __asm__ __volatile__ (
                "mov %[idx], %%eax\n\t"
                "mov %[arr](,%%eax,4), %%ebx\n\t"
                "add %[global], %%ebx\n\t"
                "mov %%ebx, %[out]"
                : [out] "=r" (temp)
                : [arr] "m" (*array),  /* Array base */
                  [idx] "r" (index),   /* Non-constant index */
                  [global] "r" (global_int)
                : "eax", "ebx", "memory", "cc"
            );
            
            sum += temp;
        }
    }
    
    /* Use sum to prevent dead code elimination */
    __asm__ __volatile__ (
        "add %[val], %[sum]"
        : [sum] "+r" (sum)
        : [val] "r" (volatile_global)
        : "cc"
    );
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    memset(global_char_array, 'A', sizeof(global_char_array));
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests multiple times to increase pressure */
    for (int i = 0; i < 3; i++) {
        test_many_operands();
        test_nested_calls();
        test_explicit_registers();
        test_mixed_types();
        test_secondary_reloads();
        test_array_indexing();
        
        /* Add to checksum to ensure code isn't optimized away */
        checksum += global_int + volatile_global + i;
    }
    
    printf("Tests completed. Checksum: %d\n", checksum);
    
    /* Return deterministic value for testing */
    return checksum == 0 ? 1 : 0;
}

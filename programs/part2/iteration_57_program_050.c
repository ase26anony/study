/* reload-stress.c
 * A program designed to stress GCC's reload mechanism and trigger
 * initialization of new reload entries in push_reload().
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -funroll-loops -fno-optimize-sibling-calls -march=x86-64 -mno-sse -mno-avx reload-stress.c -o reload-stress
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
char global_char_array[256];

/* Function that returns a value, forcing evaluation before assembly */
int get_value(int x) {
    return x * 2 + 1;
}

/* Another function with side effects */
double compute_double(int a, int b) {
    return (double)(a + b) / 2.0;
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int out1, out2, out3, out4;
    
    /* Complex inline assembly with many input/output operands */
    __asm__ __volatile__ (
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "imul %[c], %[out1]\n\t"
        "mov %[d], %[out2]\n\t"
        "sub %[e], %[out2]\n\t"
        "mov %[f], %[out3]\n\t"
        "xor %[g], %[out3]\n\t"
        "mov %[h], %[out4]\n\t"
        "or %[i], %[out4]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "m" (j), [k] "m" (k), [l] "m" (l),
          [m] "m" (m), [n] "m" (n), [o] "m" (o), [p] "m" (p)
        : "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int x = 100, y = 200, z = 300;
    int result1, result2;
    
    /* Function calls as operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%eax\n\t"
        : "=a" (result1)
        : "a" (get_value(x)), 
          "b" (get_value(y)), 
          "c" (get_value(z))
        : "memory"
    );
    
    /* Complex addressing with array indexing */
    __asm__ __volatile__ (
        "movl (%[ptr]), %%eax\n\t"
        "addl %[idx], %%eax\n\t"
        : "=a" (result2)
        : [ptr] "r" (&global_array[get_value(x) % 50]),
          [idx] "i" (global_int)
        : "memory"
    );
    
    return result1 + result2;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 1000000L, l2 = 2000000L;
    float f1 = 1.5f;
    double d1 = 2.71828;
    int out_int;
    char out_char;
    
    /* Mixing different sized operands */
    __asm__ __volatile__ (
        "movsbl %[c1], %%eax\n\t"
        "addw %[s1], %%ax\n\t"
        "addl %[i1], %%eax\n\t"
        "movq %[l1], %%rcx\n\t"
        "addq %%rcx, %%rax\n\t"
        : "=a" (out_int)
        : [c1] "r" (c1), [s1] "r" (s1), [i1] "r" (i1), [l1] "r" (l1)
        : "rcx", "memory"
    );
    
    /* Using float/double in integer assembly (forcing mode changes) */
    int float_as_int = *(int*)&f1;
    long double_as_long = *(long*)&d1;
    
    __asm__ __volatile__ (
        "movl %[f], %%eax\n\t"
        "addl %[i2], %%eax\n\t"
        "movq %[d], %%rcx\n\t"
        "shrq $32, %%rcx\n\t"
        "addl %%ecx, %%eax\n\t"
        : "=a" (out_int)
        : [f] "r" (float_as_int), [i2] "r" (i2), [d] "r" (double_as_long)
        : "rcx", "memory"
    );
    
    return out_int;
}

/* Test 4: Explicit register variables and secondary reload triggers */
int test_register_vars(void) {
    /* Explicit register variables */
    register int r12_var asm ("r12") = 0x12345678;
    register int r13_var asm ("r13") = 0x87654321;
    register int r14_var asm ("r14") = 0xABCDEF01;
    int out1, out2, out3;
    
    /* Force moves between specific registers */
    __asm__ __volatile__ (
        "mov %%r12, %%rax\n\t"
        "add %%r13, %%rax\n\t"
        "mov %%rax, %%rbx\n\t"
        "sub %%r14, %%rbx\n\t"
        : "=a" (out1), "=b" (out2)
        : 
        : "r12", "r13", "r14", "memory"
    );
    
    /* Try to force accumulator-specific constraint */
    int acc_val = 999;
    __asm__ __volatile__ (
        "add $1, %%eax\n\t"
        : "+a" (acc_val)
        : 
        : "memory", "cc"
    );
    
    /* Memory constraint with complex addressing */
    __asm__ __volatile__ (
        "movl $0, %%eax\n\t"
        "1:\n\t"
        "addl (%[base], %%rax, 4), %%ebx\n\t"
        "inc %%eax\n\t"
        "cmp $10, %%eax\n\t"
        "jl 1b\n\t"
        : "=b" (out3)
        : [base] "r" (global_array), "b" (0)
        : "rax", "memory", "cc"
    );
    
    return out1 + out2 + out3 + acc_val;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int tmp1, tmp2, tmp3, final;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "mov %[a], %[t1]\n\t"
        "imul %[b], %[t1]\n\t"
        : [t1] "=r" (tmp1)
        : [a] "r" (a), [b] "r" (b)
        : "memory"
    );
    
    /* Chain 2 (depends on chain 1) */
    __asm__ __volatile__ (
        "mov %[t1], %[t2]\n\t"
        "add %[c], %[t2]\n\t"
        : [t2] "=r" (tmp2)
        : [t1] "r" (tmp1), [c] "r" (c)
        : "memory"
    );
    
    /* Chain 3 (depends on chain 2) */
    __asm__ __volatile__ (
        "mov %[t2], %[t3]\n\t"
        "sub %[d], %[t3]\n\t"
        : [t3] "=r" (tmp3)
        : [t2] "r" (tmp2), [d] "r" (d)
        : "memory"
    );
    
    /* Final operation using all temporaries */
    __asm__ __volatile__ (
        "mov %[t1], %[f]\n\t"
        "add %[t2], %[f]\n\t"
        "add %[t3], %[f]\n\t"
        : [f] "=r" (final)
        : [t1] "r" (tmp1), [t2] "r" (tmp2), [t3] "r" (tmp3)
        : "memory"
    );
    
    return final;
}

/* Test 6: Complex addressing modes with pointer arithmetic */
int test_complex_addressing(void) {
    int index = global_int;
    int offset = get_value(index);
    int *ptr1 = &global_array[0];
    int *ptr2 = &global_array[50];
    int result1, result2;
    
    /* Complex address calculation */
    __asm__ __volatile__ (
        "movl (%[base], %[idx], 4), %%eax\n\t"
        "addl (%[base], %[off], 4), %%eax\n\t"
        : "=a" (result1)
        : [base] "r" (global_array),
          [idx] "r" (index),
          [off] "r" (offset)
        : "memory"
    );
    
    /* Pointer difference calculation */
    __asm__ __volatile__ (
        "mov %[p2], %%rax\n\t"
        "sub %[p1], %%rax\n\t"
        "sar $2, %%rax\n\t"  /* Divide by sizeof(int) */
        : "=a" (result2)
        : [p1] "r" (ptr1), [p2] "r" (ptr2)
        : "memory"
    );
    
    return result1 + result2;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_register_vars();
    checksum += test_volatile_chains();
    checksum += test_complex_addressing();
    
    /* Print checksum for verification */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return lower byte */
}

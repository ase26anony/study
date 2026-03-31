/* reload_stress_test.c
 * Designed to trigger GCC's reload pass and hit push_reload initialization
 * Target: lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function that returns values forcing register allocation */
int func_return_int(int x) {
    return x * 2 + 1;
}

double func_return_double(double x) {
    return x * 1.5;
}

int* func_return_ptr(int *p) {
    return p + 1;
}

/* Complex addressing computation */
size_t compute_offset(int idx) {
    return (idx * 7 + 3) % 256;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5;
    int in0 = 10, in1 = 20, in2 = 30, in3 = 40, in4 = 50, in5 = 60;
    double d0 = 1.1, d1 = 2.2;
    char c0 = 'A', c1 = 'B';
    short s0 = 100, s1 = 200;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in0], %[out0]\n\t"
        "add %[in1], %[out1]\n\t"
        "imul %[in2], %[out2]\n\t"
        /* Force register shuffling */
        "mov %[r0], %%eax\n\t"
        "add %[r1], %%eax\n\t"
        "mov %%eax, %[out3]\n\t"
        /* Mixed size operations */
        "movsx %[c0], %[out4]\n\t"
        "movsx %[s0], %[out5]"
        : [out0] "=r" (out0), 
          [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5)
        : [in0] "r" (in0),
          [in1] "r" (in1),
          [in2] "r" (in2),
          [r0] "r" (r0),
          [r1] "r" (r1),
          [c0] "r" ((int)c0),
          [s0] "r" ((int)s0)
        : "eax", "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    int *ptr_result;
    
    /* Function calls directly in asm operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %%eax, %[res1]\n\t"
        "mov %%ebx, %[res2]\n\t"
        "mov %%ecx, %[res3]"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=r" (result3)
        : "a" (func_return_int(global_int)),
          "b" ((int)func_return_double(global_double)),
          "c" ((int)(func_return_ptr(global_ptr) - global_ptr))
        : "memory"
    );
    
    /* Mixed types with complex addressing */
    int idx = global_int;
    __asm__ __volatile__ (
        "mov (%[ptr], %[idx], 4), %%eax\n\t"
        "add %%eax, %[sum]"
        : [sum] "+r" (result1)
        : [ptr] "r" (global_ptr),
          [idx] "r" (compute_offset(idx))
        : "eax", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'X', c2 = 'Y';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000;
    long l1 = 100000, l2 = 200000;
    float f1 = 1.5, f2 = 2.5;
    double d1 = 3.14159, d2 = 2.71828;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Force mode changes through casts in constraints */
    __asm__ __volatile__ (
        /* Integer operations with different sizes */
        "add %[i1], %[i2]\n\t"
        "mov %[i2], %[out_i]\n\t"
        /* Float to int conversion */
        "cvttsd2si %[d1], %%eax\n\t"
        "add %%eax, %[out_i]"
        : [out_i] "=r" (out_int)
        : [i1] "r" (i1),
          [i2] "r" (i2),
          [d1] "x" (d1)
        : "eax", "memory"
    );
    
    /* Mixed constraints forcing reloads */
    __asm__ __volatile__ (
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "movsx %%al, %[out_c]"
        : [out_c] "=r" (out_char)
        : [c1] "i" ((int)c1),
          [c2] "r" ((int)c2)
        : "al", "memory"
    );
    
    return out_int + out_char;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int value = 0x12345678;
    double dvalue = 123.456;
    int result;
    
    /* Try to force secondary reloads with specific register constraints */
    __asm__ __volatile__ (
        /* Force value through accumulator */
        "mov %[val], %%eax\n\t"
        "shl $4, %%eax\n\t"
        "mov %%eax, %[res]"
        : [res] "=r" (result)
        : [val] "r" (value)
        : "eax", "memory", "cc"
    );
    
    /* Complex memory operand with index */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    int idx1 = 10, idx2 = 20;
    __asm__ __volatile__ (
        "mov (%[arr], %[idx1], 4), %%eax\n\t"
        "add (%[arr], %[idx2], 4), %%eax\n\t"
        "mov %%eax, %[res]"
        : [res] "=r" (result)
        : [arr] "r" (array),
          [idx1] "r" (idx1),
          [idx2] "r" (idx2)
        : "eax", "memory"
    );
    
    return result;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x, y, z;
    
    /* Chain of volatile asm statements */
    __asm__ __volatile__ (
        "mov %[a], %[x]\n\t"
        "add %[b], %[x]"
        : [x] "=r" (x)
        : [a] "r" (a),
          [b] "r" (b)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[x], %[y]\n\t"
        "imul %[c], %[y]\n\t"
        "add %[d], %[y]"
        : [y] "=r" (y)
        : [x] "r" (x),
          [c] "r" (c),
          [d] "r" (d)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[y], %[z]\n\t"
        "sub %[e], %[z]\n\t"
        "neg %[z]"
        : [z] "=r" (z)
        : [y] "r" (y),
          [e] "r" (e)
        : "memory", "cc"
    );
    
    return z;
}

/* Test 6: Complex addressing with pointer arithmetic */
int test_complex_addressing(void) {
    int array[256];
    int *ptr = array;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex addressing in asm */
    for (int i = 0; i < 10; i++) {
        int offset = i * 7;
        __asm__ __volatile__ (
            "mov (%[base], %[off], 4), %%eax\n\t"
            "add %%eax, %[sum]"
            : [sum] "+r" (sum)
            : [base] "r" (ptr),
              [off] "r" (offset)
            : "eax", "memory"
        );
    }
    
    /* More complex: base + index * scale + displacement */
    __asm__ __volatile__ (
        "mov 16(%[base], %[idx], 4), %%eax\n\t"
        "add %%eax, %[sum]"
        : [sum] "+r" (sum)
        : [base] "r" (ptr),
          [idx] "r" (5)
        : "eax", "memory"
    );
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    checksum += test_complex_addressing();
    
    /* Final assembly barrier */
    __asm__ __volatile__ (
        "mov %[sum], %%eax\n\t"
        "add $42, %%eax"
        : 
        : [sum] "r" (checksum)
        : "eax", "memory"
    );
    
    return checksum;
}

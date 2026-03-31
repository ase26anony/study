/* reload_stress.c - Stress test for GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_char = 'A';
int global_array[100] = {0};
volatile int volatile_var = 0;

/* Function to force computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int result = 0;
    
    /* Force all these into registers with complex constraints */
    __asm__ __volatile__ (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0\n\t"
        "addl %11, %0\n\t"
        "addl %12, %0\n\t"
        "addl %13, %0\n\t"
        "addl %14, %0\n\t"
        "addl %15, %0\n\t"
        "addl %16, %0"
        : "+r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h),
          "r" (i), "r" (j), "r" (k), "r" (l), "r" (m), "r" (n), "r" (o), "r" (p)
        : "cc", "memory"
    );
    
    return result;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'x';
    short s1 = 1000;
    int i1 = 50000;
    long l1 = 1000000L;
    float f1 = 2.5f;
    double d1 = 3.14159;
    int result = 0;
    
    /* Force mode changes and register class conflicts */
    __asm__ __volatile__ (
        "movsbl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "addl %3, %%eax\n\t"
        "addq %4, %%rax\n\t"
        "cvttss2sil %5, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cvttsd2sil %6, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (c1), "r" (s1), "r" (i1), "r" (l1), "r" (f1), "r" (d1)
        : "rax", "rbx", "cc", "memory"
    );
    
    return result;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int result = 0;
    int temp1, temp2, temp3;
    
    /* Function calls that must be evaluated into registers */
    __asm__ __volatile__ (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (compute_value(global_int)),
          "r" (compute_value(global_int + 1)),
          "r" (compute_value(global_int * 2))
        : "rax", "cc", "memory"
    );
    
    /* More complex with pointer arithmetic */
    int idx = global_int % 50;
    __asm__ __volatile__ (
        "movl (%1,%2,4), %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (result)
        : "r" (global_array), "r" (idx)
        : "rax", "cc", "memory"
    );
    
    return result;
}

/* Test 4: Explicit register variables */
int test_explicit_registers(void) {
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    register int r14_var asm ("r14") = 300;
    register int r15_var asm ("r15") = 400;
    int result = 0;
    
    /* Force moves between specific registers */
    __asm__ __volatile__ (
        "movl %%r12d, %%eax\n\t"
        "addl %%r13d, %%eax\n\t"
        "addl %%r14d, %%eax\n\t"
        "addl %%r15d, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : 
        : "rax", "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    /* Try to force secondary reloads */
    __asm__ __volatile__ (
        "testl %1, %1\n\t"
        "setnz %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result)
        : "r" (r12_var)
        : "rax", "cc", "memory"
    );
    
    return result;
}

/* Test 5: Complex addressing modes */
int test_complex_addressing(void) {
    int result = 0;
    int base = 100;
    int offset1 = global_int;
    int offset2 = compute_value(global_int);
    
    /* Complex addressing that may need reloads */
    __asm__ __volatile__ (
        "leal (%1,%2,2), %%eax\n\t"
        "addl (%3,%%eax,4), %0"
        : "+r" (result)
        : "r" (base), "r" (offset1), "r" (global_array)
        : "rax", "cc", "memory"
    );
    
    /* Memory operand with complex computation */
    __asm__ __volatile__ (
        "imull %2, %1\n\t"
        "addl (%3,%1,4), %0"
        : "+r" (result), "+r" (offset2)
        : "r" (global_int), "r" (global_array)
        : "cc", "memory"
    );
    
    return result;
}

/* Test 6: Floating point through integer registers */
int test_fp_reloads(void) {
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    int result = 0;
    
    /* Force float/double through integer registers */
    __asm__ __volatile__ (
        "movd %1, %%xmm0\n\t"
        "movd %2, %%xmm1\n\t"
        "addss %%xmm1, %%xmm0\n\t"
        "movd %%xmm0, %0"
        : "=r" (result)
        : "r" (*(int*)&f1), "r" (*(int*)&f2)
        : "xmm0", "xmm1", "cc", "memory"
    );
    
    /* Mixed precision */
    __asm__ __volatile__ (
        "cvtsi2sd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "cvtsd2si %%xmm0, %0"
        : "=r" (result)
        : "r" (global_int), "r" (*(long long*)&d1)
        : "xmm0", "cc", "memory"
    );
    
    return result;
}

/* Test 7: Volatile chains with interdependencies */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int r1, r2, r3, r4;
    
    /* Chain of volatile asm with dependencies */
    __asm__ __volatile__ (
        "movl %1, %0\n\t"
        "addl $10, %0"
        : "=r" (r1)
        : "r" (a)
        : "cc", "memory"
    );
    
    __asm__ __volatile__ (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=r" (r2), "+r" (r1)
        : "r" (b)
        : "cc", "memory"
    );
    
    __asm__ __volatile__ (
        "imull %2, %1\n\t"
        "leal (%1,%3,2), %0"
        : "=r" (r3), "+r" (r2)
        : "r" (c), "r" (r1)
        : "cc", "memory"
    );
    
    __asm__ __volatile__ (
        "xorl %2, %1\n\t"
        "orl %3, %1\n\t"
        "movl %1, %0"
        : "=r" (r4), "+r" (r3)
        : "r" (d), "r" (r2)
        : "cc", "memory"
    );
    
    return r4;
}

/* Test 8: Immediate constraints and memory operands */
int test_immediate_memory(void) {
    int result = 0;
    int mem_var;
    
    /* Mix of immediate, register, and memory constraints */
    __asm__ __volatile__ (
        "movl $0x12345678, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "i" (100), "m" (global_int)
        : "rax", "cc", "memory"
    );
    
    /* Memory output operand */
    __asm__ __volatile__ (
        "movl %1, %%eax\n\t"
        "addl $42, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (mem_var)
        : "r" (result)
        : "rax", "cc", "memory"
    );
    
    return result + mem_var;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_complex_addressing();
    checksum += test_fp_reloads();
    checksum += test_volatile_chains();
    checksum += test_immediate_memory();
    
    /* Use the result to prevent optimization */
    volatile_var = checksum;
    
    return checksum % 256;
}

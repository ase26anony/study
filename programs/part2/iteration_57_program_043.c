/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int* global_ptr = NULL;

/* Helper functions for nested calls */
int func1(int x) { return x * 2; }
int func2(int x) { return x + 1; }
double func3(double x) { return x * 2.0; }
int* func4(int* p) { return p + 1; }

/* Complex addressing computation */
int compute_index(int i) {
    return (i * 7 + 3) % 256;
}

/* Test 1: Many operands with mixed constraints */
void test1_many_operands() {
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A';
    short s1 = 12345;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        "movl %[i1], %%eax\n\t"
        "addl %[i2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "imull %[i3], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "leal (%[i4], %[r1], 4), %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        "movl %[r2], %[o4]"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3), [o4] "=r" (out4)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3), [i4] "i" (1000),
          [r1] "r" (r1), [r2] "r" (r2)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Force use of outputs to prevent optimization */
    global_int += out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
void test2_nested_calls() {
    int result1, result2, result3;
    double dresult;
    
    /* Function calls as input operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "cvtsi2sd %1, %%xmm0\n\t"
        "movq %%xmm0, %2"
        : "=r" (result1), "=r" (result2), "=m" (dresult)
        : "r" (func1(global_int)), "r" (func2(global_int + 1)),
          "i" (func3(global_double)), "m" (global_array[compute_index(10)])
        : "eax", "ebx", "xmm0", "memory"
    );
    
    /* Chain of volatile asm with dependencies */
    int temp = result1;
    __asm__ __volatile__ (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result3)
        : "r" (temp + func1(func2(global_int)))
        : "eax", "memory"
    );
    
    global_int += result1 + result2 + result3;
    global_double += dresult;
}

/* Test 3: Mixed data types and mode changes */
void test3_mixed_types() {
    char c1 = 'X', c2, c3;
    short s1 = 1000, s2;
    int i1 = 50000, i2, i3;
    long l1 = 1000000, l2;
    float f1 = 3.14f;
    double d1 = 2.71828, d2;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        "movsbl %[c], %%eax\n\t"
        "addw %[s], %%ax\n\t"
        "addl %[i], %%eax\n\t"
        "movl %%eax, %[oi]\n\t"
        "movq %[d], %%xmm0\n\t"
        "cvtsd2ss %%xmm0, %%xmm1\n\t"
        "movss %%xmm1, %[of]"
        : [oi] "=r" (i2), [of] "=m" (f1)
        : [c] "r" ((int)c1), [s] "r" ((int)s1), [i] "r" (i1), [d] "r" (d1)
        : "eax", "xmm0", "xmm1", "memory"
    );
    
    /* Explicit casts forcing mode changes */
    __asm__ __volatile__ (
        "cvtsi2sd %[li], %%xmm0\n\t"
        "movq %%xmm0, %[od]"
        : [od] "=m" (d2)
        : [li] "r" ((int)l1)
        : "xmm0", "memory"
    );
    
    /* Using same variable with different modes */
    __asm__ __volatile__ (
        "movb %[ic], %%al\n\t"
        "movb %%al, %[oc]"
        : [oc] "=r" (c3)
        : [ic] "r" ((int)i2)
        : "al", "memory"
    );
    
    global_int += i2 + (int)f1 + (int)d2 + c3;
}

/* Test 4: Complex addressing modes */
void test4_complex_addressing() {
    int array[100];
    int* ptr = array;
    int index1, index2, result;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Complex addressing in operands */
    __asm__ __volatile__ (
        "movl (%[base], %[idx1], 4), %%eax\n\t"
        "addl (%[base], %[idx2], 4), %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r" (result)
        : [base] "r" (ptr), 
          [idx1] "r" (compute_index(5)),  /* Function call in index */
          [idx2] "r" (global_int % 50)    /* Global variable in index */
        : "eax", "memory"
    );
    
    /* Pointer arithmetic forcing address computation */
    int* new_ptr;
    __asm__ __volatile__ (
        "leal (%[ptr], %[off], 4), %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (new_ptr)
        : [ptr] "r" (ptr), [off] "r" (func1(global_int) % 25)
        : "eax", "memory"
    );
    
    /* Access through computed pointer */
    int value;
    __asm__ __volatile__ (
        "movl (%[ptr]), %%eax\n\t"
        "movl %%eax, %[val]"
        : [val] "=r" (value)
        : [ptr] "r" (func4(new_ptr))  /* Nested function call */
        : "eax", "memory"
    );
    
    global_int += result + value;
}

/* Test 5: Secondary reload triggers */
void test5_secondary_reloads() {
    register int a_reg asm ("eax") = 1;
    register int d_reg asm ("edx") = 2;
    int result1, result2;
    
    /* Force use of specific registers with conflicting constraints */
    __asm__ __volatile__ (
        "movl %[a], %%eax\n\t"
        "movl %[d], %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %%eax, %[out1]"
        : [out1] "=r" (result1)
        : [a] "a" (a_reg), [d] "d" (d_reg)
        : "memory"
    );
    
    /* Try to force a value into flags register indirectly */
    int cmp1 = 10, cmp2 = 20;
    int flag_result;
    
    __asm__ __volatile__ (
        "cmpl %[c2], %[c1]\n\t"
        "setg %%al\n\t"
        "movzbl %%al, %[out]"
        : [out] "=r" (flag_result)
        : [c1] "r" (cmp1), [c2] "r" (cmp2)
        : "eax", "cc", "memory"
    );
    
    /* Memory constraint forcing load/store */
    int mem_var = 999;
    __asm__ __volatile__ (
        "movl %[in], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (mem_var)
        : [in] "m" (mem_var)
        : "eax", "memory"
    );
    
    global_int += result1 + flag_result + mem_var;
}

/* Test 6: Maximum register pressure */
void test6_register_pressure() {
    /* Declare many register variables */
    register int v1 asm ("r8") = 1;
    register int v2 asm ("r9") = 2;
    register int v3 asm ("r10") = 3;
    register int v4 asm ("r11") = 4;
    
    int o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    
    /* Huge asm statement with many operands */
    __asm__ __volatile__ (
        "movl %[a1], %%eax\n\t"
        "addl %[a2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[a3], %%ebx\n\t"
        "subl %[a4], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "movl %[a5], %%ecx\n\t"
        "imull %[a6], %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        "movl %[a7], %%edx\n\t"
        "andl %[a8], %%edx\n\t"
        "movl %%edx, %[o4]\n\t"
        "movl %[a9], %%esi\n\t"
        "orl %[a10], %%esi\n\t"
        "movl %%esi, %[o5]"
        : [o1] "=r" (o1), [o2] "=r" (o2), [o3] "=r" (o3),
          [o4] "=r" (o4), [o5] "=r" (o5)
        : [a1] "r" (i1), [a2] "r" (i2), [a3] "r" (i3), [a4] "r" (i4),
          [a5] "r" (i5), [a6] "r" (i6), [a7] "r" (i7), [a8] "r" (i8),
          [a9] "r" (i9), [a10] "r" (i10),
          "r" (v1), "r" (v2), "r" (v3), "r" (v4)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    /* Second asm block using outputs from first */
    __asm__ __volatile__ (
        "leal (%[x1], %[x2], 2), %%eax\n\t"
        "addl %[x3], %%eax\n\t"
        "addl %[x4], %%eax\n\t"
        "addl %[x5], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (o6)
        : [x1] "r" (o1), [x2] "r" (o2), [x3] "r" (o3),
          [x4] "r" (o4), [x5] "r" (o5)
        : "eax", "memory"
    );
    
    global_int += o1 + o2 + o3 + o4 + o5 + o6;
}

int main() {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    global_ptr = &global_int;
    
    /* Run all tests multiple times to increase reload opportunities */
    for (int iteration = 0; iteration < 3; iteration++) {
        test1_many_operands();
        test2_nested_calls();
        test3_mixed_types();
        test4_complex_addressing();
        test5_secondary_reloads();
        test6_register_pressure();
        
        checksum += global_int + (int)global_double;
    }
    
    /* Final computation using all modified globals */
    checksum += global_int * 2 + (int)(global_double * 100);
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256;  /* Return deterministic value */
}

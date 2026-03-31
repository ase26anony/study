/* reload_stress_test.c
 * 
 * This program is designed to stress GCC's reload pass by creating inline
 * assembly patterns that force the register allocator to generate many
 * reloads, including secondary reloads. The goal is to trigger the
 * initialization block in reload.cc (lines 1381-1399) for multiple reload
 * entries with varied parameters.
 *
 * Compilation recommendations:
 *   gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 *   gcc -O3 -funroll-loops -fno-optimize-sibling-calls reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_char_array[256];
int global_int_array[100];
volatile int volatile_global = 100;

/* Function that returns a value, used in assembly operands */
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
    
    /* Use explicit register variables to force specific register allocation */
    register int reg_var1 asm ("r12") = a + b;
    register int reg_var2 asm ("r13") = c + d;
    register int reg_var3 asm ("r14") = e + f;
    register int reg_var4 asm ("r15") = g + h;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        "mov %[rv1], %%eax\n\t"
        "add %[rv2], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "imul %[rv3], %[o2]\n\t"
        "lea (%[rv4], %[i1], 2), %[o3]\n\t"
        "or %[i2], %[o4]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3), [o4] "=r" (out4)
        : [rv1] "r" (reg_var1), [rv2] "r" (reg_var2), [rv3] "r" (reg_var3),
          [rv4] "r" (reg_var4), [i1] "r" (i), [i2] "r" (j),
          "m" (global_int_array[10]), "m" (global_int_array[20])
        : "eax", "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in assembly operands */
int test_nested_calls(void) {
    int x = 5, y = 10, z = 15;
    int result1, result2;
    double dbl_result;
    
    /* Function calls as operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %[call1], %%ebx\n\t"
        "add %[call2], %%ebx\n\t"
        "mov %%ebx, %[r1]\n\t"
        "mov %[gint], %%ecx\n\t"
        "sub %%ecx, %[r2]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [call1] "r" (get_value(x)), [call2] "r" (get_value(y)),
          [gint] "m" (global_int)
        : "ebx", "ecx", "memory", "cc"
    );
    
    /* Mixed types with function call */
    __asm__ __volatile__ (
        "fldl %[dbl]\n\t"
        "fistpl %[out]\n\t"
        : [out] "=m" (result2)
        : [dbl] "m" (compute_double(z, global_int))
        : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    return result1 + result2;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long long ll1 = 5000000000LL, ll2 = 6000000000LL;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Force mode changes by using different sized operands */
    __asm__ __volatile__ (
        "movsbl %[ch1], %%eax\n\t"
        "addw %[sh1], %%ax\n\t"
        "addl %[in1], %%eax\n\t"
        "mov %%eax, %[oi]\n\t"
        "mov %[ch2], %[oc]\n\t"
        : [oi] "=r" (out_int), [oc] "=r" (out_char)
        : [ch1] "r" (c1), [sh1] "r" (s1), [in1] "r" (i1), [ch2] "r" (c2)
        : "eax", "cc"
    );
    
    /* Floating point forced through integer registers (with -mno-sse) */
    __asm__ __volatile__ (
        "fldl %[dbl1]\n\t"
        "faddl %[dbl2]\n\t"
        "fstpl %[od]\n\t"
        : [od] "=m" (out_double)
        : [dbl1] "m" (d1), [dbl2] "m" (d2)
        : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Cast double to int for mode change */
    int double_as_int = (int)d1;
    __asm__ __volatile__ (
        "addl %[dai], %[oi]\n\t"
        : [oi] "+r" (out_int)
        : [dai] "r" (double_as_int)
        : "cc"
    );
    
    return out_int + out_char + (int)out_double;
}

/* Test 4: Complex addressing modes with pointer arithmetic */
int test_complex_addressing(void) {
    int array[50];
    int *ptr1 = &array[10];
    int *ptr2 = &array[20];
    int index = volatile_global % 30;
    int result1, result2, result3;
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        array[i] = i * 2;
    }
    
    /* Complex addressing in operands */
    __asm__ __volatile__ (
        "mov (%[p1], %[idx], 4), %%eax\n\t"
        "add (%[p2]), %%eax\n\t"
        "mov %%eax, %[r1]\n\t"
        "lea (%[p1], %[idx], 2), %%ebx\n\t"
        "mov (%%ebx), %[r2]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [p1] "r" (ptr1), [p2] "r" (ptr2), [idx] "r" (index)
        : "eax", "ebx", "memory", "cc"
    );
    
    /* More complex: array indexing with function call */
    __asm__ __volatile__ (
        "mov %[call], %%ecx\n\t"
        "mov (%[arr], %%ecx, 4), %[r3]\n\t"
        : [r3] "=r" (result3)
        : [arr] "r" (array), [call] "r" (get_value(index) % 50)
        : "ecx", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 5: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2;
    
    /* Try to force moves between register classes */
    __asm__ __volatile__ (
        /* Force use of specific registers that might need secondary reloads */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[in3], %%ebx\n\t"
        "imul %%eax, %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c)
        : "eax", "ebx", "cc"
    );
    
    /* Memory constraint that might need secondary reload */
    int mem_var = 999;
    __asm__ __volatile__ (
        "lock xaddl %[val], %[mem]\n\t"
        : [mem] "+m" (mem_var)
        : [val] "r" (out1)
        : "memory", "cc"
    );
    
    /* Immediate constraint mixed with register constraints */
    __asm__ __volatile__ (
        "add $0x1234, %[out]\n\t"
        : [out] "+r" (out2)
        :
        : "cc"
    );
    
    return out1 + out2 + mem_var;
}

/* Test 6: Chain of volatile assembly blocks with interdependent operands */
int test_chain_of_reloads(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int t1, t2, t3, t4, t5;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "mov %[in1], %[tmp1]\n\t"
        "add %[in2], %[tmp1]\n\t"
        : [tmp1] "=r" (t1)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "cc"
    );
    
    /* Chain 2 - depends on result of chain 1 */
    __asm__ __volatile__ (
        "imul %[tmp1], %[in3]\n\t"
        "mov %[in3], %[tmp2]\n\t"
        : [tmp2] "=r" (t2)
        : [tmp1] "r" (t1), [in3] "r" (v3)
        : "cc"
    );
    
    /* Chain 3 - memory clobber between operations */
    __asm__ __volatile__ (
        "mov %[tmp2], %%eax\n\t"
        "add %[in4], %%eax\n\t"
        "mov %%eax, %[tmp3]\n\t"
        : [tmp3] "=r" (t3)
        : [tmp2] "r" (t2), [in4] "r" (v4)
        : "eax", "memory", "cc"
    );
    
    /* Chain 4 - function call operand */
    __asm__ __volatile__ (
        "add %[call], %[tmp3]\n\t"
        "mov %[tmp3], %[tmp4]\n\t"
        : [tmp4] "=r" (t4)
        : [tmp3] "r" (t3), [call] "r" (get_value(v5))
        : "cc"
    );
    
    /* Final chain with all previous results */
    __asm__ __volatile__ (
        "lea (%[t1], %[t2], 2), %%eax\n\t"
        "add %[t3], %%eax\n\t"
        "sub %[t4], %%eax\n\t"
        "mov %%eax, %[t5]\n\t"
        : [t5] "=r" (t5)
        : [t1] "r" (t1), [t2] "r" (t2), [t3] "r" (t3), [t4] "r" (t4)
        : "eax", "cc"
    );
    
    return t5;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 3;
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to trigger various reload patterns */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    checksum += test_chain_of_reloads();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return deterministic result for verification */
    return checksum % 256;
}

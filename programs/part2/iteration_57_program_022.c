/* reload_stress_test.c
 * Designed to trigger GCC's reload logic, specifically push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int* global_ptr = &global_int;

/* Function to force evaluation into register */
int get_value(int x) {
    return x * 2 + 1;
}

double compute_double(int a, int b) {
    return (double)(a + b) / 2.0;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60, g = 70, h = 80;
    int out1, out2, out3, out4, out5, out6, out7, out8;
    
    /* Complex inline asm with many operands */
    __asm__ __volatile__ (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        "sub %[in3], %[out3]\n\t"
        "imul %[in4], %[out4]\n\t"
        "and %[in5], %[out5]\n\t"
        "or %[in6], %[out6]\n\t"
        "xor %[in7], %[out7]\n\t"
        "lea (%[in8],%[reg0],2), %[out8]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5), [out6] "=r" (out6),
          [out7] "=r" (out7), [out8] "=r" (out8)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c), [in4] "r" (d),
          [in5] "r" (e), [in6] "r" (f), [in7] "r" (g), [in8] "r" (h),
          [reg0] "r" (r0)
        : "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4 + out5 + out6 + out7 + out8;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    
    /* Function calls as operands - forces evaluation before asm */
    __asm__ __volatile__ (
        "mov %%eax, %%ebx\n\t"
        "add %%ecx, %%ebx\n\t"
        "mov %%ebx, %0\n\t"
        "movsd %1, %%xmm0\n\t"
        "cvttsd2si %%xmm0, %2"
        : "=r" (result1), "=r" (result3)
        : "r" (get_value(global_int)), 
          "r" (compute_double(get_value(10), get_value(20))),
          "m" (global_double)
        : "eax", "ebx", "ecx", "xmm0", "memory"
    );
    
    /* Another asm with pointer arithmetic */
    int index = global_int % 256;
    __asm__ __volatile__ (
        "mov (%[ptr],%[idx],1), %%al\n\t"
        "movsbl %%al, %[out]"
        : [out] "=r" (result2)
        : [ptr] "r" (global_array), [idx] "r" (index)
        : "al", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 1000000L, l2 = 2000000L;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "movsbl %%al, %[outc]\n\t"
        "mov %[s1], %%ax\n\t"
        "imul %[s2], %%ax\n\t"
        "movswl %%ax, %[outs]\n\t"
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "mov %%eax, %[outi]\n\t"
        "fld %[d1]\n\t"
        "fadd %[d2]\n\t"
        "fstp %[outd]"
        : [outc] "=r" (out_char), [outs] "=r" (out_short),
          [outi] "=r" (out_int), [outd] "=m" (out_double)
        : [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [d1] "m" (d1), [d2] "m" (d2)
        : "al", "ax", "eax", "st", "st(1)", "memory"
    );
    
    /* Cast double to int via asm - forces mode change */
    int double_as_int;
    __asm__ __volatile__ (
        "fld %[dbl]\n\t"
        "fistpl %[out]"
        : [out] "=m" (double_as_int)
        : [dbl] "m" (d1)
        : "st", "memory"
    );
    
    return out_char + out_short + out_int + (int)out_double + double_as_int;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2, out3;
    
    /* Using specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "test %[in3], %[in3]\n\t"
        "setg %%al\n\t"
        "movzx %%al, %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "a" (a), [in2] "r" (b), [in3] "r" (c)
        : "cc", "memory"
    );
    
    /* Memory operand with complex addressing */
    int array[100];
    for (int i = 0; i < 100; i++) array[i] = i;
    
    __asm__ __volatile__ (
        "mov (%[base],%[index],4), %[out]"
        : [out] "=r" (out3)
        : [base] "r" (array), [index] "r" (get_value(10) % 100)
        : "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int x = 1, y = 2, z = 3;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Chain of volatile asm blocks */
    __asm__ __volatile__ (
        "mov %[x], %[t1]\n\t"
        "add $1, %[t1]"
        : [t1] "=r" (tmp1)
        : [x] "r" (x)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[y], %[t2]\n\t"
        "add %[t1], %[t2]"
        : [t2] "=r" (tmp2)
        : [y] "r" (y), [t1] "r" (tmp1)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[z], %[t3]\n\t"
        "imul %[t2], %[t3]"
        : [t3] "=r" (tmp3)
        : [z] "r" (z), [t2] "r" (tmp2)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[t3], %[t4]\n\t"
        "shr $2, %[t4]"
        : [t4] "=r" (tmp4)
        : [t3] "r" (tmp3)
        : "cc", "memory"
    );
    
    return tmp1 + tmp2 + tmp3 + tmp4;
}

/* Test 6: Explicit register variables with constraints */
int test_explicit_registers(void) {
    register int var1 asm ("r10") = 100;
    register int var2 asm ("r11") = 200;
    int result;
    
    /* Reference explicit register variables in asm */
    __asm__ __volatile__ (
        "add %%r10d, %%r11d\n\t"
        "mov %%r11d, %0"
        : "=r" (result)
        : 
        : "r10", "r11", "memory"
    );
    
    /* Force move from explicit register to constrained register */
    int out;
    __asm__ __volatile__ (
        "mov %%eax, %0"
        : "=a" (out)
        : "r" (var1)
        : "memory"
    );
    
    return result + out;
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
    checksum += test_volatile_chains();
    checksum += test_explicit_registers();
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

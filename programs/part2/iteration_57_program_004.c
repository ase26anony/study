/* reload_stress_test.c
 * Designed to trigger push_reload initialization (lines 1381-1399 in reload.cc)
 * through complex inline assembly patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create register pressure */
int global_int1 = 12345;
int global_int2 = 67890;
int global_int3 = 54321;
double global_double1 = 3.14159;
double global_double2 = 2.71828;
char global_char_array[256];
int global_int_array[100];

/* Function declarations for nested calls */
int compute_value(int a, int b);
double compute_double(int x);
int* get_pointer(int index);
int complex_address(int base, int offset);

/* Test function 1: Many operands with mixed types */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Register variables to force specific register allocation */
    register int reg_var1 asm ("r12") = 42;
    register int reg_var2 asm ("r13") = 84;
    register double reg_double asm ("xmm0") = 3.14;
    
    /* Complex inline assembly with many operands */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out2]\n\t"
        "mov %[reg_var1], %[out3]\n\t"
        "cvtsi2sd %[in1], %[reg_double]\n\t"
        "movsd %[reg_double], %[d1]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=m" (out4),
          [d1] "=m" (d1)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [reg_var1] "r" (reg_var1),
          [reg_var2] "r" (reg_var2),
          [reg_double] "x" (reg_double),
          [d2] "m" (d2),
          [c1] "r" (c1),
          [s1] "r" (s1)
        : "memory", "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r14", "r15",
          "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test function 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int temp1, temp2, temp3;
    
    /* Inline assembly with function calls as operands */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "imul %[call3], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "lea (%[ptr], %[idx], 4), %%rbx\n\t"
        "mov (%%rbx), %[res2]\n\t"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=m" (result3)
        : [call1] "r" (compute_value(global_int1, global_int2)),
          [call2] "r" (compute_value(global_int2, global_int3)),
          [call3] "r" (complex_address(1000, 200)),
          [ptr] "r" (global_int_array),
          [idx] "r" (compute_value(10, 20))
        : "memory", "cc", "rax", "rbx", "rcx", "rdx"
    );
    
    /* Another volatile block with memory clobber */
    __asm__ __volatile__ (
        ""
        : "=r" (temp1), "=r" (temp2), "=r" (temp3)
        : "0" (result1), "1" (result2), "2" (global_int1),
          "m" (global_int_array[0]), "m" (global_int_array[1])
        : "memory"
    );
    
    return result1 + result2 + result3 + temp1 + temp2 + temp3;
}

/* Test function 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c_out;
    short s_out;
    int i_out;
    long l_out;
    float f_out;
    double d_out;
    
    char c_in = 'X';
    short s_in = 1234;
    int i_in = 123456;
    long l_in = 1234567890L;
    float f_in = 2.5f;
    double d_in = 3.14159;
    
    /* Force mode changes through casts in operands */
    __asm__ __volatile__ (
        "mov %[c_in], %b[c_out]\n\t"
        "mov %[s_in], %w[s_out]\n\t"
        "mov %[i_in], %k[i_out]\n\t"
        "mov %[l_in], %[l_out]\n\t"
        "movd %[f_in], %[f_out]\n\t"
        "movq %[d_in], %[d_out]\n\t"
        : [c_out] "=r" (c_out),
          [s_out] "=r" (s_out),
          [i_out] "=r" (i_out),
          [l_out] "=r" (l_out),
          [f_out] "=x" (f_out),
          [d_out] "=x" (d_out)
        : [c_in] "r" ((int)c_in),      /* Cast forces mode change */
          [s_in] "r" ((int)s_in),      /* short to int */
          [i_in] "r" (i_in),
          [l_in] "r" (l_in),
          [f_in] "x" (f_in),
          [d_in] "x" (d_in)
        : "memory"
    );
    
    /* Use different register classes for same value */
    register int acc_var asm ("eax") = i_out;
    int mem_var;
    
    __asm__ __volatile__ (
        "test %[acc], %[acc]\n\t"
        "setnz %b[mem]\n\t"
        : [mem] "=m" (mem_var)
        : [acc] "r" (acc_var)
        : "cc", "memory"
    );
    
    return c_out + s_out + i_out + (l_out & 0xFFFF) + (int)f_out + (int)d_out + mem_var;
}

/* Test function 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result;
    double dbl_result;
    
    /* Try to force moves between register classes */
    register int fixed_reg asm ("ebx") = 0x12345678;
    
    /* Assembly that might require secondary reloads */
    __asm__ __volatile__ (
        /* Move to accumulator for specific operation */
        "mov %[fixed], %%eax\n\t"
        "cpuid\n\t"  /* Requires specific registers */
        "mov %%eax, %[res]\n\t"
        : [res] "=r" (result)
        : [fixed] "r" (fixed_reg),
          "a" (0),   /* EAX must be 0 */
          "c" (0)    /* ECX must be 0 */
        : "memory", "cc", "rdx", "rbx"
    );
    
    /* Floating point with integer constraints */
    __asm__ __volatile__ (
        "cvtsi2sd %[int_val], %[dbl_res]\n\t"
        "movsd %[dbl_res], %[mem]\n\t"
        : [dbl_res] "=x" (dbl_result),
          [mem] "=m" (global_double1)
        : [int_val] "r" (result)
        : "memory"
    );
    
    return result + (int)dbl_result;
}

/* Test function 5: Complex addressing modes */
int test_complex_addressing(void) {
    int results[4];
    int* ptr1 = global_int_array;
    int* ptr2 = &global_int_array[50];
    
    /* Complex addressing in operands */
    for (int i = 0; i < 4; i++) {
        __asm__ __volatile__ (
            "mov (%[base], %[idx], 4), %%eax\n\t"
            "add (%[base2], %[idx], 4), %%eax\n\t"
            "mov %%eax, %[out]\n\t"
            : [out] "=r" (results[i])
            : [base] "r" (ptr1),
              [base2] "r" (ptr2),
              [idx] "r" (i * 5 + compute_value(i, 2))
            : "memory", "cc", "rax"
        );
    }
    
    /* Chain of dependent operations */
    int chain_result;
    __asm__ __volatile__ (
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "imul %[c], %%eax\n\t"
        "sub %[d], %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (chain_result)
        : [a] "r" (results[0]),
          [b] "r" (results[1]),
          [c] "r" (results[2]),
          [d] "r" (results[3])
        : "memory", "cc", "rax"
    );
    
    return chain_result;
}

/* Helper functions */
int compute_value(int a, int b) {
    return a * 3 + b * 2;
}

double compute_double(int x) {
    return x * 0.5;
}

int* get_pointer(int index) {
    return &global_int_array[index % 100];
}

int complex_address(int base, int offset) {
    return base + offset * 2;
}

/* Main function */
int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 3;
    }
    
    /* Run all test functions */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_complex_addressing();
    
    /* Final assembly barrier */
    __asm__ __volatile__ (
        ""
        :
        :
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte */
}

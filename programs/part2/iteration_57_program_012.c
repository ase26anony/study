/* reload_stress.c - Stress GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create register pressure */
int global_int1 = 123, global_int2 = 456, global_int3 = 789;
double global_double1 = 3.14159, global_double2 = 2.71828;
char global_char_array[256];
int *global_ptr = &global_int1;

/* Function that returns a value - used in nested calls */
int get_value(int x) { return x * 2 + 1; }
double get_double(int x) { return (double)x / 3.0; }
int* get_pointer(void) { return &global_int2; }

/* Complex addressing helper */
int array_index(int idx) { return idx % 128; }

/* Test 1: Many operands with mixed types and constraints */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = 111;
    register int reg_var2 asm ("r13") = 222;
    register double reg_double asm ("xmm0") = 3.14;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        /* Use register variables */
        "add %%r12, %[out1]\n\t"
        "add %%r13, %[out2]\n\t"
        /* Mixed size operations */
        "movsx %[c1], %[out3]\n\t"
        "movsx %[s1], %[out4]\n\t"
        /* Memory operand */
        "add (%[gptr]), %[out3]\n\t"
        /* Clobber many registers */
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (in1), [in2] "r" (in2), 
          [in3] "r" (in3), [in4] "r" (in4),
          [c1] "r" ((int)c1), [s1] "r" ((int)s1),
          [gptr] "r" (global_ptr),
          "r" (reg_var1), "r" (reg_var2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result1, result2;
    int temp1, temp2;
    
    /* Multiple volatile asm blocks with interdependent operands */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        : [res1] "=r" (result1)
        : [call1] "r" (get_value(global_int1)),
          [call2] "r" (get_value(global_int2))
        : "rax", "rcx", "rdx", "memory"
    );
    
    /* Second asm using result from first */
    __asm__ __volatile__ (
        "lea (%[prev], %[idx], 4), %[res2]\n\t"
        : [res2] "=r" (result2)
        : [prev] "r" (result1),
          [idx] "r" (array_index(global_int3))
        : "cc"
    );
    
    /* Third asm with complex addressing */
    __asm__ __volatile__ (
        "mov (%[base], %[offset], 1), %[tmp]\n\t"
        : [tmp] "=r" (temp1)
        : [base] "r" (global_char_array),
          [offset] "r" (get_value(10))
        : "memory"
    );
    
    return result1 + result2 + temp1;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int int_result;
    double double_result;
    float float_val = 7.5f;
    long long big_val = 0x123456789ABCDEF0LL;
    char small_val = 127;
    
    /* Force mode changes between int and float */
    __asm__ __volatile__ (
        /* Convert float to int */
        "cvttss2si %[float], %%eax\n\t"
        /* Mix with char (sign extension) */
        "movsx %[char], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        /* Use part of long long */
        "mov %[ll], %%rcx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (int_result)
        : [float] "x" (float_val),
          [char] "r" ((int)small_val),
          [ll] "r" (big_val)
        : "rax", "rbx", "rcx", "xmm0", "xmm1", "cc"
    );
    
    /* Double with integer operations */
    __asm__ __volatile__ (
        "pxor %%xmm0, %%xmm0\n\t"
        "cvtsi2sd %[val], %%xmm0\n\t"
        "addsd %[dbl], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]\n\t"
        : [dout] "=x" (double_result)
        : [val] "r" (global_int1),
          [dbl] "x" (global_double1)
        : "xmm0", "xmm1", "cc"
    );
    
    return int_result + (int)double_result;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int out1, out2;
    
    /* Try to force specific register constraints */
    __asm__ __volatile__ (
        /* 'a' constraint for accumulator */
        "mov %[in1], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* 'b' constraint for base register */
        "mov %[in2], %%ebx\n\t"
        "shl $2, %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        : [out1] "=a" (out1), [out2] "=b" (out2)
        : [in1] "rm" (get_value(50)),  /* Allow memory or register */
          [in2] "rm" (get_value(100))
        : "cc"
    );
    
    /* Memory constraint with complex address */
    int array[100];
    int idx = global_int1;
    
    __asm__ __volatile__ (
        "movl $99, %[mem]\n\t"
        : [mem] "=m" (array[array_index(idx)])
        : 
        : "memory"
    );
    
    return out1 + out2 + array[0];
}

/* Test 5: Extreme register pressure */
int test_extreme_pressure(void) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4;
    int results[8];
    
    /* Chain of asm statements with many live values */
    __asm__ __volatile__ (
        "mov %[a1], %[r1]\n\t"
        "add %[a2], %[r1]\n\t"
        "add %[a3], %[r1]\n\t"
        "add %[a4], %[r1]\n\t"
        : [r1] "=r" (results[0])
        : [a1] "r" (v1), [a2] "r" (v2), 
          [a3] "r" (v3), [a4] "r" (v4)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "imul %[b1], %[r2]\n\t"
        "imul %[b2], %[r2]\n\t"
        : [r2] "=r" (results[1])
        : [b1] "r" (v5), [b2] "r" (v6)
        : "cc"
    );
    
    /* Use all variables in one big asm */
    __asm__ __volatile__ (
        "mov %[x1], %%eax\n\t"
        "add %[x2], %%eax\n\t"
        "add %[x3], %%eax\n\t"
        "add %[x4], %%eax\n\t"
        "add %[x5], %%eax\n\t"
        "add %[x6], %%eax\n\t"
        "add %[x7], %%eax\n\t"
        "add %[x8], %%eax\n\t"
        "mov %%eax, %[r3]\n\t"
        : [r3] "=r" (results[2])
        : [x1] "r" (v7), [x2] "r" (v8), [x3] "r" (v9),
          [x4] "r" (v10), [x5] "r" (v11), [x6] "r" (v12),
          [x7] "r" (v13), [x8] "r" (v14)
        : "rax", "cc"
    );
    
    return results[0] + results[1] + results[2];
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_extreme_pressure();
    
    /* Final asm to use the checksum */
    __asm__ __volatile__ (
        "add $1, %0\n\t"
        : "+r" (checksum)
        : 
        : "cc"
    );
    
    return checksum;
}

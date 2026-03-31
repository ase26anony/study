/* reload_test.c - Program to stress GCC's reload mechanism */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function that returns values requiring computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(int x) {
    return (double)x / 3.0;
}

int* get_pointer(int offset) {
    return global_array + offset;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int out1, out2, out3, out4;
    
    /* Force register pressure with many input/output operands */
    __asm__ __volatile__ (
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "mov %[c], %[out2]\n\t"
        "imul %[d], %[out2]\n\t"
        "mov %[e], %[out3]\n\t"
        "sub %[f], %[out3]\n\t"
        "mov %[g], %[out4]\n\t"
        "xor %[h], %[out4]"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k), [l] "r" (l)
        : "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4 + i + j + k + l + m + n + o + p;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2;
    double dresult;
    
    /* Function calls as operands force evaluation before assembly */
    __asm__ __volatile__ (
        "mov %%eax, %[res1]\n\t"
        "mov %%ebx, %[res2]"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : "a" (compute_value(global_int)), 
          "b" (compute_value(global_int * 2)),
          "c" (compute_value(compute_value(10))),  /* Nested call */
          "d" ((int)compute_double(20))            /* Type conversion */
        : "memory"
    );
    
    /* Mixed types with memory constraint */
    __asm__ __volatile__ (
        "fldl %[in]\n\t"
        "fstpl %[out]"
        : [out] "=m" (dresult)
        : [in] "m" (compute_double(30)),
          "r" (get_pointer(100)),      /* Pointer computation */
          "r" (global_array[compute_value(5)])  /* Array with computed index */
        : "st", "st(1)", "memory"
    );
    
    return result1 + result2 + (int)dresult;
}

/* Test 3: Mixed data types and explicit register variables */
int test_mixed_types(void) {
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 50000, i2 = 60000;
    long long ll1 = 1234567890LL, ll2 = 9876543210LL;
    float f1 = 1.234f, f2 = 5.678f;
    double d1 = 9.876, d2 = 5.432;
    int out_int;
    double out_double;
    
    /* Mixing different sized operands */
    __asm__ __volatile__ (
        "mov %[c1], %%al\n\t"
        "mov %[s1], %%bx\n\t"
        "add %[i1], %%ecx\n\t"
        "mov %[ll1], %%rdx\n\t"
        "movss %[f1], %%xmm0\n\t"
        "movsd %[d1], %%xmm1"
        : 
        : [c1] "r" ((int)c1), [s1] "r" ((int)s1), 
          [i1] "r" (i1), [ll1] "r" (ll1),
          [f1] "x" (f1), [d1] "x" (d1),
          "r" (r12_var), "r" (r13_var)  /* Explicit register vars */
        : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory"
    );
    
    /* Type conversions requiring mode changes */
    __asm__ __volatile__ (
        "cvtsi2sd %[int_in], %%xmm0\n\t"
        "movsd %%xmm0, %[dbl_out]"
        : [dbl_out] "=m" (out_double)
        : [int_in] "r" (i2),
          [ll_in] "r" (ll2),      /* Different mode input */
          [flt_in] "x" (f2)       /* Different type in different reg class */
        : "xmm0", "memory"
    );
    
    /* Memory operand with complex addressing */
    __asm__ __volatile__ (
        "mov (%[base], %[index], 4), %[out]"
        : [out] "=r" (out_int)
        : [base] "r" (global_array),
          [index] "r" (compute_value(20) % 64)  /* Computed index */
        : "memory"
    );
    
    return out_int + (int)out_double + r12_var + r13_var;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2;
    
    /* Force specific register constraints */
    __asm__ __volatile__ (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0"
        : "=r" (out1)
        : "a" (a),   /* Must be in eax */
          "r" (b),   /* General register - may need secondary reload to get to eax */
          "c" (c)    /* Must be in ecx */
        : "eax", "ecx", "memory"
    );
    
    /* Memory output with register input */
    __asm__ __volatile__ (
        "mov %1, %0"
        : "=m" (global_array[10])  /* Memory output constraint */
        : "r" (compute_value(50))  /* Register input from function call */
        : "memory"
    );
    
    /* Immediate constraints mixed with registers */
    __asm__ __volatile__ (
        "lea (%1, %2, 2), %0"
        : "=r" (out2)
        : "r" (a),
          "i" (8),      /* Immediate - may need secondary handling */
          "m" (global_int)  /* Memory operand */
        : "memory"
    );
    
    return out1 + out2 + global_array[10];
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int x = 1, y = 2, z = 3;
    int tmp1, tmp2, tmp3;
    
    /* Chain of volatile asm blocks */
    __asm__ __volatile__ (
        "mov %1, %0\n\t"
        "add $10, %0"
        : "=r" (tmp1)
        : "r" (x)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "imul %1, %0"
        : "=r" (tmp2)
        : "r" (tmp1),  /* Depends on previous output */
          "r" (y)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "sub %2, %0\n\t"
        "add %1, %0"
        : "=r" (tmp3)
        : "r" (tmp2),  /* Another dependency */
          "r" (z),
          "m" (global_double)  /* Memory clobbered value */
        : "memory"
    );
    
    /* Complex expression with addressing */
    __asm__ __volatile__ (
        "mov (%[ptr], %[idx], 1), %[out]"
        : [out] "=r" (x)
        : [ptr] "r" (global_array),
          [idx] "r" (tmp3 % 256)  /* Computed from previous result */
        : "memory"
    );
    
    return x + tmp1 + tmp2 + tmp3;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to trigger various reload scenarios */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    
    /* Final complex asm to ensure all paths are exercised */
    {
        int final_result;
        register int r10_var asm ("r10") = 999;
        register int r11_var asm ("r11") = 888;
        
        __asm__ __volatile__ (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "add %3, %%eax\n\t"
            "add %4, %%eax\n\t"
            "mov %%eax, %0"
            : "=r" (final_result)
            : "r" (checksum),
              "r" (r10_var),
              "r" (r11_var),
              "i" (777),      /* Immediate */
              "m" (global_int) /* Memory */
            : "rax", "memory", "cc"
        );
        
        checksum = final_result;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return deterministic value for verification */
    return checksum % 256;
}

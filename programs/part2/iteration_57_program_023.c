/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function to force evaluation into register */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5;
}

char* get_string_offset(int idx) {
    return &global_array[idx];
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int out1, out2, out3, out4;
    
    /* Complex inline asm with many input/output operands */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "subl %[in4], %[out2]\n\t"
        "movl %[in5], %[out3]\n\t"
        "imull %[in6], %[out3]\n\t"
        "movl %[in7], %[out4]\n\t"
        "xorl %[in8], %[out4]"
        : [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h),
          "r" (i), "r" (j), "r" (k), "r" (l)  /* Extra inputs without explicit binding */
        : "memory", "cc"
    );
    
    /* Second asm block using previous outputs */
    int final;
    __asm__ __volatile__ (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0"
        : "=r" (final)
        : "r" (out1), "r" (out2), "r" (out3), "0" (out4)
        : "cc"
    );
    
    return final;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A';
    short s1 = 1000;
    int i1 = 50000;
    long l1 = 1000000L;
    float f1 = 2.5f;
    double d1 = 3.14159;
    
    int result_int;
    double result_double;
    
    /* Mixed type constraints forcing mode changes */
    __asm__ __volatile__ (
        /* Convert and combine different types */
        "movsbl %[char_in], %%eax\n\t"
        "movswl %[short_in], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[int_in], %%eax\n\t"
        "movl %%eax, %[int_out]\n\t"
        /* Floating point through integer registers (no SSE) */
        "fldl %[double_in]\n\t"
        "fstpl %[double_out]"
        : [int_out] "=r" (result_int),
          [double_out] "=m" (result_double)
        : [char_in] "r" (c1),
          [short_in] "r" (s1),
          [int_in] "r" (i1),
          [double_in] "m" (d1)
        : "eax", "ebx", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
    );
    
    /* Use explicit register variable with constraint mismatch */
    register long reg_var asm ("r12") = l1;
    int from_reg;
    __asm__ __volatile__ (
        "movq %1, %%rax\n\t"
        "movl %%eax, %0"
        : "=r" (from_reg)
        : "r" (reg_var)
        : "rax"
    );
    
    return result_int + (int)result_double + from_reg;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int x = 10;
    double y = 20.5;
    int result1, result2;
    
    /* Function calls as operands - must be evaluated before asm */
    __asm__ __volatile__ (
        "movl %[call1], %%eax\n\t"
        "addl %[call2], %%eax\n\t"
        "movl %%eax, %[out1]"
        : [out1] "=r" (result1)
        : [call1] "r" (compute_value(x)),
          [call2] "r" (compute_value(x + 5))
        : "eax", "memory"
    );
    
    /* Complex addressing with array indexing */
    char *ptr;
    int offset_result;
    __asm__ __volatile__ (
        "movq %[addr], %%rax\n\t"
        "movb (%%rax), %%al\n\t"
        "movsbl %%al, %[out]"
        : [out] "=r" (offset_result)
        : [addr] "r" (get_string_offset(compute_value(3)))
        : "rax", "memory"
    );
    
    return result1 + offset_result;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2;
    
    /* Try to force specific register constraints */
    __asm__ __volatile__ (
        /* Constraint 'a' for accumulator */
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (out1)
        : "a" (a), "r" (b)
        : "cc"
    );
    
    /* Memory constraint with complex address */
    __asm__ __volatile__ (
        "movl (%[ptr]), %[out]"
        : [out] "=r" (out2)
        : [ptr] "r" (&global_int + compute_value(1) / 10)
        : "memory"
    );
    
    /* Immediate constraint mixed with register */
    __asm__ __volatile__ (
        "addl %1, %0"
        : "+r" (out1)
        : "i" (15)
        : "cc"
    );
    
    return out1 + out2;
}

/* Test 5: Volatile chains with memory clobbers */
int test_volatile_chains(void) {
    int vals[10];
    for (int i = 0; i < 10; i++) vals[i] = i * 2;
    
    int sum = 0;
    int temp;
    
    /* Chain of volatile asm blocks */
    for (int i = 0; i < 10; i++) {
        __asm__ __volatile__ (
            "movl %[in], %%eax\n\t"
            "addl $1, %%eax"
            : "=a" (temp)
            : [in] "r" (vals[i])
            : "cc"
        );
        
        __asm__ __volatile__ (
            "addl %[in], %[out]"
            : [out] "+r" (sum)
            : [in] "r" (temp)
            : "cc"
        );
        
        /* Memory clobber between operations */
        __asm__ __volatile__ ("" : : : "memory");
    }
    
    /* Force spill with many live values */
    int v1 = sum, v2 = sum/2, v3 = sum/3, v4 = sum/4;
    int v5 = sum/5, v6 = sum/6, v7 = sum/7, v8 = sum/8;
    
    __asm__ __volatile__ (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0"
        : "+r" (sum)
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4),
          "r" (v5), "r" (v6), "r" (v7)
        : "cc"
    );
    
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    int checksum = 0;
    
    /* Run all tests, accumulating results */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_nested_calls();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    
    /* Final assembly to ensure all values are used */
    __asm__ __volatile__ (
        "addl $1, %0"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte */
}

/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_char = 'A';
short global_short = 32767;
long global_long = 1234567890L;
float global_float = 2.71828f;

/* Function to force evaluation into register */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5;
}

float compute_float(float x) {
    return x / 2.0f;
}

/* Complex addressing function */
int* get_pointer(int offset) {
    static int array[100];
    return &array[offset];
}

/* Test 1: Many operands with mixed types to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4, out5;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    double d1 = 1.1, d2 = 2.2;
    float f1 = 3.3f, f2 = 4.4f;
    char c1 = 'X', c2 = 'Y';
    short s1 = 100, s2 = 200;
    long l1 = 1000L, l2 = 2000L;
    
    /* Register variables with explicit registers */
    register int reg1 asm ("r12") = 111;
    register int reg2 asm ("r13") = 222;
    register int reg3 asm ("r14") = 333;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "subl %[in4], %[out2]\n\t"
        /* Force register moves */
        "movl %[reg1], %%eax\n\t"
        "addl %[reg2], %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        /* Mixed size operations */
        "movsbl %[c1], %%ebx\n\t"
        "movswl %[s1], %%ecx\n\t"
        "addl %%ebx, %%ecx\n\t"
        "movl %%ecx, %[out4]\n\t"
        /* Memory operand */
        "movl (%[ptr]), %%edx\n\t"
        "addl %[global], %%edx\n\t"
        "movl %%edx, %[out5]"
        
        : [out1] "=r" (out1), 
          [out2] "=r" (out2), 
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5)
        
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [in4] "r" (in4),
          [reg1] "r" (reg1),
          [reg2] "r" (reg2),
          [c1] "r" (c1),
          [s1] "r" (s1),
          [ptr] "r" (get_pointer(10)),
          [global] "m" (global_int)
        
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    
    /* Function calls as operands - forces evaluation before assembly */
    __asm__ __volatile__ (
        "movl %%eax, %[res1]\n\t"
        "addl %%ebx, %[res2]\n\t"
        "subl %%ecx, %[res3]"
        
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=r" (result3)
        
        : "a" (compute_value(global_int)),
          "b" (compute_value(global_short)),
          "c" (compute_value((int)global_char))
        
        : "memory"
    );
    
    /* Chain of volatile assembly with interdependent operands */
    int temp;
    __asm__ __volatile__ (
        "movl %[r1], %[t]\n\t"
        "addl $1, %[t]"
        : [t] "=r" (temp)
        : [r1] "r" (result1)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "addl %[t], %[r2]"
        : [r2] "+r" (result2)
        : [t] "r" (temp)
        : "cc"
    );
    
    return result1 + result2 + result3 + temp;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int int_result;
    double double_temp;
    float float_temp;
    char char_temp;
    
    /* Using double in integer assembly - forces mode change */
    __asm__ __volatile__ (
        "fldl %[dbl]\n\t"
        "fistpl %[intr]\n\t"
        "movl %[intr], %[out]"
        : [out] "=r" (int_result)
        : [dbl] "m" (compute_double(global_double)),
          [intr] "m" (int_result)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
    );
    
    /* Float to int with explicit cast in operand */
    __asm__ __volatile__ (
        "movl %[flt], %%eax\n\t"
        "movl %%eax, %[chr]"
        : [chr] "=r" (char_temp)
        : [flt] "r" ((int)compute_float(global_float))
        : "eax"
    );
    
    /* Mixed size operations in single asm */
    short s1 = 100, s2 = 200;
    int i1 = 1000;
    __asm__ __volatile__ (
        "movswl %[s1], %%eax\n\t"
        "movswl %[s2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[i1], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (int_result)
        : [s1] "r" (s1),
          [s2] "r" (s2),
          [i1] "r" (i1)
        : "eax", "ebx", "cc"
    );
    
    return int_result + char_temp;
}

/* Test 4: Complex addressing modes and pointer arithmetic */
int test_complex_addressing(void) {
    int array[50];
    int *ptr1, *ptr2;
    int result1, result2, result3;
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        array[i] = i * 2;
    }
    
    ptr1 = &array[10];
    ptr2 = &array[20];
    
    /* Complex addressing with non-constant offset */
    __asm__ __volatile__ (
        "movl (%[base], %[idx], 4), %%eax\n\t"
        "addl (%[ptr2]), %%eax\n\t"
        "movl %%eax, %[res1]"
        : [res1] "=r" (result1)
        : [base] "r" (array),
          [idx] "r" (global_int % 25),  /* Non-constant index */
          [ptr2] "r" (ptr2)
        : "eax", "memory"
    );
    
    /* Pointer arithmetic in operand */
    int offset = compute_value(5);
    __asm__ __volatile__ (
        "movl (%[ptr], %[off], 4), %%eax\n\t"
        "imull $3, %%eax\n\t"
        "movl %%eax, %[res2]"
        : [res2] "=r" (result2)
        : [ptr] "r" (array),
          [off] "r" (offset)
        : "eax", "cc", "memory"
    );
    
    /* Multiple memory operands */
    __asm__ __volatile__ (
        "movl %[val1], %%eax\n\t"
        "addl %[val2], %%eax\n\t"
        "addl %[val3], %%eax\n\t"
        "movl %%eax, %[res3]"
        : [res3] "=r" (result3)
        : [val1] "m" (array[1]),
          [val2] "m" (array[2]),
          [val3] "m" (array[3])
        : "eax", "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 5: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result;
    double dbl_val = 123.456;
    float flt_val = 78.9f;
    
    /* Attempt to force accumulator-specific constraint with incompatible value */
    register int acc_val asm ("eax") = 999;
    
    /* Using specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        /* 'a' constraint for accumulator */
        "addl $100, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)
        : "a" (acc_val)
        : "cc"
    );
    
    /* Mixing register classes */
    int x = 100, y = 200;
    __asm__ __volatile__ (
        "movl %[x], %%ecx\n\t"
        "addl %[y], %%ecx\n\t"
        /* Try to move to accumulator for specific operation */
        "movl %%ecx, %%eax\n\t"
        "imull $2, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)
        : [x] "r" (x),
          [y] "r" (y)
        : "eax", "ecx", "cc"
    );
    
    /* Force spill/reload with many clobbered registers */
    __asm__ __volatile__ (
        "movl $1, %%eax\n\t"
        "movl $2, %%ebx\n\t"
        "movl $3, %%ecx\n\t"
        "movl $4, %%edx\n\t"
        "movl $5, %%esi\n\t"
        "movl $6, %%edi\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "addl %%esi, %%eax\n\t"
        "addl %%edi, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc"
    );
    
    return result;
}

/* Test 6: Extreme register pressure */
int test_extreme_pressure(void) {
    /* Many local variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4;
    float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f;
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    
    int result = 0;
    
    /* Series of assembly blocks that use all variables */
    __asm__ __volatile__ (
        "addl %[a], %[b]\n\t"
        "addl %[c], %[b]"
        : [b] "+r" (v2)
        : [a] "r" (v1),
          [c] "r" (v3)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "imull %[a], %[b]"
        : [b] "+r" (v4)
        : [a] "r" (v5)
        : "cc"
    );
    
    /* Use function results */
    __asm__ __volatile__ (
        "addl %[a], %[out]"
        : [out] "=r" (result)
        : [a] "r" (compute_value(v6))
        : "cc"
    );
    
    /* Chain computations */
    for (int i = 0; i < 5; i++) {
        __asm__ __volatile__ (
            "addl $1, %[val]"
            : [val] "+r" (v7)
            :
            : "cc"
        );
    }
    
    /* Final combination */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    result += v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += c1 + c2 + c3 + c4;
    result += s1 + s2 + s3 + s4;
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    checksum += test_extreme_pressure();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum in a way that prevents dead code elimination */
    if (checksum > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen with our test values */
    }
}

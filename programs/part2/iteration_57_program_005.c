/* reload_stress_test.c - Stress test for GCC reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function prototypes for nested calls */
int compute_value(int x);
double compute_double(double x);
void *get_pointer(void);
int complex_index(int *arr, int idx);

/* Complex function that returns a value requiring computation */
int compute_value(int x) {
    return (x * 3 + 7) % 13;
}

double compute_double(double x) {
    return sin(x) * cos(x);
}

void *get_pointer(void) {
    return &global_array[128];
}

int complex_index(int *arr, int idx) {
    return arr[idx * 2 + 1];
}

/* Test 1: Many operands exhausting registers */
void test_many_operands(void) {
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int out1, out2, out3, out4, out5, out6, out7, out8;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.1, d2 = 2.2;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Complex inline asm with many operands of different types */
    __asm__ __volatile__ (
        "/* Many operand test */\n\t"
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        "sub %[in3], %[out3]\n\t"
        "imul %[in4], %[out4]\n\t"
        "mov %[d1_int], %[out5]\n\t"
        "add %[d2_int], %[out6]\n\t"
        "movzx %[c1], %[out7]\n\t"
        "movsx %[s1], %[out8]"
        : [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4),
          [out5] "=r" (out5), [out6] "=r" (out6),
          [out7] "=r" (out7), [out8] "=r" (out8)
        : [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4),
          [d1_int] "r" ((int)d1), [d2_int] "r" ((int)d2),
          [c1] "r" ((int)c1), [s1] "r" ((int)s1),
          "r" (r1), "r" (r2), "r" (r3), "r" (r4)
        : "memory", "cc"
    );
    
    /* Use results to prevent optimization */
    global_int += out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
void test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    void *ptr_result;
    
    /* Function calls as input operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "/* Nested calls test */\n\t"
        "mov %[call1], %[res1]\n\t"
        "add %[call2], %[res2]\n\t"
        "mov %[call3], %[res3]\n\t"
        "mov %[ptr], %[dres]"
        : [res1] "=r" (result1), [res2] "=r" (result2),
          [res3] "=r" (result3), [dres] "=r" (dresult)
        : [call1] "r" (compute_value(global_int)),
          [call2] "r" (compute_value(global_int * 2)),
          [call3] "r" ((int)compute_double(global_double)),
          [ptr] "r" (get_pointer())
        : "memory"
    );
    
    /* Complex addressing mode with function call */
    int array[100];
    int idx_result;
    
    __asm__ __volatile__ (
        "mov (%[arr], %[idx], 4), %[out]"
        : [out] "=r" (idx_result)
        : [arr] "r" (array),
          [idx] "r" (complex_index(array, compute_value(10)))
        : "memory"
    );
    
    global_int += result1 + result2 + result3 + idx_result;
}

/* Test 3: Mixed data types and mode changes */
void test_mixed_types(void) {
    char c = 'X';
    short s = 12345;
    int i = 987654;
    long l = 1234567890L;
    float f = 3.14f;
    double d = 2.71828;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed type conversions in asm */
    __asm__ __volatile__ (
        "/* Mixed types test */\n\t"
        "movsx %[cin], %[iout]\n\t"          /* char to int */
        "mov %[sin], %w[sout]\n\t"           /* short to short */
        "cvtsi2sd %[iin], %[dout]\n\t"       /* int to double */
        "mov %[lin], %[iout2]"
        : [iout] "=r" (out_int),
          [sout] "=r" (out_short),
          [dout] "=x" (out_double),
          [iout2] "=r" (global_int)
        : [cin] "r" ((int)c),
          [sin] "r" ((int)s),
          [iin] "r" (i),
          [lin] "r" (l)
        : "memory"
    );
    
    /* Force mode change with explicit cast in asm operand */
    __asm__ __volatile__ (
        "mov %[float_as_int], %[out]"
        : [out] "=r" (out_int)
        : [float_as_int] "r" (*(int*)&f)
        : "memory"
    );
}

/* Test 4: Secondary reload triggers */
void test_secondary_reloads(void) {
    int value1 = 0x12345678;
    int value2 = 0x87654321;
    int result;
    
    /* Try to force specific register constraints */
    __asm__ __volatile__ (
        "/* Secondary reload test */\n\t"
        "mov %[v1], %%eax\n\t"
        "add %[v2], %%eax\n\t"
        "mov %%eax, %[res]"
        : [res] "=r" (result)
        : [v1] "r" (value1), [v2] "r" (value2)
        : "%eax", "memory", "cc"
    );
    
    /* Multiple clobbers to force spills */
    double d1 = 1.234, d2 = 5.678, d3 = 9.012;
    double dout1, dout2, dout3;
    
    __asm__ __volatile__ (
        "/* Multiple clobber test */\n\t"
        "movsd %[din1], %%xmm0\n\t"
        "addsd %[din2], %%xmm0\n\t"
        "movsd %%xmm0, %[dout1]\n\t"
        "movsd %[din3], %%xmm1\n\t"
        "mulsd %%xmm0, %%xmm1\n\t"
        "movsd %%xmm1, %[dout2]"
        : [dout1] "=m" (dout1), [dout2] "=m" (dout2)
        : [din1] "m" (d1), [din2] "m" (d2), [din3] "m" (d3)
        : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4",
          "%xmm5", "%xmm6", "%xmm7", "memory"
    );
}

/* Test 5: Complex addressing modes */
void test_complex_addressing(void) {
    int array[256];
    int *ptr = array;
    int index1, index2, result1, result2;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Complex addressing with multiple computations */
    index1 = compute_value(global_int);
    index2 = compute_value(global_int + 1);
    
    __asm__ __volatile__ (
        "/* Complex addressing test */\n\t"
        "mov (%[base], %[idx1], 4), %[res1]\n\t"
        "lea (%[base], %[idx2], 8), %[temp]\n\t"
        "mov (%[temp], %[idx1], 2), %[res2]"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [base] "r" (ptr),
          [idx1] "r" (index1),
          [idx2] "r" (index2),
          [temp] "r" (global_int)  /* dummy input to force register */
        : "memory"
    );
    
    global_int += result1 + result2;
}

/* Test 6: Volatile sequence with interdependencies */
void test_volatile_sequence(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int x, y, z;
    
    /* Chain of volatile asm blocks */
    __asm__ __volatile__ (
        "mov %[a], %[x]\n\t"
        "add %[b], %[x]"
        : [x] "=r" (x)
        : [a] "r" (a), [b] "r" (b)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[x], %[y]\n\t"
        "imul %[c], %[y]"
        : [y] "=r" (y)
        : [x] "r" (x), [c] "r" (c)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[y], %[z]\n\t"
        "sub %[d], %[z]"
        : [z] "=r" (z)
        : [y] "r" (y), [d] "r" (d)
        : "memory", "cc"
    );
    
    global_int += z;
}

/* Test 7: Memory clobber stress */
void test_memory_clobber(void) {
    int values[10];
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        values[i] = i * 10;
    }
    
    /* Force memory clobber between operations */
    for (int i = 0; i < 10; i++) {
        int temp;
        __asm__ __volatile__ (
            "mov %[val], %[tmp]"
            : [tmp] "=r" (temp)
            : [val] "m" (values[i])
            : "memory"
        );
        
        __asm__ __volatile__ (
            "add %[tmp], %[sum]"
            : [sum] "+r" (sum)
            : [tmp] "r" (temp)
            : "memory", "cc"
        );
    }
    
    global_int += sum;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    memset(global_array, 0, sizeof(global_array));
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 256);
    }
    
    /* Run all tests multiple times to increase reload pressure */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_many_operands();
        test_nested_calls();
        test_mixed_types();
        test_secondary_reloads();
        test_complex_addressing();
        test_volatile_sequence();
        test_memory_clobber();
        
        checksum += global_int;
        global_int = 42 + iteration; /* Reset with variation */
    }
    
    /* Final computation using inline asm */
    int final_result;
    __asm__ __volatile__ (
        "mov %[sum], %[final]\n\t"
        "add $0x1234, %[final]"
        : [final] "=r" (final_result)
        : [sum] "r" (checksum)
        : "cc"
    );
    
    printf("Final checksum: %d\n", final_result);
    return final_result;
}

/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -funroll-loops reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int* global_ptr = &global_int;

/* Helper functions for nested calls in asm operands */
int func_return_int(int x) { return x * 2 + 1; }
double func_return_double(double x) { return x * 1.5; }
int* func_return_ptr(int* p) { return p + 1; }
int compute_index(int i) { return (i * 7) % 256; }

/* Test 1: Many operands with mixed types to exhaust registers */
int test_many_operands(void) {
    /* Declare many variables of different types */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int out1, out2, out3, out4, out5, out6, out7, out8;
    char c1 = 'A', c2 = 'B';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000;
    long l1 = 10000, l2 = 20000;
    double d1 = 1.1, d2 = 2.2;
    void* p1 = &global_int;
    void* p2 = &global_double;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple output operands with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        "mov %[in5], %[out3]\n\t"
        "sub %[in6], %[out3]\n\t"
        "mov %[in7], %[out4]\n\t"
        "xor %[in8], %[out4]\n\t"
        /* Force memory operands */
        "mov %[mem1], %%rax\n\t"
        "add %%rax, %[out5]\n\t"
        "mov %[mem2], %%rbx\n\t"
        "add %%rbx, %[out6]\n\t"
        /* Use explicit register variables */
        "add %[reg1], %[out7]\n\t"
        "add %[reg2], %[out8]\n\t"
        /* Clobber many registers */
        :
        [out1] "=r" (out1),
        [out2] "=r" (out2),
        [out3] "=r" (out3),
        [out4] "=r" (out4),
        [out5] "=r" (out5),
        [out6] "=r" (out6),
        [out7] "=r" (out7),
        [out8] "=r" (out8)
        :
        [in1] "r" (i1),
        [in2] "r" (i2),
        [in3] "r" (l1),
        [in4] "r" (l2),
        [in5] "r" (s1),
        [in6] "r" (s2),
        [in7] "r" (c1),
        [in8] "r" (c2),
        [mem1] "m" (global_int),
        [mem2] "m" (global_double),
        [reg1] "r" (r1),
        [reg2] "r" (r2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5 + out6 + out7 + out8;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    int* presult;
    
    /* Function calls as input operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %[call1], %[out1]\n\t"
        "add %[call2], %[out1]\n\t"
        "mov %[call3], %[out2]\n\t"
        "add %[call4], %[out2]\n\t"
        /* Cast double to int through memory */
        "movq %[dbl], %%rax\n\t"
        "movq %%rax, %[out3]\n\t"
        :
        [out1] "=r" (result1),
        [out2] "=r" (result2),
        [out3] "=r" (result3)
        :
        [call1] "r" (func_return_int(10)),
        [call2] "r" (func_return_int(20)),
        [call3] "r" ((int)func_return_double(2.0)),  /* Cast forces mode change */
        [call4] "r" (global_int),
        [dbl] "x" (global_double)  /* xmm register constraint */
        : "rax", "memory"
    );
    
    /* Another asm with pointer arithmetic */
    int idx = compute_index(5);
    __asm__ __volatile__ (
        "mov (%[base], %[index], 4), %[out]\n\t"
        : [out] "=r" (result1)
        : [base] "r" (global_ptr),
          [index] "r" (idx)
        : "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c = 'X';
    short s = 1234;
    int i = 56789;
    long l = 123456789L;
    float f = 2.71828f;
    double d = 3.14159;
    int out_int;
    double out_double;
    
    /* Force mode changes through casts in asm operands */
    __asm__ __volatile__ (
        /* Mix different sized operands */
        "movsx %[char], %[out]\n\t"
        "add %[short], %[out]\n\t"
        "add %[int], %[out]\n\t"
        : [out] "=r" (out_int)
        : [char] "r" ((int)c),  /* Cast char to int */
          [short] "r" ((int)s),  /* Cast short to int */
          [int] "r" (i)
        : "cc"
    );
    
    /* Double to int conversion through memory */
    __asm__ __volatile__ (
        "cvttsd2si %[double], %[out]\n\t"
        : [out] "=r" (out_int)
        : [double] "x" (d)  /* xmm register */
        :
    );
    
    /* Int to double conversion */
    __asm__ __volatile__ (
        "cvtsi2sd %[int], %[out]\n\t"
        : [out] "=x" (out_double)
        : [int] "r" (i)  /* General purpose register */
        :
    );
    
    return out_int + (int)out_double;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int value = 42;
    int result;
    
    /* Try to force a value into specific register classes */
    __asm__ __volatile__ (
        /* 'a' constraint for accumulator */
        "mov %[in], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (result)
        : [in] "r" (value)  /* General reg -> needs move to eax */
        : "eax", "cc"
    );
    
    /* Multiple specific register constraints */
    register int rbx_var asm ("rbx") = 100;
    __asm__ __volatile__ (
        "mov %1, %%rax\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %%rax, %0\n\t"
        : "=r" (result)
        : "r" (value), "r" (rbx_var)
        : "rax", "rbx", "cc"
    );
    
    /* Memory operand with complex addressing */
    int array[100];
    for (int i = 0; i < 100; i++) array[i] = i;
    
    __asm__ __volatile__ (
        "mov (%[arr], %[idx], 4), %[out]\n\t"
        "add %[val], %[out]\n\t"
        : [out] "=r" (result)
        : [arr] "r" (array),
          [idx] "r" (compute_index(10)),  /* Function call in addressing */
          [val] "i" (5)  /* Immediate constraint */
        : "memory"
    );
    
    return result;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x, y, z;
    
    /* Chain of volatile asm statements */
    __asm__ __volatile__ (
        "mov %[a], %[x]\n\t"
        "add %[b], %[x]\n\t"
        : [x] "=r" (x)
        : [a] "r" (a),
          [b] "r" (b)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[x], %[y]\n\t"
        "imul %[c], %[y]\n\t"
        : [y] "=r" (y)
        : [x] "r" (x),
          [c] "r" (c)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[y], %[z]\n\t"
        "sub %[d], %[z]\n\t"
        "add %[e], %[z]\n\t"
        : [z] "=r" (z)
        : [y] "r" (y),
          [d] "r" (d),
          [e] "r" (e)
        : "cc"
    );
    
    /* Memory clobber between operations */
    __asm__ __volatile__ (
        "movl $0x12345678, %0\n\t"
        : "=m" (global_array[0])
        :
        : "memory"
    );
    
    return x + y + z;
}

/* Test 6: Complex addressing modes with multiple indices */
int test_complex_addressing(void) {
    int array1[256];
    int array2[256];
    int* ptr1 = array1;
    int* ptr2 = array2;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3;
    }
    
    int result1, result2, result3;
    
    /* Complex addressing with multiple computations */
    __asm__ __volatile__ (
        "mov (%[ptr1], %[idx1], 4), %[out1]\n\t"
        "add (%[ptr2], %[idx2], 4), %[out1]\n\t"
        "lea (%[out1], %[out1], 2), %[out2]\n\t"
        "mov %[out2], %[out3]\n\t"
        "neg %[out3]\n\t"
        : [out1] "=r" (result1),
          [out2] "=r" (result2),
          [out3] "=r" (result3)
        : [ptr1] "r" (ptr1),
          [ptr2] "r" (ptr2),
          [idx1] "r" (compute_index(7)),
          [idx2] "r" (compute_index(13))
        : "memory"
    );
    
    return result1 + result2 + result3;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)i;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    checksum += test_complex_addressing();
    
    /* Final volatile asm to ensure all code is used */
    __asm__ __volatile__ (
        "addl $0, %0\n\t"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte as exit code */
}

/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100] = {0};
char global_buffer[256] = {0};

/* Helper functions that return values needing computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(int x) {
    return (double)x / 7.0;
}

int* get_pointer(int index) {
    return &global_array[index];
}

char* get_buffer_ptr(int offset) {
    return &global_buffer[offset];
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5;
    int in0 = 100, in1 = 200, in2 = 300, in3 = 400;
    double d0 = 1.5, d1 = 2.5;
    char c0 = 'A', c1 = 'B';
    short s0 = 1000, s1 = 2000;
    
    /* Complex inline asm with many operands of different types */
    __asm__ __volatile__ (
        /* Multiple outputs */
        "mov %[in0], %[out0]\n\t"
        "add %[in1], %[out0]\n\t"
        "mov %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        /* Use explicit register variables */
        "add %%r12, %[out0]\n\t"
        "add %%r13, %[out1]\n\t"
        /* Mixed size operations */
        "movzx %[c0], %[out2]\n\t"
        "movsx %[s0], %[out3]\n\t"
        /* Force register pressure */
        "mov %[out0], %[out4]\n\t"
        "mov %[out1], %[out5]\n\t"
        : [out0] "=r" (out0), [out1] "=r" (out1),
          [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in0] "r" (in0), [in1] "r" (in1),
          [in2] "r" (in2), [in3] "r" (in3),
          [c0] "r" ((int)c0), [s0] "r" ((int)s0),
          "r" (r0), "r" (r1)
        : "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int* ptr1, *ptr2;
    
    /* Function calls as operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %[call1], %[res1]\n\t"
        "add %[call2], %[res1]\n\t"
        "mov %[ptr1], %[res2]\n\t"
        "add %[ptr2], %[res2]\n\t"
        "lea (%[res1],%[res2],2), %[res3]\n\t"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=r" (result3)
        : [call1] "r" (compute_value(global_int)),
          [call2] "r" (compute_value(global_int + 10)),
          [ptr1] "r" (get_pointer(global_int)),
          [ptr2] "r" (get_pointer(global_int * 2))
        : "memory", "cc"
    );
    
    /* Second asm with memory clobber to force reloads */
    int final_result;
    __asm__ __volatile__ (
        "add %[r1], %[r2]\n\t"
        "add %[r3], %[r2]\n\t"
        : [r2] "=r" (final_result)
        : [r1] "r" (result1),
          [r3] "r" (result3)
        : "memory"
    );
    
    return final_result + result2;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int int_val = 255;
    double double_val = 123.456;
    char char_val = 'X';
    short short_val = 32767;
    long long_val = 0xFFFFFFFF;
    
    int out_int;
    double out_double;
    char out_char;
    
    /* Force mode changes by using different types */
    __asm__ __volatile__ (
        /* Integer to different integer sizes */
        "mov %[intv], %%eax\n\t"
        "mov %%al, %[outc]\n\t"
        "mov %%ax, %[outs]\n\t"
        /* Double through integer register (no SSE) */
        "movq %[doublev], %%rax\n\t"
        "shr $32, %%rax\n\t"
        "mov %%eax, %[outi]\n\t"
        : [outi] "=r" (out_int),
          [outc] "=r" ((int)out_char),
          [outs] "=r" ((int)out_char)  /* Reuse for demonstration */
        : [intv] "r" (int_val),
          [doublev] "r" ((long)double_val)  /* Bit-cast double to long */
        : "rax", "memory", "cc"
    );
    
    /* Another asm with explicit register constraints */
    register double dreg asm ("rax") = double_val;
    int converted;
    __asm__ __volatile__ (
        "movq %[dreg], %[conv]\n\t"
        : [conv] "=r" (converted)
        : [dreg] "r" ((long)dreg)
        : "memory"
    );
    
    return out_int + converted + (int)out_char;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2, out3;
    
    /* Complex addressing modes */
    __asm__ __volatile__ (
        /* Force addressing computation */
        "lea (%[a],%[b],4), %[out1]\n\t"
        "imul %[c], %[out1]\n\t"
        /* Use specific register constraints */
        "mov %[out1], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        /* Chain operations */
        "mov %[out2], %[out3]\n\t"
        "neg %[out3]\n\t"
        : [out1] "=&r" (out1),  /* Early clobber */
          [out2] "=a" (out2),   /* Specific constraint */
          [out3] "=r" (out3)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c)
        : "memory", "cc"
    );
    
    /* Memory operand with complex expression */
    int index = global_int;
    int value;
    __asm__ __volatile__ (
        "mov (%[ptr],%[idx],4), %[val]\n\t"
        : [val] "=r" (value)
        : [ptr] "r" (global_array),
          [idx] "r" (compute_value(index))  /* Function call in index */
        : "memory"
    );
    
    return out1 + out2 + out3 + value;
}

/* Test 5: Pointer arithmetic and array indexing */
int test_pointer_arithmetic(void) {
    int sum = 0;
    int temp1, temp2, temp3;
    
    /* Complex pointer expressions */
    char* ptr1 = get_buffer_ptr(global_int);
    int* ptr2 = get_pointer(global_int * 2);
    
    __asm__ __volatile__ (
        /* Multiple memory accesses with computed addresses */
        "mov (%[p1],%[off1],1), %[t1]\n\t"
        "mov (%[p2],%[off2],4), %[t2]\n\t"
        "add %[t1], %[t2]\n\t"
        "mov %[t2], %[t3]\n\t"
        : [t1] "=r" (temp1),
          [t2] "=r" (temp2),
          [t3] "=r" (temp3)
        : [p1] "r" (ptr1),
          [p2] "r" (ptr2),
          [off1] "r" (compute_value(10)),
          [off2] "r" (compute_value(20))
        : "memory", "cc"
    );
    
    /* Volatile sequence creating dependency chain */
    int chain = temp3;
    for (int i = 0; i < 5; i++) {
        __asm__ __volatile__ (
            "add $1, %[ch]\n\t"
            "imul $3, %[ch]\n\t"
            : [ch] "+r" (chain)
            :
            : "cc"
        );
    }
    
    return temp1 + temp2 + temp3 + chain;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_pointer_arithmetic();
    
    /* Final volatile asm to ensure all code is used */
    __asm__ __volatile__ (
        "add $1, %0\n\t"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return lower byte */
}

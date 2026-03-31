/* reload_stress.c
 * A program designed to stress GCC's reload mechanism and trigger
 * the initialization block in push_reload (lines 1381-1399 of reload.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function that returns a value requiring computation */
int compute_value(int x) {
    return x * 2 + 1;
}

/* Function that returns a pointer with offset */
int* get_offset_ptr(int offset) {
    return global_ptr + offset;
}

/* Function that returns a double after computation */
double compute_double(int x) {
    return (double)x / 7.0;
}

/* Test 1: Many operands with mixed constraints to exhaust registers */
int test_many_operands(void) {
    int result = 0;
    
    /* Declare explicit register variables */
    register int r1 asm ("r12") = 100;
    register int r2 asm ("r13") = 200;
    register int r3 asm ("r14") = 300;
    
    int out1, out2, out3, out4, out5;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out2]\n\t"
        "imull %[r1], %[out3]\n\t"
        /* Mix in different data types */
        "movsbl %[c1], %[out4]\n\t"
        "movswl %[s1], %[out5]\n\t"
        /* Use explicit register variables */
        "addl %%r12d, %[out1]\n\t"
        "subl %%r13d, %[out2]\n\t"
        /* Clobber many registers */
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "i" (10),
          [r1] "r" (r1), [c1] "r" (c1), [s1] "r" (s1),
          "r" (r2), "r" (r3)  /* Additional register inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    result = out1 + out2 + out3 + out4 + out5;
    return result;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result = 0;
    int out1, out2;
    double dout;
    
    /* Function calls as direct operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "movl %[call1], %%eax\n\t"
        "addl %[call2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Use pointer from function call */
        "movl (%[ptr]), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [call1] "r" (compute_value(global_int)),
          [call2] "r" (compute_value(global_int + 1)),
          [ptr] "r" (get_offset_ptr(compute_value(2)))
        : "rax", "rbx", "rcx", "memory", "cc"
    );
    
    /* Mixed types with function calls */
    __asm__ __volatile__ (
        "cvtsi2sd %[intval], %%xmm0\n\t"
        "addsd %[dblval], %%xmm0\n\t"
        "movq %%xmm0, %[dout]\n\t"
        : [dout] "=r" (dout)
        : [intval] "r" (compute_value(5)),
          [dblval] "r" (compute_double(compute_value(3)))
        : "xmm0", "xmm1", "memory"
    );
    
    result = out1 + out2 + (int)dout;
    return result;
}

/* Test 3: Complex addressing modes with non-constant offsets */
int test_complex_addressing(void) {
    int result = 0;
    int out1, out2, out3;
    
    /* Initialize array with values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    /* Complex addressing with array indexing and pointer arithmetic */
    int index = compute_value(10);
    int offset = compute_value(5);
    
    __asm__ __volatile__ (
        /* Array access with computed index */
        "movsbl (%[arr], %[idx], 1), %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Pointer arithmetic in addressing */
        "movl (%[ptr], %[off], 4), %%ebx\n\t"
        "addl %%ebx, %[out1]\n\t"
        /* Multiple memory accesses */
        "movl %[global], %%ecx\n\t"
        "addl %%ecx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [arr] "r" (global_array), [idx] "r" (index),
          [ptr] "r" (global_ptr), [off] "r" (offset),
          [global] "m" (global_int)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Chain of volatile asm blocks with interdependent operands */
    __asm__ __volatile__ (
        "movl %[val1], %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%ebx\n\t"
        : "=b" (out3)
        : [val1] "r" (out1)
        : "rax", "cc"
    );
    
    result = out1 + out2 + out3;
    return result;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    char c_out;
    short s_out;
    int i_out;
    long long ll_out;
    double d_out;
    float f_out;
    
    char c_in = 'X';
    short s_in = 12345;
    int i_in = 0x12345678;
    long long ll_in = 0x123456789ABCDEF0LL;
    double d_in = 2.71828;
    float f_in = 1.414f;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        /* Char to int with sign extension */
        "movsbl %[cin], %%eax\n\t"
        "movl %%eax, %[iout]\n\t"
        /* Short to int */
        "movswl %[sin], %%ebx\n\t"
        "addl %%ebx, %[iout]\n\t"
        /* Double to int via conversion */
        "cvttsd2si %[din], %%ecx\n\t"
        "addl %%ecx, %[iout]\n\t"
        /* Float operations */
        "cvtss2sd %[fin], %%xmm0\n\t"
        "movq %%xmm0, %[dout]\n\t"
        : [iout] "=r" (i_out), [dout] "=r" (d_out),
          [cout] "=r" (c_out), [sout] "=r" (s_out)
        : [cin] "r" (c_in), [sin] "r" (s_in),
          [din] "r" (d_in), [fin] "r" (f_in),
          [llin] "r" (ll_in)
        : "rax", "rbx", "rcx", "xmm0", "xmm1", "memory", "cc"
    );
    
    /* Casts that force mode changes */
    __asm__ __volatile__ (
        "movq %[llin], %%rax\n\t"
        "shrq $32, %%rax\n\t"
        "movl %%eax, %[iout2]\n\t"
        : [iout2] "=r" (i_out)
        : [llin] "r" ((long long)i_in)  /* Cast forces different mode */
        : "rax", "rdx", "cc"
    );
    
    result = i_out + (int)d_out + c_out + s_out;
    return result;
}

/* Test 5: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int result = 0;
    int out1, out2;
    
    /* Try to force moves through specific registers */
    int value = compute_value(42);
    
    /* Using specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        /* 'a' constraint for accumulator */
        "movl %[val], %%eax\n\t"
        "addl $1, %%eax\n\t"
        /* 'b' constraint for base register */
        "movl %%eax, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        : [out1] "=r" (out1)
        : [val] "a" (value)  /* 'a' constraint */
        : "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Multiple specific constraints */
    __asm__ __volatile__ (
        "xchgl %%eax, %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out2] "=r" (out2)
        : "a" (out1), "b" (value)  /* Both 'a' and 'b' constraints */
        : "cc"
    );
    
    /* Memory constraint that may need secondary reload */
    double dval = global_double;
    long long ll_out;
    
    __asm__ __volatile__ (
        "movq %[dval], %%rax\n\t"
        "movq %%rax, %[llout]\n\t"
        : [llout] "=r" (ll_out)
        : [dval] "m" (dval)  /* Memory constraint */
        : "rax", "memory"
    );
    
    result = out1 + out2 + (int)ll_out;
    return result;
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times to increase reload opportunities */
    for (int i = 0; i < 3; i++) {
        checksum += test_many_operands();
        checksum += test_nested_calls();
        checksum += test_complex_addressing();
        checksum += test_mixed_types();
        checksum += test_secondary_reloads();
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return deterministic value for verification */
    return checksum % 256;
}

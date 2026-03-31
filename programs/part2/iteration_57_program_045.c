/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload pass by creating inline
 * assembly statements that force the register allocator to generate many
 * reloads, including secondary reloads. The goal is to trigger the
 * initialization block in push_reload (lines 1381-1399 of reload.cc).
 *
 * Compilation recommendations:
 *   gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -c reload_stress.c
 *   gcc -O2 -funroll-loops -fno-optimize-sibling-calls -march=x86-64 -c reload_stress.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile char global_char = 'A';
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Function that returns a value, forcing evaluation before assembly */
int get_value(int x) {
    return x * 2 + 1;
}

double compute_double(int a, double b) {
    return b + (double)a;
}

/* Test 1: Many operands with mixed types to exhaust registers */
int test_many_operands(void) {
    int result = 0;
    
    /* Declare explicit register variables */
    register int r1 asm ("r10") = global_int;
    register int r2 asm ("r11") = global_int + 1;
    register double d1 asm ("xmm0") = global_double; /* Will be forced to integer regs with -mno-sse */
    
    int out1, out2, out3;
    double dout1;
    char cout;
    
    /* Complex inline asm with many input/output/clobbered operands */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]\n\t"
        "imull %[in3], %[out2]\n\t"
        /* Force a mode change: using double as integer */
        "movq %[din1], %%rax\n\t"
        "shrq $32, %%rax\n\t"
        "movl %%eax, %[out3]\n\t"
        /* Character operation */
        "movb %[cin], %%al\n\t"
        "addb $1, %%al\n\t"
        "movb %%al, %[cout]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [cout] "=r" (cout), [dout1] "=r" (dout1)  /* '=r' for double when SSE disabled */
        : [in1] "r" (r1), [in2] "r" (r2), [in3] "r" (global_int),
          [din1] "r" ((int)d1),  /* Cast double to int to force mode change */
          [cin] "r" (global_char),
          "m" (global_array[0])   /* Memory constraint forces address computation */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    result = out1 + out2 + out3 + cout;
    return result;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result = 0;
    int out1, out2;
    double dout;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Complex address computation */
        "leaq %[arrptr], %%rbx\n\t"
        "movl (%%rbx, %[idx], 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), [dout] "=r" (dout)
        : [in1] "r" (get_value(global_int)),
          [in2] "r" (get_value(get_value(global_int))), /* Nested call */
          [arrptr] "m" (global_array[0]),
          [idx] "r" (get_value(5) % 50)  /* Dynamic index calculation */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "r10", "r11", "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    result = out1 + out2;
    return result;
}

/* Test 3: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int result = 0;
    int out1, out2;
    
    /* Use specific register constraints that may require secondary reloads */
    register int forced_ax asm ("ax") = global_int;  /* 16-bit register constraint */
    
    __asm__ __volatile__ (
        /* Force use of specific registers */
        "movw %w[in_ax], %%ax\n\t"
        "movzwl %%ax, %%ebx\n\t"
        "addl %[in_mem], %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        /* Another operation with mixed constraints */
        "movl %[in_r], %%ecx\n\t"
        "imull %%ecx, %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in_ax] "r" (forced_ax),
          [in_mem] "m" (global_array[get_value(2)]),  /* Memory with computed address */
          [in_r] "r" (get_value(global_int)),
          "i" (12345)  /* Immediate constraint */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "r10", "r11", "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    result = out1 + out2;
    return result;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    char c1 = 'Z', c2 = 'A';
    short s1 = 1000, s2 = 2000;
    int i1 = 1000000, i2 = 2000000;
    double d1 = 1.23456, d2 = 7.89012;
    long long ll1 = 0x123456789ABCDEF0LL;
    
    char cout;
    short sout;
    int iout;
    double dout;
    long long llout;
    
    /* Assembly with many different types */
    __asm__ __volatile__ (
        /* Character operations */
        "movb %[c1], %%al\n\t"
        "subb %[c2], %%al\n\t"
        "movb %%al, %[cout]\n\t"
        /* Short operations */
        "movw %[s1], %%ax\n\t"
        "addw %[s2], %%ax\n\t"
        "movw %%ax, %[sout]\n\t"
        /* Integer operations */
        "movl %[i1], %%ebx\n\t"
        "xorl %[i2], %%ebx\n\t"
        "movl %%ebx, %[iout]\n\t"
        /* Double operations (forced to integer regs) */
        "movq %[d1], %%rcx\n\t"
        "movq %[d2], %%rdx\n\t"
        "addq %%rcx, %%rdx\n\t"
        "movq %%rdx, %[dout]\n\t"
        /* Long long operation */
        "movq %[ll1], %%r8\n\t"
        "rorq $32, %%r8\n\t"
        "movq %%r8, %[llout]\n\t"
        : [cout] "=r" (cout), [sout] "=r" (sout), [iout] "=r" (iout),
          [dout] "=r" (dout), [llout] "=r" (llout)
        : [c1] "r" (c1), [c2] "r" (c2),
          [s1] "r" (s1), [s2] "r" (s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [d1] "r" ((long long)d1),  /* Cast to force mode change */
          [d2] "r" ((long long)d2),
          [ll1] "r" (ll1)
        : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    result = cout + sout + iout + (int)dout + (int)llout;
    return result;
}

/* Test 5: Chain of volatile assembly blocks with interdependent operands */
int test_chain_blocks(void) {
    int result = 0;
    int val1 = global_int;
    int val2, val3, val4, val5;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "movl %[in1], %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        : [out1] "=r" (val2)
        : [in1] "r" (val1)
        : "rax", "rbx", "cc"
    );
    
    /* Chain 2 - depends on result from chain 1 */
    __asm__ __volatile__ (
        "movl %[in1], %%ecx\n\t"
        "addl $100, %%ecx\n\t"
        "movl %%ecx, %[out1]\n\t"
        "movl %[in2], %%edx\n\t"
        "subl %%ecx, %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        : [out1] "=r" (val3), [out2] "=r" (val4)
        : [in1] "r" (val2), [in2] "r" (get_value(val2))
        : "rcx", "rdx", "cc", "memory"
    );
    
    /* Chain 3 - depends on previous results */
    __asm__ __volatile__ (
        "movl %[in1], %%r8d\n\t"
        "movl %[in2], %%r9d\n\t"
        "imull %%r9d, %%r8d\n\t"
        "addl %[in3], %%r8d\n\t"
        "movl %%r8d, %[out1]\n\t"
        : [out1] "=r" (val5)
        : [in1] "r" (val3), [in2] "r" (val4),
          [in3] "m" (global_array[val3 % 100])  /* Memory with computed index */
        : "r8", "r9", "cc", "memory"
    );
    
    result = val2 + val3 + val4 + val5;
    return result;
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array with values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Run all tests to stress reload pass */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_secondary_reloads();
    checksum += test_mixed_types();
    checksum += test_chain_blocks();
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (checksum)
        : "memory"
    );
    
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}

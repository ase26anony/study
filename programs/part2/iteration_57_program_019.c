/* reload_stress.c - Stress test for GCC reload.cc push_reload logic */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
char global_buffer[256];

/* Function to force evaluation into register */
int get_value(int x) {
    return x * 2 + 1;
}

double compute_double(int a, double b) {
    return b + (double)a;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5;
    int in0 = global_int;
    int in1 = get_value(10);
    int in2 = global_array[5];
    int in3 = r0 + r1;
    double d0 = global_double;
    double d1 = 2.71828;
    char c0 = 'A';
    short s0 = 32767;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        "movl %[in0], %%eax\n\t"
        "addl %[in1], %%eax\n\t"
        "imull %[in2], %%eax\n\t"
        "addl %%r12d, %%eax\n\t"
        "movl %%eax, %[out0]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl $100, %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "movq %[d0], %%xmm0\n\t"
        "cvttsd2si %%xmm0, %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "movzwl %w[s0], %%edx\n\t"
        "addl $1, %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        "movsbl %b[c0], %%edi\n\t"
        "movl %%edi, %[out4]\n\t"
        "leaq %[garr], %%rsi\n\t"
        "movl (%%rsi), %%r8d\n\t"
        "movl %%r8d, %[out5]"
        : [out0] "=r" (out0), [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4), [out5] "=r" (out5)
        : [in0] "r" (in0), [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [d0] "x" (d0), [s0] "r" (s0), [c0] "r" (c0), [garr] "m" (global_array)
        : "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    
    /* Function calls directly in asm operands */
    __asm__ __volatile__ (
        "movl %[call1], %%eax\n\t"
        "addl %[call2], %%eax\n\t"
        "movl %%eax, %[res1]\n\t"
        "cvttsd2si %[dcalc], %%ebx\n\t"
        "movl %%ebx, %[res2]\n\t"
        "leaq %[buf], %%rcx\n\t"
        "movl (%%rcx, %[idx], 4), %%edx\n\t"
        "movl %%edx, %[res3]"
        : [res1] "=r" (result1), [res2] "=r" (result2), [res3] "=r" (result3)
        : [call1] "r" (get_value(global_int)),
          [call2] "r" (get_value(get_value(5))),
          [dcalc] "x" (compute_double(global_int, global_double)),
          [buf] "m" (global_buffer),
          [idx] "r" (get_value(3) % 64)
        : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory", "cc"
    );
    
    /* Another volatile block with memory clobber */
    __asm__ __volatile__ (
        "movl %[val], %%eax\n\t"
        "notl %%eax"
        : "=a" (dresult)
        : [val] "r" ((int)global_double)
        : "memory"
    );
    
    return result1 + result2 + result3 + (int)dresult;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'X', c2 = 'Y';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 3000000000L, l2 = 4000000000L;
    float f1 = 1.234f, f2 = 5.678f;
    double d1 = 9.876, d2 = 5.432;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        "movsbl %b[c1], %%eax\n\t"
        "movswl %w[s1], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[i1], %%eax\n\t"
        "movl %%eax, %[oi]\n\t"
        "movsbw %b[c2], %%cx\n\t"
        "addw %w[s2], %%cx\n\t"
        "movw %%cx, %[os]\n\t"
        "movsbl %b[c1], %%edx\n\t"
        "movb %%dl, %[oc]\n\t"
        "movq %[d1], %%xmm0\n\t"
        "addsd %[d2], %%xmm0\n\t"
        "movq %%xmm0, %[od]"
        : [oi] "=r" (out_int), [os] "=r" (out_short),
          [oc] "=r" (out_char), [od] "=x" (out_double)
        : [c1] "r" (c1), [s1] "r" (s1), [i1] "r" (i1),
          [c2] "r" (c2), [s2] "r" (s2),
          [d1] "x" (d1), [d2] "x" (d2)
        : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory", "cc"
    );
    
    /* Force mode change through cast in operand */
    __asm__ __volatile__ (
        "cvtsi2sd %[ival], %%xmm0\n\t"
        "addsd %[dval], %%xmm0"
        : "=x" (out_double)
        : [ival] "r" ((int)l1), [dval] "x" (d2)
        : "xmm0", "memory"
    );
    
    return out_int + out_short + out_char + (int)out_double;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    register int acc asm ("rax") = 0;
    register int counter asm ("rcx") = 100;
    
    /* Force specific register constraints that may need secondary reloads */
    __asm__ __volatile__ (
        "movl $0, %%eax\n\t"
        "1:\n\t"
        "addl %[val], %%eax\n\t"
        "decl %%ecx\n\t"
        "jnz 1b"
        : "=a" (result), "+c" (counter)
        : [val] "ri" (get_value(1))  /* 'ri' constraint: register or immediate */
        : "memory", "cc"
    );
    
    /* Complex addressing with multiple index registers */
    int idx1 = get_value(2);
    int idx2 = get_value(3);
    int idx3 = get_value(4);
    
    __asm__ __volatile__ (
        "leaq %[arr], %%r8\n\t"
        "movl (%%r8, %[i1], 4), %%r9d\n\t"
        "addl (%%r8, %[i2], 4), %%r9d\n\t"
        "addl (%%r8, %[i3], 4), %%r9d\n\t"
        "movl %%r9d, %[res]"
        : [res] "=r" (result)
        : [arr] "m" (global_array),
          [i1] "r" (idx1), [i2] "r" (idx2), [i3] "r" (idx3)
        : "r8", "r9", "r10", "memory", "cc"
    );
    
    return result;
}

/* Test 5: Chain of dependent asm blocks */
int test_chain_reloads(void) {
    int v1, v2, v3, v4, v5;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "movl $100, %[out]"
        : [out] "=r" (v1)
        :
        : "memory"
    );
    
    /* Chain 2 depends on chain 1 */
    __asm__ __volatile__ (
        "addl $50, %[in]\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (v2)
        : [in] "0" (v1)
        : "memory", "cc"
    );
    
    /* Chain 3 with function call */
    v3 = get_value(v2);
    
    /* Chain 4 uses result in complex addressing */
    __asm__ __volatile__ (
        "leaq %[arr], %%rbx\n\t"
        "movl (%%rbx, %[idx], 4), %%eax"
        : "=a" (v4)
        : [arr] "m" (global_array), [idx] "r" (v3 % 50)
        : "rbx", "memory"
    );
    
    /* Chain 5 final computation */
    __asm__ __volatile__ (
        "imull $2, %[in1], %%eax\n\t"
        "addl %[in2], %%eax"
        : "=a" (v5)
        : [in1] "r" (v2), [in2] "r" (v4)
        : "memory", "cc"
    );
    
    return v1 + v2 + v3 + v4 + v5;
}

/* Main function orchestrating all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = (char)(i % 128);
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_chain_reloads();
    
    /* Final asm to ensure all values are used */
    __asm__ __volatile__ (
        "addl $1, %0"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    return checksum;
}

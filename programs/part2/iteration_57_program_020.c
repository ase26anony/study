/* reload_stress_test.c - Stress GCC's reload mechanism to hit uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create register pressure */
int global_int1 = 123;
int global_int2 = 456;
int global_int3 = 789;
double global_double1 = 3.14159;
double global_double2 = 2.71828;
char global_char_array[256];
int global_int_array[100];

/* Function prototypes */
int test_many_operands(void);
int test_nested_calls(void);
int test_mixed_types(void);
int test_secondary_reloads(void);
int test_register_variables(void);
int test_memory_clobber_chains(void);

/* Helper functions for nested calls */
int func_return_int(int x) { return x * 2 + 1; }
double func_return_double(double x) { return x * 1.5; }
int* func_return_ptr(int* p) { return p + 1; }

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 3;
    }
    
    /* Run all test patterns to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_register_variables();
    checksum += test_memory_clobber_chains();
    
    printf("Final checksum: %d\n", checksum);
    return checksum;
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4, out5, out6, out7, out8;
    int in1 = global_int1;
    int in2 = global_int2;
    int in3 = global_int3;
    int in4 = 42;
    int in5 = 99;
    int in6 = 777;
    int in7 = 888;
    int in8 = 999;
    double din1 = global_double1;
    double din2 = global_double2;
    
    /* Complex inline asm with many operands - forces register allocation pressure */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "sub %[in4], %[out2]\n\t"
        "imul %[in5], %[out1]\n\t"
        "lea (%[in6],%[in7],2), %[out3]\n\t"
        "mov %[in8], %[out4]\n\t"
        /* Force memory operand */
        "movl %[in1], (%[mem])\n\t"
        /* Use floating point value in integer context */
        "movq %[din1], %%rax\n\t"
        "shr $32, %%rax\n\t"
        "mov %%eax, %[out5]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5),
          [out6] "=m" (global_int_array[10]),
          [out7] "=r" (out7),
          [out8] "=r" (out8)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [in4] "r" (in4),
          [in5] "r" (in5),
          [in6] "r" (in6),
          [in7] "r" (in7),
          [in8] "r" (in8),
          [din1] "x" (din1),
          [din2] "x" (din2),
          [mem] "r" (global_int_array)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "cc", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5 + out7 + out8;
}

/* Test 2: Nested function calls within asm operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    int* ptr_result;
    
    /* Function calls in input operands force evaluation before asm */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "movq %[dcall], %%xmm0\n\t"
        "movq %%xmm0, %[dres]\n\t"
        "mov %[ptr], %[pres]\n\t"
        : [res1] "=r" (result1),
          [dres] "=x" (dresult),
          [pres] "=r" (ptr_result)
        : [call1] "r" (func_return_int(global_int1)),
          [call2] "r" (func_return_int(global_int2)),
          [dcall] "x" (func_return_double(global_double1)),
          [ptr] "r" (func_return_ptr(global_int_array))
        : "rax", "xmm0", "memory"
    );
    
    /* Another asm with complex address computation */
    int index = global_int1 % 50;
    __asm__ __volatile__ (
        "mov (%[base],%[idx],4), %[res2]\n\t"
        "add $1, %[res2]\n\t"
        : [res2] "=r" (result2)
        : [base] "r" (global_int_array),
          [idx] "r" (index + func_return_int(1))
        : "memory"
    );
    
    /* Mixed addressing modes */
    __asm__ __volatile__ (
        "lea (%[base],%[idx1],8), %[res3]\n\t"
        "add (%[base],%[idx2],4), %[res3]\n\t"
        : [res3] "=r" (result3)
        : [base] "r" (global_int_array),
          [idx1] "r" (func_return_int(global_int1) % 20),
          [idx2] "r" (func_return_int(global_int2) % 20)
        : "memory"
    );
    
    return result1 + result2 + result3 + (int)dresult + (int)(ptr_result - global_int_array);
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B', cout;
    short s1 = 1000, s2 = 2000, sout;
    int i1 = 1000000, i2 = 2000000, iout;
    long l1 = 3000000000L, l2 = 4000000000L, lout;
    float f1 = 1.234f, f2 = 5.678f;
    double d1 = 9.876, d2 = 5.432, dout;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        /* Char operations */
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "mov %%al, %[cout]\n\t"
        /* Short operations */
        "mov %[s1], %%ax\n\t"
        "sub %[s2], %%ax\n\t"
        "mov %%ax, %[sout]\n\t"
        /* Integer operations */
        "mov %[i1], %%eax\n\t"
        "imul %[i2], %%eax\n\t"
        "mov %%eax, %[iout]\n\t"
        /* Long operations - different mode */
        "mov %[l1], %%rax\n\t"
        "add %[l2], %%rax\n\t"
        "mov %%rax, %[lout]\n\t"
        /* Float to integer conversion */
        "movd %[f1], %%xmm0\n\t"
        "cvttss2si %%xmm0, %%ebx\n\t"
        "add %%ebx, %[iout]\n\t"
        /* Double operations */
        "movq %[d1], %%xmm1\n\t"
        "addsd %[d2], %%xmm1\n\t"
        "movq %%xmm1, %[dout]\n\t"
        : [cout] "=r" (cout),
          [sout] "=r" (sout),
          [iout] "=r" (iout),
          [lout] "=r" (lout),
          [dout] "=x" (dout)
        : [c1] "r" ((int)c1),
          [c2] "r" ((int)c2),
          [s1] "r" ((int)s1),
          [s2] "r" ((int)s2),
          [i1] "r" (i1),
          [i2] "r" (i2),
          [l1] "r" (l1),
          [l2] "r" (l2),
          [f1] "x" (f1),
          [d1] "x" (d1),
          [d2] "x" (d2)
        : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "cc", "memory"
    );
    
    return cout + sout + iout + (int)lout + (int)dout;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Use explicit register variables */
    register int r12_var asm ("r12") = 0x1234;
    register int r13_var asm ("r13") = 0x5678;
    register int r14_var asm ("r14") = 0x9ABC;
    
    /* Force moves between specific registers */
    __asm__ __volatile__ (
        /* Try to force a secondary reload by using specific constraints */
        "mov %[in1], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[r12], %%ebx\n\t"
        "add %%ebx, %[out1]\n\t"
        "mov %[r13], %%ecx\n\t"
        "add %%ecx, %[out1]\n\t"
        /* Force memory operand with complex addressing */
        "movl %[out1], (%[mem],%[idx],4)\n\t"
        : [out1] "=r" (result)
        : [in1] "i" (0x1000),  /* Immediate constraint */
          [r12] "r" (r12_var),
          [r13] "r" (r13_var),
          [mem] "r" (global_int_array),
          [idx] "r" (r14_var & 0xF)
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Another attempt with flag register constraints */
    int flag_test = 0;
    __asm__ __volatile__ (
        "test %[a], %[a]\n\t"
        "setz %b[flag]\n\t"
        : [flag] "=r" (flag_test)
        : [a] "r" (result)
        : "cc"
    );
    
    /* Mix of constraints that are hard to satisfy */
    int a = 10, b = 20, c = 30;
    __asm__ __volatile__ (
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "mov %%eax, %[c]\n\t"
        "imul %[imm], %%eax\n\t"
        : [c] "+r" (c)
        : [a] "r" (a),
          [b] "m" (b),  /* Memory constraint */
          [imm] "i" (5) /* Immediate constraint */
        : "rax", "cc", "memory"
    );
    
    return result + flag_test + c;
}

/* Test 5: Explicit register variables with complex usage */
int test_register_variables(void) {
    /* Declare multiple register variables */
    register int reg1 asm ("r10") = 111;
    register int reg2 asm ("r11") = 222;
    register double reg3 asm ("xmm15") = 3.33;
    register void* reg4 asm ("r9") = (void*)global_int_array;
    
    int out1, out2;
    double dout;
    
    /* Use register variables in asm with other constraints */
    __asm__ __volatile__ (
        "mov %[reg1], %[out1]\n\t"
        "add %[reg2], %[out1]\n\t"
        "movq %[reg3], %%xmm0\n\t"
        "addsd %[dbl], %%xmm0\n\t"
        "movq %%xmm0, %[dout]\n\t"
        "mov (%[reg4],%[idx],4), %[out2]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [dout] "=x" (dout)
        : [reg1] "r" (reg1),
          [reg2] "r" (reg2),
          [reg3] "x" (reg3),
          [reg4] "r" (reg4),
          [dbl] "x" (global_double1),
          [idx] "r" (reg1 & 0xF)
        : "xmm0", "memory"
    );
    
    /* Chain of asm statements with register variables */
    __asm__ __volatile__ (
        "add $100, %[reg1]\n\t"
        : [reg1] "+r" (reg1)
        :
        : "cc"
    );
    
    __asm__ __volatile__ (
        "sub $50, %[reg2]\n\t"
        : [reg2] "+r" (reg2)
        :
        : "cc"
    );
    
    return out1 + out2 + (int)dout + reg1 + reg2;
}

/* Test 6: Memory clobber chains with interdependent operands */
int test_memory_clobber_chains(void) {
    int chain1 = 1, chain2 = 2, chain3 = 3, chain4 = 4;
    int result = 0;
    
    /* Sequence of volatile asm blocks with memory clobbers */
    __asm__ __volatile__ (
        "mov %[c1], %%eax\n\t"
        "add $10, %%eax\n\t"
        "mov %%eax, %[c2]\n\t"
        : [c2] "=r" (chain2)
        : [c1] "r" (chain1)
        : "rax", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[c2], %%ebx\n\t"
        "imul $2, %%ebx\n\t"
        "mov %%ebx, %[c3]\n\t"
        : [c3] "=r" (chain3)
        : [c2] "r" (chain2)
        : "rbx", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[c3], %%ecx\n\t"
        "lea (%%ecx,%%ecx,2), %%ecx\n\t"
        "mov %%ecx, %[c4]\n\t"
        : [c4] "=r" (chain4)
        : [c3] "r" (chain3)
        : "rcx", "memory"
    );
    
    /* Final computation using all chain values */
    __asm__ __volatile__ (
        "mov %[c1], %%eax\n\t"
        "add %[c2], %%eax\n\t"
        "add %[c3], %%eax\n\t"
        "add %[c4], %%eax\n\t"
        "mov %%eax, %[res]\n\t"
        : [res] "=r" (result)
        : [c1] "r" (chain1),
          [c2] "r" (chain2),
          [c3] "r" (chain3),
          [c4] "r" (chain4)
        : "rax", "memory"
    );
    
    /* Another chain with floating point */
    double dchain1 = 1.1, dchain2, dchain3;
    __asm__ __volatile__ (
        "movq %[dc1], %%xmm0\n\t"
        "addsd %[dglobal], %%xmm0\n\t"
        "movq %%xmm0, %[dc2]\n\t"
        : [dc2] "=x" (dchain2)
        : [dc1] "x" (dchain1),
          [dglobal] "x" (global_double2)
        : "xmm0", "memory"
    );
    
    __asm__ __volatile__ (
        "movq %[dc2], %%xmm1\n\t"
        "mulsd %[dc1], %%xmm1\n\t"
        "movq %%xmm1, %[dc3]\n\t"
        : [dc3] "=x" (dchain3)
        : [dc2] "x" (dchain2),
          [dc1] "x" (dchain1)
        : "xmm1", "memory"
    );
    
    return result + (int)dchain2 + (int)dchain3;
}

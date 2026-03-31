/* reload_test.c - Designed to trigger push_reload initialization block */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
char global_buffer[256];

/* Helper functions for nested calls */
int func_return_int(int x) { return x * 2 + 1; }
double func_return_double(double x) { return x * 1.5; }
int* func_return_ptr(int* p) { return p + 1; }

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int out1, out2, out3, out4;
    
    /* Complex inline assembly with many operands */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
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
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h)
        : "cc", "memory"
    );
    
    /* Second assembly block with different register pressure */
    __asm__ __volatile__ (
        "mov %[i], %%eax\n\t"
        "add %[j], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[k], %%ebx\n\t"
        "imul %[l], %%ebx\n\t"
        "mov %%ebx, %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [i] "r" (i), [j] "r" (j), [k] "r" (k), [l] "r" (l)
        : "eax", "ebx", "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 1000000L, l2 = 2000000L;
    float f1 = 1.234f, f2 = 5.678f;
    double d1 = 9.876, d2 = 5.432;
    int out_int;
    double out_double;
    
    /* Assembly with mixed type constraints */
    __asm__ __volatile__ (
        /* Force mode changes by using different sized operands */
        "movsx %[c1], %%eax\n\t"
        "add %[s1], %%ax\n\t"
        "add %[i1], %%eax\n\t"
        "mov %%eax, %[out_int]"
        : [out_int] "=r" (out_int)
        : [c1] "r" ((int)c1), [s1] "r" ((int)s1), [i1] "r" (i1)
        : "eax", "cc"
    );
    
    /* Double to int conversion requiring mode change */
    __asm__ __volatile__ (
        "cvtsd2si %[d1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "mov %%eax, %[out_int]"
        : [out_int] "=r" (out_int)
        : [d1] "x" (d1), [i2] "r" (i2)
        : "eax", "cc"
    );
    
    /* Float operations with memory constraints */
    __asm__ __volatile__ (
        "movss %[f1], %%xmm0\n\t"
        "addss %[f2], %%xmm0\n\t"
        "cvtss2sd %%xmm0, %%xmm1\n\t"
        "movsd %%xmm1, %[out_double]"
        : [out_double] "=m" (out_double)
        : [f1] "m" (f1), [f2] "m" (f2)
        : "xmm0", "xmm1", "memory"
    );
    
    return out_int + (int)out_double;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int result = 0;
    int temp1, temp2, temp3;
    
    /* Function calls as input operands */
    __asm__ __volatile__ (
        "add %[call1], %[call2]\n\t"
        "mov %%eax, %[out]"
        : [out] "=r" (temp1)
        : [call1] "r" (func_return_int(global_int)),
          [call2] "r" (func_return_int(global_int + 1))
        : "eax", "cc", "memory"
    );
    
    /* Complex addressing with array indexing */
    __asm__ __volatile__ (
        "mov (%[ptr],%[idx],4), %%eax\n\t"
        "add %%eax, %[out]"
        : [out] "+r" (temp2)
        : [ptr] "r" (global_array),
          [idx] "r" (func_return_int(10))
        : "eax", "cc", "memory"
    );
    
    /* Pointer arithmetic in operand */
    __asm__ __volatile__ (
        "mov (%[base],%[offset]), %%eax\n\t"
        "mov %%eax, %[out]"
        : [out] "=r" (temp3)
        : [base] "r" (global_buffer),
          [offset] "r" (func_return_int(32) * sizeof(char))
        : "eax", "cc", "memory"
    );
    
    result = temp1 + temp2 + temp3;
    
    /* Chain of dependent assembly blocks */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "imul %[in2], %%eax\n\t"
        "mov %%eax, %[out]"
        : [out] "=r" (result)
        : [in1] "r" (result),
          [in2] "r" (func_return_double(2.0))
        : "eax", "cc"
    );
    
    return result;
}

/* Test 4: Explicit register variables and secondary reloads */
int test_explicit_registers(void) {
    /* Explicit register variables */
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    register int r14_var asm ("r14") = 300;
    int out1, out2, out3;
    
    /* Using explicit registers in assembly */
    __asm__ __volatile__ (
        "mov %[r12], %%eax\n\t"
        "add %[r13], %%eax\n\t"
        "mov %%eax, %[out1]"
        : [out1] "=r" (out1)
        : [r12] "r" (r12_var), [r13] "r" (r13_var)
        : "eax", "cc"
    );
    
    /* Force register moves with specific constraints */
    __asm__ __volatile__ (
        /* Try to force accumulator use */
        "mov %[val], %%eax\n\t"
        "test %%eax, %%eax\n\t"
        "setg %%al\n\t"
        "movzx %%al, %[out2]"
        : [out2] "=r" (out2)
        : [val] "r" (r14_var)
        : "eax", "cc"
    );
    
    /* Memory operand with complex addressing */
    __asm__ __volatile__ (
        "lea (%[base],%[index],4), %%rax\n\t"
        "mov (%%rax), %[out3]"
        : [out3] "=r" (out3)
        : [base] "r" (global_array),
          [index] "r" (func_return_int(25))
        : "rax", "cc", "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 5: Complex constraints and memory clobbers */
int test_complex_constraints(void) {
    int a = 1, b = 2, c = 3;
    int out_mem, out_reg, out_imm;
    
    /* Mix of register, memory, and immediate constraints */
    __asm__ __volatile__ (
        "movl $123, %[out_imm]\n\t"
        "movl %[a], %[out_reg]\n\t"
        "movl %[out_reg], %[out_mem]"
        : [out_imm] "=r" (out_imm),
          [out_reg] "=r" (out_reg),
          [out_mem] "=m" (out_mem)
        : [a] "r" (a),
          "i" (456)  /* Immediate input */
        : "cc", "memory"
    );
    
    /* String operation with implicit registers */
    char src[50] = "Test string for reload";
    char dst[50];
    
    __asm__ __volatile__ (
        "cld\n\t"
        "mov %[src], %%rsi\n\t"
        "mov %[dst], %%rdi\n\t"
        "mov $22, %%rcx\n\t"
        "rep movsb"
        : 
        : [src] "r" (src), [dst] "r" (dst)
        : "rsi", "rdi", "rcx", "cc", "memory"
    );
    
    /* Volatile assembly with side effects */
    int counter = 0;
    for (int i = 0; i < 10; i++) {
        __asm__ __volatile__ (
            "add $1, %[cnt]"
            : [cnt] "+m" (counter)
            :
            : "cc"
        );
    }
    
    return out_imm + out_reg + counter;
}

/* Main function orchestrating all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests to trigger various reload patterns */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_complex_constraints();
    
    /* Final assembly with many clobbers to force spills */
    __asm__ __volatile__ (
        "mov %[sum], %%eax\n\t"
        "add $999, %%eax\n\t"
        "mov %%eax, %[sum]"
        : [sum] "+r" (checksum)
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory"
    );
    
    printf("Final checksum: %d\n", checksum);
    return checksum;
}

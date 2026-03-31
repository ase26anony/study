/* reload_stress_test.c
 * Designed to trigger push_reload initialization (lines 1381-1399 in reload.cc)
 * through complex inline assembly patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create register pressure */
int global_int1 = 1, global_int2 = 2, global_int3 = 3, global_int4 = 4;
double global_double1 = 1.1, global_double2 = 2.2;
char global_char_array[256];
int global_int_array[100];

/* Function that returns values - used in nested calls */
int func_return_int(int x) { return x * 2; }
double func_return_double(double x) { return x * 1.5; }
void* func_return_ptr(void* p) { return (char*)p + 1; }

/* Test 1: Many operands with mixed constraints */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 10, in2 = 20, in3 = 30, in4 = 40;
    double dout1, dout2;
    double din1 = 1.5, din2 = 2.5;
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = 100;
    register int reg_var2 asm ("r13") = 200;
    register double reg_double asm ("xmm0") = 3.14;
    
    /* Complex assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        /* Use explicit register variables */
        "add %%r12, %[out1]\n\t"
        "add %%r13, %[out2]\n\t"
        /* Memory operand */
        "mov %[mem_in], %%rax\n\t"
        "add %%rax, %[out3]\n\t"
        /* Immediate operand */
        "mov $999, %[out4]\n\t"
        /* Floating point - forces different register class */
        "movsd %[din1], %[dout1]\n\t"
        "addsd %[din2], %[dout1]\n\t"
        /* Clobber many registers */
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4),
          [dout1] "=x" (dout1), [dout2] "=x" (dout2)
        : [in1] "r" (in1), [in2] "r" (in2), 
          [in3] "r" (in3), [in4] "r" (in4),
          [din1] "x" (din1), [din2] "x" (din2),
          [mem_in] "m" (global_int1),
          "r" (reg_var1), "r" (reg_var2), "x" (reg_double)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r14", "r15",
          "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "cc", "memory"
    );
    
    return out1 + out2 + out3 + out4 + (int)dout1 + (int)dout2;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    
    /* Complex addressing with function calls */
    int index = func_return_int(5);
    char* ptr = global_char_array + func_return_int(10);
    
    __asm__ __volatile__ (
        /* Function call results as direct operands */
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        /* Pointer arithmetic result as operand */
        "mov (%[ptr], %[idx], 1), %%bl\n\t"
        "movzx %%bl, %[res2]\n\t"
        /* Mixed type operation */
        "cvtsi2sd %[call3], %%xmm0\n\t"
        "addsd %[dbl_call], %%xmm0\n\t"
        "movsd %%xmm0, %[dres]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2),
          [dres] "=x" (dresult)
        : [call1] "r" (func_return_int(global_int1)),
          [call2] "r" (func_return_int(global_int2)),
          [call3] "r" (func_return_int(global_int3)),
          [dbl_call] "x" (func_return_double(global_double1)),
          [ptr] "r" (ptr), [idx] "r" (index)
        : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "cc", "memory"
    );
    
    return result1 + result2 + (int)dresult;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long long ll1 = 10000000000LL, ll2 = 20000000000LL;
    float f1 = 3.14f, f2 = 2.71f;
    double d1 = 1.618, d2 = 2.718;
    
    char cout;
    short sout;
    int iout;
    long long llout;
    float fout;
    double dout;
    
    /* Assembly that forces mode conversions */
    __asm__ __volatile__ (
        /* Char operations - different mode */
        "mov %[c1], %%al\n\t"
        "add %[c2], %%al\n\t"
        "mov %%al, %[cout]\n\t"
        /* Short operations */
        "mov %[s1], %%ax\n\t"
        "add %[s2], %%ax\n\t"
        "mov %%ax, %[sout]\n\t"
        /* Int operations */
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "mov %%eax, %[iout]\n\t"
        /* Long long - different register pair */
        "mov %[ll1], %%rax\n\t"
        "add %[ll2], %%rax\n\t"
        "mov %%rax, %[llout]\n\t"
        /* Float to double conversion */
        "cvtss2sd %[f1], %%xmm0\n\t"
        "addsd %[d1], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]\n\t"
        /* Double operation */
        "movsd %[d2], %%xmm1\n\t"
        "addsd %%xmm0, %%xmm1\n\t"
        "cvtpd2ps %%xmm1, %[fout]\n\t"
        : [cout] "=r" (cout), [sout] "=r" (sout),
          [iout] "=r" (iout), [llout] "=r" (llout),
          [fout] "=x" (fout), [dout] "=x" (dout)
        : [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [ll1] "r" (ll1), [ll2] "r" (ll2),
          [f1] "x" (f1), [d1] "x" (d1), [d2] "x" (d2)
        : "rax", "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "cc", "memory"
    );
    
    return (int)cout + (int)sout + iout + (int)llout + (int)fout + (int)dout;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Try to force specific register constraints */
    register int must_be_eax asm ("eax") = 123;
    register int must_be_ebx asm ("ebx") = 456;
    
    /* Complex chain of operations requiring intermediate registers */
    __asm__ __volatile__ (
        /* Force value into specific register */
        "mov %[val1], %%eax\n\t"
        /* Use it in computation requiring different register */
        "mov %%eax, %%ecx\n\t"
        "add %[val2], %%ecx\n\t"
        /* Memory barrier */
        "mfence\n\t"
        /* Use result in floating point operation */
        "cvtsi2sd %%ecx, %%xmm0\n\t"
        "movsd %%xmm0, %[temp]\n\t"
        /* Another specific register constraint */
        "mov %[val3], %%ebx\n\t"
        "add %%ecx, %%ebx\n\t"
        "mov %%ebx, %[result]\n\t"
        : [result] "=r" (result), [temp] "=m" (global_double1)
        : [val1] "r" (must_be_eax), [val2] "r" (must_be_ebx),
          [val3] "i" (789)
        : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1",
          "xmm2", "xmm3", "cc", "memory"
    );
    
    /* Another assembly block with interdependent operands */
    int out1, out2;
    __asm__ __volatile__ (
        "mov %[in1], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (result)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[in2], %[out2]\n\t"
        "add $2, %[out2]\n\t"
        : [out2] "=r" (out2)
        : [in2] "r" (out1)
        : "cc"
    );
    
    return result + out1 + out2;
}

/* Test 5: Array indexing with complex expressions */
int test_array_indexing(void) {
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 2;
    }
    
    /* Complex array access patterns */
    for (int i = 0; i < 10; i++) {
        int idx1 = func_return_int(i);
        int idx2 = func_return_int(i + 10);
        int idx3 = func_return_int(i + 20);
        
        int val1, val2, val3;
        
        __asm__ __volatile__ (
            /* Multiple array accesses with computed indices */
            "mov %[arr], %%rax\n\t"
            "mov (%[arr], %[idx1], 4), %[v1]\n\t"
            "mov (%[arr], %[idx2], 4), %[v2]\n\t"
            "add %[v1], %[v2]\n\t"
            "mov (%[arr], %[idx3], 4), %[v3]\n\t"
            "imul %[v2], %[v3]\n\t"
            : [v1] "=r" (val1), [v2] "=r" (val2), [v3] "=r" (val3)
            : [arr] "r" (global_int_array),
              [idx1] "r" (idx1), [idx2] "r" (idx2), [idx3] "r" (idx3)
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
        
        sum += val1 + val2 + val3;
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    memset(global_char_array, 'X', sizeof(global_char_array));
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 3;
    }
    
    /* Run all tests to trigger various reload patterns */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_array_indexing();
    
    /* Final assembly barrier */
    __asm__ __volatile__ ("" ::: "memory");
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256; /* Return deterministic value */
}

/* reload_stress.c - Stress test for GCC reload mechanism */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create register pressure */
int global_int1 = 12345;
int global_int2 = 67890;
int global_int3 = 11111;
int global_int4 = 22222;
double global_double1 = 3.14159;
double global_double2 = 2.71828;
char global_char_array[256];
int global_int_array[100];

/* Function that returns a value - used in nested calls */
int get_value(int x) {
    return x * 2 + 1;
}

/* Another function for complex expressions */
int compute_offset(int a, int b) {
    return (a + b) * sizeof(int);
}

/* Test 1: Many operands with mixed types and constraints */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = 111;
    register int reg_var2 asm ("r13") = 222;
    register double reg_double asm ("xmm0") = 3.14;
    
    /* Complex inline assembly with many operands */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "imul %[reg1], %[out2]\n\t"
        "add %[in3], %[out2]\n\t"
        "mov %[d1], %%xmm1\n\t"
        "addsd %[d2], %%xmm1\n\t"
        "cvttsd2si %%xmm1, %[out3]\n\t"
        "movzbl %[c1], %[out4]\n\t"
        "add %[s1], %[out4]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [reg1] "r" (reg_var1), [d1] "x" (d1), [d2] "x" (d2),
          [c1] "r" ((int)c1), [s1] "r" ((int)s1)
        : "eax", "xmm1", "memory", "cc"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in assembly operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int base = 100;
    
    /* Complex addressing with function calls */
    __asm__ __volatile__ (
        "lea (%[idx1], %[idx2], 4), %%eax\n\t"
        "add %%eax, %[res1]\n\t"
        "mov %[ptr], %%rbx\n\t"
        "mov (%%rbx, %[off], 1), %[res2]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [idx1] "r" (get_value(10)),
          [idx2] "r" (compute_offset(5, 3)),
          [ptr] "r" (global_int_array),
          [off] "r" (get_value(20) % 50)
        : "eax", "rbx", "memory"
    );
    
    /* Another volatile block with memory clobber */
    __asm__ __volatile__ (
        "movl $0, %0\n\t"
        : "=r" (result3)
        :
        : "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c_out;
    short s_out;
    int i_out;
    long l_out;
    double d_out;
    
    int i_in = 255;
    double d_in = 123.456;
    long l_in = 999999999L;
    
    /* Force mode changes through casts */
    __asm__ __volatile__ (
        "mov %[iin], %%al\n\t"
        "movb %%al, %[cout]\n\t"
        "movswl %[iin], %%eax\n\t"
        "movw %%ax, %[sout]\n\t"
        "cvtsi2sd %[iin], %%xmm0\n\t"
        "addsd %[din], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]\n\t"
        : [cout] "=r" (c_out), [sout] "=r" (s_out),
          [dout] "=x" (d_out)
        : [iin] "r" (i_in), [din] "x" (d_in)
        : "eax", "xmm0", "memory"
    );
    
    /* Different mode for same value */
    __asm__ __volatile__ (
        "mov %[lin], %%rax\n\t"
        "shr $32, %%rax\n\t"
        "movl %%eax, %[iout]\n\t"
        : [iout] "=r" (i_out), [lout] "=r" (l_out)
        : [lin] "r" (l_in)
        : "rax", "memory"
    );
    
    return (int)c_out + s_out + i_out + (int)d_out;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    int temp1, temp2, temp3;
    
    /* Try to force specific register constraints */
    __asm__ __volatile__ (
        "mov %[val1], %%eax\n\t"
        "test %%eax, %%eax\n\t"
        "setne %%al\n\t"
        "movzx %%al, %[out1]\n\t"
        : [out1] "=r" (temp1)
        : [val1] "r" (global_int1)
        : "eax", "memory", "cc"
    );
    
    /* Multiple constraints that might need secondary reloads */
    register int acc_var asm ("eax") = 777;
    
    __asm__ __volatile__ (
        "addl $100, %[acc]\n\t"
        "movl %[acc], %[out2]\n\t"
        : [out2] "=rm" (temp2)
        : [acc] "a" (acc_var)
        : "memory", "cc"
    );
    
    /* Memory operand with complex addressing */
    __asm__ __volatile__ (
        "movl $42, %[out3]\n\t"
        : [out3] "=r" (temp3)
        :
        : "memory"
    );
    
    result = temp1 + temp2 + temp3;
    
    /* Chain of assembly blocks */
    __asm__ __volatile__ (
        "addl $1, %0\n\t"
        : "+r" (result)
        :
        : "memory", "cc"
    );
    
    return result;
}

/* Test 5: Maximum register pressure */
int test_max_pressure(void) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4;
    int out1, out2, out3, out4, out5;
    
    /* Very complex assembly with many operands */
    __asm__ __volatile__ (
        "mov %[a1], %%r10d\n\t"
        "add %[a2], %%r10d\n\t"
        "add %[a3], %%r10d\n\t"
        "add %[a4], %%r10d\n\t"
        "mov %%r10d, %[o1]\n\t"
        
        "mov %[a5], %%r11d\n\t"
        "imul %[a6], %%r11d\n\t"
        "add %[a7], %%r11d\n\t"
        "mov %%r11d, %[o2]\n\t"
        
        "mov %[a8], %%r12d\n\t"
        "xor %[a9], %%r12d\n\t"
        "mov %%r12d, %[o3]\n\t"
        
        "mov %[a10], %%r13d\n\t"
        "sub %[a11], %%r13d\n\t"
        "mov %%r13d, %[o4]\n\t"
        
        "mov %[a12], %%r14d\n\t"
        "and %[a13], %%r14d\n\t"
        "or %[a14], %%r14d\n\t"
        "mov %%r14d, %[o5]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3),
          [o4] "=r" (out4), [o5] "=r" (out5)
        : [a1] "r" (v1), [a2] "r" (v2), [a3] "r" (v3), [a4] "r" (v4),
          [a5] "r" (v5), [a6] "r" (v6), [a7] "r" (v7), [a8] "r" (v8),
          [a9] "r" (v9), [a10] "r" (v10), [a11] "r" (v11), [a12] "r" (v12),
          [a13] "r" (v13), [a14] "r" (v14)
        : "r10", "r11", "r12", "r13", "r14", "memory", "cc"
    );
    
    /* Use floating point to force different register class */
    __asm__ __volatile__ (
        "movsd %[d1], %%xmm1\n\t"
        "addsd %[d2], %%xmm1\n\t"
        "mulsd %[d3], %%xmm1\n\t"
        "divsd %[d4], %%xmm1\n\t"
        "cvttsd2si %%xmm1, %%eax\n\t"
        "addl %%eax, %[o1]\n\t"
        : [o1] "+r" (out1)
        : [d1] "x" (d1), [d2] "x" (d2), [d3] "x" (d3), [d4] "x" (d4)
        : "xmm1", "eax", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 10;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_max_pressure();
    
    /* Final assembly barrier */
    __asm__ __volatile__ (
        ""
        :
        :
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum;
}

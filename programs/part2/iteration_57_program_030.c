/* reload_stress.c - Stress test for GCC's reload mechanism */
#include <stdint.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
char global_buffer[256];

/* Function that returns values requiring computation */
int compute_int(int x) {
    return x * 3 + 7;
}

double compute_double(double x) {
    return x * 2.5 - 1.0;
}

int* get_pointer(int index) {
    return &global_array[index];
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5, out6, out7;
    int in0 = compute_int(10);
    int in1 = compute_int(20);
    int in2 = compute_int(30);
    int in3 = compute_int(40);
    int in4 = compute_int(50);
    int in5 = compute_int(60);
    int in6 = compute_int(70);
    int in7 = compute_int(80);
    
    /* Force many reloads with mixed constraints */
    __asm__ __volatile__ (
        "mov %[i0], %[o0]\n\t"
        "add %[i1], %[o1]\n\t"
        "sub %[i2], %[o2]\n\t"
        "and %[i3], %[o3]\n\t"
        "or %[i4], %[o4]\n\t"
        "xor %[i5], %[o5]\n\t"
        "shl %[i6], %[o6]\n\t"
        "shr %[i7], %[o7]"
        : [o0] "=r" (out0), [o1] "=r" (out1), [o2] "=r" (out2),
          [o3] "=r" (out3), [o4] "=r" (out4), [o5] "=r" (out5),
          [o6] "=r" (out6), [o7] "=r" (out7)
        : [i0] "r" (in0), [i1] "r" (in1), [i2] "r" (in2),
          [i3] "r" (in3), [i4] "r" (in4), [i5] "r" (in5),
          [i6] "r" (in6), [i7] "r" (in7),
          "r" (r0), "r" (r1), "r" (r2), "r" (r3)
        : "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5 + out6 + out7;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 1000000L, l2 = 2000000L;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    int out_int;
    double out_double;
    char out_char;
    
    /* Mixed types in same asm - forces mode changes */
    __asm__ __volatile__ (
        "mov %[c1], %%al\n\t"
        "add %[s1], %%ax\n\t"
        "add %[i1], %%eax\n\t"
        "cvtsi2sd %[i2], %%xmm0\n\t"
        "addsd %[d1], %%xmm0\n\t"
        "movsd %%xmm0, %[od]\n\t"
        "mov %%al, %[oc]"
        : [oi] "=r" (out_int), [od] "=m" (out_double), [oc] "=r" (out_char)
        : [c1] "r" ((int)c1), [s1] "r" ((int)s1), [i1] "r" (i1),
          [i2] "r" (i2), [d1] "x" (d1),
          "m" (f1), "m" (f2), "m" (l1), "m" (l2)
        : "rax", "xmm0", "memory", "cc"
    );
    
    /* Force double to int conversion requiring reload */
    int int_from_double;
    __asm__ __volatile__ (
        "cvttsd2si %[dbl], %[out]"
        : [out] "=r" (int_from_double)
        : [dbl] "x" (d2)
        : "memory"
    );
    
    return out_int + (int)out_double + (int)out_char + int_from_double;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    
    /* Function calls as direct operands */
    __asm__ __volatile__ (
        "add %[call1], %[call2]\n\t"
        "mov %%eax, %[res1]"
        : [res1] "=r" (result1)
        : [call1] "r" (compute_int(compute_int(5))),
          [call2] "r" (compute_int(global_int))
        : "rax", "memory", "cc"
    );
    
    /* Complex addressing with function calls */
    int* ptr;
    __asm__ __volatile__ (
        "lea (%[base], %[idx], 4), %[ptr]"
        : [ptr] "=r" (ptr)
        : [base] "r" (get_pointer(10)),
          [idx] "r" (compute_int(3) % 20)
        : "memory"
    );
    
    /* Memory clobber forcing reloads */
    int x = 100, y = 200;
    __asm__ __volatile__ (
        "mov %[x], %%eax\n\t"
        "add %[y], %%eax\n\t"
        "mov %%eax, %[res2]"
        : [res2] "=m" (result2)
        : [x] "r" (x), [y] "r" (y)
        : "rax", "memory", "cc"
    );
    
    /* Another memory barrier */
    __asm__ __volatile__ (""
        :
        :
        : "memory"
    );
    
    /* Use the pointer from earlier asm */
    result3 = *ptr + compute_int(result2);
    
    return result1 + result2 + result3;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int a = 100, b = 200, c = 300;
    int out1, out2;
    
    /* Try to force specific register constraints */
    register int forced_reg asm ("eax") = compute_int(50);
    
    /* Multiple constraints that might need secondary reloads */
    __asm__ __volatile__ (
        "mov %[forced], %%eax\n\t"
        "add %[in1], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "test %[in2], %[in2]\n\t"
        "setz %%al\n\t"
        "movzx %%al, %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [forced] "r" (forced_reg),
          [in1] "rm" (a + compute_int(b)),  /* 'rm' constraint may need reload */
          [in2] "r" (c)
        : "rax", "memory", "cc"
    );
    
    /* Force memory operand with complex addressing */
    int index = compute_int(25);
    int value;
    __asm__ __volatile__ (
        "mov (%[base], %[idx], 4), %[val]"
        : [val] "=r" (value)
        : [base] "r" (global_array),
          [idx] "r" (index)
        : "memory"
    );
    
    return out1 + out2 + value;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int t1, t2, t3, t4, t5;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "mov %[a], %[t1]\n\t"
        "add $1, %[t1]"
        : [t1] "=r" (t1)
        : [a] "r" (v1)
        : "memory", "cc"
    );
    
    /* Chain 2 - depends on Chain 1 result */
    __asm__ __volatile__ (
        "add %[t1], %[b]\n\t"
        "mov %%eax, %[t2]"
        : [t2] "=r" (t2)
        : [b] "r" (v2), [t1] "r" (t1)
        : "rax", "memory", "cc"
    );
    
    /* Chain 3 - mixed types */
    double d = 10.5;
    int int_result;
    __asm__ __volatile__ (
        "cvttsd2si %[dbl], %%eax\n\t"
        "add %[t2], %%eax\n\t"
        "mov %%eax, %[t3]"
        : [t3] "=r" (t3)
        : [dbl] "x" (d), [t2] "r" (t2)
        : "rax", "xmm0", "memory", "cc"
    );
    
    /* Chain 4 - pointer arithmetic */
    char* ptr = global_buffer + compute_int(10);
    int offset = compute_int(5);
    char char_result;
    __asm__ __volatile__ (
        "mov (%[ptr], %[off]), %%al\n\t"
        "mov %%al, %[t4]"
        : [t4] "=r" (char_result)
        : [ptr] "r" (ptr), [off] "r" (offset)
        : "rax", "memory"
    );
    
    /* Chain 5 - final combination */
    __asm__ __volatile__ (
        "add %[x], %[y]\n\t"
        "add %[z], %%eax\n\t"
        "mov %%eax, %[t5]"
        : [t5] "=r" (t5)
        : [x] "r" (t3), [y] "r" ((int)char_result), [z] "r" (v5)
        : "rax", "memory", "cc"
    );
    
    return t1 + t2 + t3 + (int)char_result + t5;
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = compute_int(i);
    }
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = (char)(i % 128);
    }
    
    int checksum = 0;
    
    /* Run all tests multiple times to increase reload pressure */
    for (int i = 0; i < 3; i++) {
        checksum += test_many_operands();
        checksum += test_mixed_types();
        checksum += test_nested_calls();
        checksum += test_secondary_reloads();
        checksum += test_volatile_chains();
        
        /* Modify globals to prevent optimization */
        global_int += checksum % 100;
        global_double += (double)(checksum % 1000) / 1000.0;
    }
    
    /* Final assembly barrier */
    __asm__ __volatile__ (""
        :
        :
        : "memory"
    );
    
    return checksum % 256;  /* Return deterministic result */
}

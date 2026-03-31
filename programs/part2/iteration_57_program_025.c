/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100] = {0};
char global_buffer[256] = {0};

/* Function that returns values - used in nested calls */
int func_return_int(int x) {
    return x * 2 + 1;
}

double func_return_double(double x) {
    return x * 1.5;
}

int* func_return_ptr(int idx) {
    return &global_array[idx];
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int out1, out2, out3, out4, out5, out6, out7, out8;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.1, d2 = 2.2;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Complex inline asm with many operands and mixed types */
    __asm__ __volatile__ (
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "imul %[i3], %%eax\n\t"
        "add %[r1], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "mov %[i4], %%ebx\n\t"
        "sub %[r2], %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        "mov %[s1], %%cx\n\t"
        "add %[s2], %%cx\n\t"
        "movsw %%cx, %[o3]\n\t"
        "mov %[c1], %%dl\n\t"
        "add %[c2], %%dl\n\t"
        "movb %%dl, %[o4]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), 
          [o3] "=r" (out3), [o4] "=r" (out4),
          [o5] "=m" (global_array[0]), [o6] "=m" (global_array[1])
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "i" (in4), [r1] "r" (r1), [r2] "r" (r2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          "m" (global_array[10])
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int out1, out2, out3;
    int* ptr_out;
    
    /* Function calls that must be evaluated into registers */
    __asm__ __volatile__ (
        "mov %[f1], %%eax\n\t"
        "add %[f2], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "lea (%[f3], %[g], 2), %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        "mov %[f4], %%ecx\n\t"
        "mov %%ecx, %[o3]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3),
          [ptr] "=r" (ptr_out)
        : [f1] "r" (func_return_int(10)),
          [f2] "r" (func_return_int(20)),
          [f3] "r" (func_return_int(30)),
          [f4] "r" (func_return_ptr(5)),
          [g] "r" (global_int)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Another asm with memory clobber to force reloads */
    __asm__ __volatile__ (
        ""
        : "=r" (out1), "=r" (out2)
        : "r" (out1), "r" (out2), "m" (*ptr_out)
        : "memory"
    );
    
    return out1 + out2 + out3 + (int)(intptr_t)ptr_out;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int int_out;
    double double_out;
    char char_out;
    short short_out;
    
    int int_in = 255;
    double double_in = 123.456;
    char char_in = 'X';
    short short_in = 4096;
    
    /* Force mode changes by using different types */
    __asm__ __volatile__ (
        "mov %[ii], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"  /* Convert int to double - but SSE disabled! */
        "movd %%xmm0, %[do]\n\t"      /* This should require complex reloads */
        "mov %[ci], %%dl\n\t"
        "movsx %%dl, %%eax\n\t"       /* Sign extend char to int */
        "add %[si], %%ax\n\t"         /* Add short */
        "mov %%eax, %[io]\n\t"
        "mov %%dl, %[co]\n\t"
        "mov %%ax, %[so]\n\t"
        : [io] "=r" (int_out), [do] "=r" (double_out),
          [co] "=r" (char_out), [so] "=r" (short_out)
        : [ii] "r" (int_in), [di] "r" ((int)double_in),  /* Cast double to int for reload */
          [ci] "r" ((int)char_in), [si] "r" ((int)short_in)
        : "eax", "edx", "xmm0", "memory"
    );
    
    /* Use explicit register variables with different types */
    register double dr asm ("r12") = 3.14;
    register int ir asm ("r13") = 100;
    
    __asm__ __volatile__ (
        "mov %[ir], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %[dr], %%xmm0\n\t"
        "movd %%xmm0, %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (int_out)
        : [ir] "r" (ir), [dr] "r" ((int)dr)  /* Cast forces reload */
        : "eax", "xmm0", "memory"
    );
    
    return int_out + (int)double_out + char_out + short_out;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int out1, out2, out3;
    
    /* Try to force accumulator-specific constraints */
    __asm__ __volatile__ (
        "mov %[in1], %%eax\n\t"
        "mul %[in2]\n\t"           /* Requires accumulator */
        "mov %%eax, %[out1]\n\t"
        "mov %%edx, %[out2]\n\t"
        : [out1] "=a" (out1), [out2] "=d" (out2), [out3] "=r" (out3)
        : [in1] "a" (func_return_int(5)),  /* Input in accumulator */
          [in2] "r" (func_return_int(7)),  /* General register input */
          "m" (global_array[20])
        : "cc", "memory"
    );
    
    /* Complex addressing modes */
    int index = global_int;
    __asm__ __volatile__ (
        "mov %[idx], %%ecx\n\t"
        "mov global_array(,%%ecx,4), %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (out3)
        : [idx] "r" (index),
          "m" (global_array)
        : "eax", "ecx", "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 5: Chain of volatile asm blocks */
int test_asm_chains(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int t1, t2, t3, t4;
    
    /* Chain 1 */
    __asm__ __volatile__ (
        "add %2, %1\n\t"
        "mov %1, %0\n\t"
        : "=r" (t1), "+r" (v1)
        : "r" (v2)
        : "cc"
    );
    
    /* Chain 2 - depends on chain 1 */
    __asm__ __volatile__ (
        "imul %2, %1\n\t"
        "add %3, %1\n\t"
        "mov %1, %0\n\t"
        : "=r" (t2), "+r" (v2)
        : "r" (t1), "r" (v3)
        : "cc"
    );
    
    /* Chain 3 - with memory clobber */
    __asm__ __volatile__ (
        "lea (%1, %2, 2), %0\n\t"
        : "=r" (t3)
        : "r" (t2), "r" (v4)
        : "memory"
    );
    
    /* Chain 4 - complex expression */
    __asm__ __volatile__ (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "imul %3, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (t4)
        : "r" (t3), 
          "r" (func_return_int(global_int)),
          "r" (global_array[func_return_int(2)])
        : "eax", "memory"
    );
    
    return t1 + t2 + t3 + t4;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Initialize global array with values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_asm_chains();
    
    /* Final complex asm to ensure all paths are used */
    int final_result;
    __asm__ __volatile__ (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "add %3, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (final_result)
        : "r" (checksum),
          "r" (global_int),
          "r" (func_return_int(checksum))
        : "eax", "memory"
    );
    
    printf("Final checksum: %d\n", final_result);
    
    /* Return deterministic value for testing */
    return final_result % 256;
}

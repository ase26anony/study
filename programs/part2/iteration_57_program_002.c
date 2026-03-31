/* reload_stress_test.c
 * Designed to trigger push_reload initialization block (lines 1381-1399 in reload.cc)
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile char* global_ptr = NULL;
int global_array[100] = {0};

/* Helper functions that return values needing computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(int x) {
    return (double)x / 7.0;
}

int* get_array_ptr(int index) {
    return &global_array[index % 100];
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = global_counter++;
    register int r1 asm ("r13") = global_counter++;
    register int r2 asm ("r14") = global_counter++;
    register int r3 asm ("r15") = global_counter++;
    
    int out0, out1, out2, out3, out4, out5;
    int in0 = compute_value(1);
    int in1 = compute_value(2);
    int in2 = compute_value(3);
    int in3 = compute_value(4);
    int in4 = compute_value(5);
    int in5 = compute_value(6);
    int in6 = compute_value(7);
    int in7 = compute_value(8);
    
    /* Complex assembly with many operands forcing register spills */
    __asm__ __volatile__ (
        "movl %[in0], %[out0]\n\t"
        "addl %[in1], %[out0]\n\t"
        "movl %[in2], %[out1]\n\t"
        "subl %[in3], %[out1]\n\t"
        "imull %[in4], %[out0]\n\t"
        "movl %[in5], %[out2]\n\t"
        "andl %[in6], %[out2]\n\t"
        "orl %[in7], %[out2]\n\t"
        "movl %%r12d, %[out3]\n\t"
        "movl %%r13d, %[out4]\n\t"
        "leal (%[out0], %[out1], 2), %[out5]"
        : [out0] "=&r" (out0), [out1] "=&r" (out1), 
          [out2] "=&r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in0] "r" (in0), [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4), [in5] "r" (in5),
          [in6] "r" (in6), [in7] "r" (in7),
          "r" (r0), "r" (r1)
        : "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 300000L, l2 = 400000L;
    float f1 = 1.5f;
    double d1 = global_double;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed type operations forcing mode conversions */
    __asm__ __volatile__ (
        "movsbl %[c1], %%eax\n\t"
        "movswl %[s1], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[i1], %%eax\n\t"
        "movl %%eax, %[out_int]\n\t"
        "movw %[s2], %[out_short]\n\t"
        "movb %[c2], %[out_char]\n\t"
        /* Force double through integer registers (no SSE) */
        "movq %[d1], %%rax\n\t"
        "movq %%rax, %[out_double]"
        : [out_int] "=r" (out_int), [out_short] "=r" (out_short),
          [out_char] "=r" (out_char), [out_double] "=r" (out_double)
        : [c1] "r" (c1), [s1] "r" (s1), [i1] "r" (i1),
          [c2] "r" (c2), [s2] "r" (s2), [d1] "r" (*(uint64_t*)&d1)
        : "rax", "rbx", "memory"
    );
    
    return out_int + out_short + out_char + (int)out_double;
}

/* Test 3: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int* ptr1, *ptr2;
    
    /* Function calls as operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "movl %%eax, %[res1]\n\t"
        "movq %%rbx, %[ptr1]\n\t"
        "addl $10, %[res1]"
        : [res1] "=r" (result1), [ptr1] "=r" (ptr1)
        : "a" (compute_value(global_counter++)),
          "b" (get_array_ptr(compute_value(global_counter++)))
        : "memory"
    );
    
    /* More complex with memory clobber */
    __asm__ __volatile__ (
        "movl (%%rax), %%ecx\n\t"
        "addl (%%rbx), %%ecx\n\t"
        "movl %%ecx, %[res2]"
        : [res2] "=r" (result2)
        : "a" (get_array_ptr(5)), "b" (get_array_ptr(10))
        : "rcx", "memory"
    );
    
    /* Chain of dependencies */
    __asm__ __volatile__ (
        "leal (%[a], %[b], 4), %[res3]"
        : [res3] "=r" (result3)
        : [a] "r" (result1), [b] "r" (result2)
        : "cc"
    );
    
    return result1 + result2 + result3 + (ptr1 != NULL);
}

/* Test 4: Complex addressing modes */
int test_complex_addressing(void) {
    int array[50];
    for (int i = 0; i < 50; i++) array[i] = i * 2;
    
    int index1 = compute_value(3);
    int index2 = compute_value(7);
    int offset = compute_value(11);
    
    int result1, result2, result3;
    
    /* Complex addressing with non-constant offsets */
    __asm__ __volatile__ (
        "movl (%[arr], %[idx1], 4), %[res1]\n\t"
        "movl 16(%[arr], %[idx2], 4), %[res2]\n\t"
        "leal (%[res1], %[res2], 2), %[res3]"
        : [res1] "=&r" (result1), [res2] "=&r" (result2), [res3] "=r" (result3)
        : [arr] "r" (array), [idx1] "r" (index1), [idx2] "r" (index2)
        : "memory"
    );
    
    /* More addressing with pointer arithmetic */
    int* ptr = array + 20;
    __asm__ __volatile__ (
        "movl (%[ptr], %[off], 4), %[res1]"
        : [res1] "=r" (result1)
        : [ptr] "r" (ptr), [off] "r" (offset)
        : "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 5: Secondary reload triggers */
int test_secondary_reloads(void) {
    register int acc asm ("rax") = global_counter++;
    register int cnt asm ("rcx") = global_counter++;
    
    int out1, out2, out3;
    int in1 = compute_value(100);
    int in2 = compute_value(200);
    
    /* Force specific register constraints that may need secondary reloads */
    __asm__ __volatile__ (
        "movl %%eax, %%ebx\n\t"
        "addl %[in1], %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "movl %%ecx, %[out2]\n\t"
        "cmpl %[in2], %%ebx\n\t"
        "setg %[out3]"
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : "a" (acc), "c" (cnt), [in1] "rm" (in1), [in2] "rm" (in2)
        : "rbx", "cc", "memory"
    );
    
    /* Mix register variables with memory operands */
    double dval = compute_double(in1);
    uint64_t dval_int;
    
    __asm__ __volatile__ (
        "movq %[dval], %%rax\n\t"
        "addq $0x1000, %%rax\n\t"
        "movq %%rax, %[dout]"
        : [dout] "=r" (dval_int)
        : [dval] "m" (*(uint64_t*)&dval)
        : "rax", "memory"
    );
    
    return out1 + out2 + out3 + (int)dval_int;
}

/* Test 6: Volatile sequence with interdependencies */
int test_volatile_chain(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int t1, t2, t3, t4, t5;
    
    /* Chain of volatile asm blocks creating register pressure */
    __asm__ __volatile__ (
        "movl %[a], %[t1]\n\t"
        "addl $10, %[t1]"
        : [t1] "=r" (t1)
        : [a] "r" (v1)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "imull %[t1], %[t2]\n\t"
        "addl %[b], %[t2]"
        : [t2] "=r" (t2)
        : [t1] "r" (t1), [b] "r" (v2)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "leal (%[t1], %[t2], 2), %[t3]\n\t"
        "subl %[c], %[t3]"
        : [t3] "=r" (t3)
        : [t1] "r" (t1), [t2] "r" (t2), [c] "r" (v3)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "xorl %[t2], %[t4]\n\t"
        "orl %[t3], %[t4]"
        : [t4] "=r" (t4)
        : [t2] "r" (t2), [t3] "r" (t3)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "movl %[t4], %[t5]\n\t"
        "shrl $2, %[t5]"
        : [t5] "=r" (t5)
        : [t4] "r" (t4)
        : "cc"
    );
    
    return t1 + t2 + t3 + t4 + t5;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run all tests to trigger various reload scenarios */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_nested_calls();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chain();
    
    /* Final assembly to ensure all values are used */
    __asm__ __volatile__ (
        "addl $1, %0"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte as exit code */
}

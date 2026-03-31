/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
int global_array[100];
double global_doubles[50];
char global_chars[256];
volatile int volatile_counter = 0;

/* Function to force evaluation into registers */
int compute_value(int x) {
    return x * 3 + 7;
}

double compute_double(double x) {
    return x * 2.5 - 1.0;
}

/* Complex addressing helper */
int* get_pointer(int index) {
    return &global_array[index];
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5;
    int in0 = 100, in1 = 200, in2 = 300, in3 = 400;
    double d0 = 1.5, d1 = 2.5;
    char c0 = 'A', c1 = 'B';
    short s0 = 1000, s1 = 2000;
    
    /* Complex inline asm with many operands of different types */
    __asm__ __volatile__ (
        "/* Test 1: Many operands */\n\t"
        "mov %[in0], %[out0]\n\t"
        "add %[in1], %[out0]\n\t"
        "imul %[r0], %[out0]\n\t"
        "mov %[in2], %[out1]\n\t"
        "sub %[r1], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "xor %[r2], %[out2]\n\t"
        "mov %[r3], %[out3]\n\t"
        "lea (%[out0],%[out1],2), %[out4]\n\t"
        "movzx %[c0], %[out5]\n\t"
        : [out0] "=r" (out0), [out1] "=r" (out1),
          [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5)
        : [in0] "r" (in0), [in1] "r" (in1),
          [in2] "r" (in2), [in3] "r" (in3),
          [r0] "r" (r0), [r1] "r" (r1),
          [r2] "r" (r2), [r3] "r" (r3),
          [c0] "r" ((int)c0)
        : "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    int* ptr1, *ptr2;
    
    /* Function calls as operands - forces evaluation before assembly */
    __asm__ __volatile__ (
        "/* Test 2: Nested calls */\n\t"
        "mov %[call1], %[res1]\n\t"
        "add %[call2], %[res1]\n\t"
        "mov %[ptr1], %[res2]\n\t"
        "mov (%[res2]), %[res2]\n\t"
        "lea (%[res1],%[res2],4), %[res3]\n\t"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [res3] "=r" (result3)
        : [call1] "r" (compute_value(volatile_counter)),
          [call2] "r" (compute_value(volatile_counter + 1)),
          [ptr1] "r" (get_pointer(volatile_counter % 50))
        : "memory"
    );
    
    /* Another asm with complex addressing */
    int index = volatile_counter;
    __asm__ __volatile__ (
        "mov %[idx], %%eax\n\t"
        "mov global_array(,%%eax,4), %[res1]\n\t"
        : [res1] "=r" (result1)
        : [idx] "r" (index)
        : "eax", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c = 'X';
    short s = 1234;
    int i = 56789;
    long l = 987654321L;
    float f = 3.14159f;
    double d = 2.71828;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed types in same asm statement */
    __asm__ __volatile__ (
        "/* Test 3: Mixed types */\n\t"
        "movsx %[c], %[out_int]\n\t"      /* char -> int */
        "mov %[s], %w[out_short]\n\t"     /* short */
        "add %[i], %[out_int]\n\t"
        "mov %[out_int], %[out_char]\n\t" /* int -> char truncation */
        : [out_int] "=r" (out_int),
          [out_short] "=r" (out_short),
          [out_char] "=r" (out_char)
        : [c] "r" ((int)c),
          [s] "r" ((int)s),
          [i] "r" (i)
        : "cc"
    );
    
    /* Force double through integer registers (no SSE) */
    uint64_t d_bits;
    __asm__ __volatile__ (
        "movq %[d], %[bits]\n\t"
        "shr $32, %[bits]\n\t"
        : [bits] "=r" (d_bits)
        : [d] "x" (d)
        : "cc"
    );
    
    /* Cast double to int for mode change */
    int d_as_int = (int)d;
    __asm__ __volatile__ (
        "cvtsi2sd %[intval], %[out_double]\n\t"
        : [out_double] "=x" (out_double)
        : [intval] "r" (d_as_int)
    );
    
    return out_int + out_short + out_char + (int)d_bits;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int value1 = 12345;
    int value2 = 67890;
    int result1, result2, result3;
    
    /* Using specific register constraints that may require secondary reloads */
    __asm__ __volatile__ (
        "/* Test 4: Secondary reloads */\n\t"
        "mov %[v1], %%eax\n\t"
        "mov %[v2], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[r1]\n\t"
        "imul %%ebx, %%eax\n\t"
        "mov %%eax, %[r2]\n\t"
        : [r1] "=r" (result1),
          [r2] "=r" (result2),
          "=a" (result3)
        : [v1] "rm" (value1),  /* Allow memory or register */
          [v2] "ri" (value2)   /* Allow register or immediate */
        : "ebx", "cc"
    );
    
    /* Complex memory operand with indexing */
    int index = volatile_counter;
    __asm__ __volatile__ (
        "mov %[idx], %%ecx\n\t"
        "mov global_array(%%ecx,4), %%edx\n\t"
        "add $1, %%edx\n\t"
        "mov %%edx, global_array(%%ecx,4)\n\t"
        : 
        : [idx] "r" (index)
        : "ecx", "edx", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Test 5: Memory clobber and volatile chains */
int test_memory_clobber(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int r1, r2, r3, r4, r5;
    
    /* Chain of volatile asm statements with memory clobbers */
    __asm__ __volatile__ (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        : [r1] "=r" (r1)
        : [a] "r" (a), [b] "r" (b)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[c], %[r2]\n\t"
        "imul %[r1], %[r2]\n\t"
        : [r2] "=r" (r2)
        : [c] "r" (c), [r1] "r" (r1)
        : "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[d], %[r3]\n\t"
        "sub %[r2], %[r3]\n\t"
        "mov %[r3], %[r4]\n\t"
        "neg %[r4]\n\t"
        : [r3] "=r" (r3), [r4] "=r" (r4)
        : [d] "r" (d), [r2] "r" (r2)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[e], %[r5]\n\t"
        "xor %[r4], %[r5]\n\t"
        : [r5] "=r" (r5)
        : [e] "r" (e), [r4] "r" (r4)
        : "cc"
    );
    
    /* Force spill by using all variables */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5)
        : "memory"
    );
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Test 6: Complex addressing with array indexing */
int test_complex_addressing(void) {
    int results[10];
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        int* ptr = &global_array[i * 3];
        int offset = i * 2;
        
        __asm__ __volatile__ (
            "mov %[ptr], %%rax\n\t"
            "mov %[off], %%rbx\n\t"
            "mov (%%rax,%%rbx,4), %[res]\n\t"
            : [res] "=r" (results[i])
            : [ptr] "r" (ptr),
              [off] "r" (offset)
            : "rax", "rbx", "memory"
        );
        
        sum += results[i];
    }
    
    /* Another complex addressing mode */
    int base = 100;
    __asm__ __volatile__ (
        "lea (%[base],%[sum],4), %[sum]\n\t"
        : [sum] "+r" (sum)
        : [base] "i" (base)
        : "cc"
    );
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2 + 1;
    }
    for (int i = 0; i < 50; i++) {
        global_doubles[i] = i * 0.5;
    }
    for (int i = 0; i < 256; i++) {
        global_chars[i] = i;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    volatile_counter++;
    
    checksum += test_nested_calls();
    volatile_counter++;
    
    checksum += test_mixed_types();
    volatile_counter++;
    
    checksum += test_secondary_reloads();
    volatile_counter++;
    
    checksum += test_memory_clobber();
    volatile_counter++;
    
    checksum += test_complex_addressing();
    
    /* Final assembly barrier */
    __asm__ __volatile__ (
        "mfence\n\t"
        : 
        : 
        : "memory"
    );
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}

/* reload_stress.c
 * 
 * This program is designed to stress GCC's reload mechanism by creating
 * inline assembly patterns that force the register allocator to generate
 * many reloads, including secondary reloads. The goal is to trigger the
 * initialization block in push_reload (lines 1381-1399 of reload.cc).
 *
 * Compilation recommendations:
 *   gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -c reload_stress.c
 *   gcc -O2 -funroll-loops -fno-optimize-sibling-calls -m32 -c reload_stress.c
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

/* Another function with side effects */
double compute_double(int a, int b) {
    return (double)(a + b) / 2.0;
}

/* Complex addressing computation */
int* get_pointer(int index) {
    return &global_array[index % 100];
}

/* Test 1: Many operands with mixed constraints to exhaust registers */
int test_many_operands(void) {
    int result = 0;
    
    /* Declare explicit register variables to force specific registers */
    register int r1 asm ("r10") = global_int;
    register int r2 asm ("r11") = global_int + 1;
    register int r3 asm ("r12") = global_int + 2;
    register int r4 asm ("r13") = global_int + 3;
    
    int out1, out2, out3, out4;
    int in1 = get_value(10);
    int in2 = get_value(20);
    int in3 = get_value(30);
    int in4 = get_value(40);
    double d1 = global_double;
    double d2 = global_double * 2.0;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        /* Use explicit register variables */
        "add %%r10, %[out1]\n\t"
        "add %%r11, %[out2]\n\t"
        /* Force memory operands */
        "movl %[mem1], %%eax\n\t"
        "addl %%eax, %[out3]\n\t"
        /* Mixed size operations */
        "movzwl %[short1], %%eax\n\t"
        "addl %%eax, %[out4]\n\t"
        /* Use the double value through integer register */
        "movq %[dbl1], %%rax\n\t"
        "shr $32, %%rax\n\t"
        "addl %%eax, %[out1]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (in1), [in2] "r" (in2), 
          [in3] "r" (in3), [in4] "r" (in4),
          [mem1] "m" (global_array[10]),
          [short1] "m" (*(short*)&global_int),
          [dbl1] "x" (d1),
          "r" (r1), "r" (r2), "r" (r3), "r" (r4)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "r14", "r15", "cc", "memory"
    );
    
    result = out1 + out2 + out3 + out4;
    return result;
}

/* Test 2: Nested function calls in assembly operands */
int test_nested_calls(void) {
    int result = 0;
    int out1, out2;
    
    /* Function calls in input operands force evaluation before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "add %[idx1], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Complex addressing with function call */
        "mov %[ptr], %%rbx\n\t"
        "mov (%%rbx), %%ecx\n\t"
        "mov %%ecx, %[out2]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [call1] "r" (get_value(100)),
          [call2] "r" (get_value(200)),
          [idx1] "r" (global_array[get_value(5) % 100]),
          [ptr] "r" (get_pointer(get_value(3)))
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    result = out1 + out2;
    return result;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    char c1 = global_char;
    short s1 = 1000;
    int i1 = 50000;
    long long ll1 = 1000000000LL;
    float f1 = 2.71828f;
    double d1 = global_double;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Assembly with mixed type operands - forces mode changes */
    __asm__ __volatile__ (
        /* Mix different sized operations */
        "movsbl %[c1], %%eax\n\t"
        "addw %[s1], %%ax\n\t"
        "addl %[i1], %%eax\n\t"
        "movl %%eax, %[out_i]\n\t"
        /* Use float through integer register */
        "movd %[f1], %%eax\n\t"
        "shr $16, %%eax\n\t"
        "movw %%ax, %[out_s]\n\t"
        /* Double to char (extreme mode change) */
        "movq %[d1], %%rax\n\t"
        "movb %%al, %[out_c]\n\t"
        /* Long long operation */
        "mov %[ll1], %%rax\n\t"
        "add $100, %%rax\n\t"
        "cvtsi2sd %%rax, %[out_d]"
        
        : [out_i] "=r" (out_int), [out_s] "=r" (out_short),
          [out_c] "=r" (out_char), [out_d] "=x" (out_double)
        : [c1] "r" (c1), [s1] "r" (s1), [i1] "r" (i1),
          [ll1] "r" (ll1), [f1] "x" (f1), [d1] "x" (d1)
        : "rax", "rcx", "rdx", "memory"
    );
    
    result = out_int + out_short + out_char + (int)out_double;
    return result;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int result = 0;
    int in1 = 100, in2 = 200, in3 = 300;
    int out1, out2;
    
    /* Try to force secondary reloads by using specific constraints */
    __asm__ __volatile__ (
        /* "a" constraint for accumulator */
        "mov %[in2], %%rbx\n\t"
        "add %%rax, %%rbx\n\t"
        "mov %%rbx, %[out1]\n\t"
        /* Complex chain requiring intermediate register */
        "mov %[in3], %%rcx\n\t"
        "imul %%rcx, %%rax\n\t"
        "mov %%rax, %[out2]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), "+a" (in1)
        : [in2] "r" (in2), [in3] "r" (in3)
        : "rbx", "rcx", "rdx", "memory"
    );
    
    /* Another attempt with memory destination */
    __asm__ __volatile__ (
        "mov %[val], %%eax\n\t"
        "mov %%eax, %[mem]"
        
        : [mem] "=m" (global_array[20])
        : [val] "r" (get_value(50)), "a" (in1)
        : "rax", "memory"
    );
    
    result = out1 + out2 + global_array[20];
    return result;
}

/* Test 5: Volatile sequence with interdependent operands */
int test_volatile_chain(void) {
    int result = 0;
    int a = 1, b = 2, c = 3, d = 4;
    int x, y, z;
    
    /* Chain of volatile assembly blocks */
    __asm__ __volatile__ (
        "add %[a], %[b]\n\t"
        "mov %[b], %[x]"
        
        : [x] "=r" (x), [b] "+r" (b)
        : [a] "r" (a)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "imul %[c], %[x]\n\t"
        "mov %[x], %[y]"
        
        : [y] "=r" (y), [x] "+r" (x)
        : [c] "r" (c)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "sub %[d], %[y]\n\t"
        "mov %[y], %[z]"
        
        : [z] "=r" (z), [y] "+r" (y)
        : [d] "r" (d)
        : "cc"
    );
    
    result = x + y + z;
    return result;
}

/* Test 6: Complex addressing modes with non-constant offsets */
int test_complex_addressing(void) {
    int result = 0;
    int index = global_int;
    int offset = get_value(10);
    
    int out1, out2;
    
    /* Array indexing with computed offset */
    __asm__ __volatile__ (
        "mov %[idx], %%eax\n\t"
        "leaq global_array(,%%rax,4), %%rbx\n\t"
        "mov (%%rbx), %%ecx\n\t"
        "mov %%ecx, %[out1]\n\t"
        /* More complex: array[function_call() + global] */
        "mov %[off], %%eax\n\t"
        "add %[idx], %%eax\n\t"
        "cltq\n\t"
        "mov global_array(%%rax,4), %%edx\n\t"
        "mov %%edx, %[out2]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [idx] "r" (index), [off] "r" (offset)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    result = out1 + out2;
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array with values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chain();
    checksum += test_complex_addressing();
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (checksum)
        : "memory"
    );
    
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}

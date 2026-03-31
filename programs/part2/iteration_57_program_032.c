/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -funroll-loops reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile long global_long = 1234567890;
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Function to force computation before assembly */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5 - 2.0;
}

long compute_long(long x, int y) {
    return x + y * 3;
}

/* Test 1: Many operands with mixed types to exhaust registers */
int test_many_operands(void) {
    int result = 0;
    
    /* Declare explicit register variables */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60, g = 70, h = 80;
    short s1 = 100, s2 = 200;
    char ch1 = 'A', ch2 = 'B';
    double d1 = 1.1, d2 = 2.2;
    float f1 = 3.3f, f2 = 4.4f;
    long l1 = 1000, l2 = 2000;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "addl %[r1], %[a]\n\t"
        "subl %[r2], %[b]\n\t"
        "imull %[r3], %[c]\n\t"
        "movw %w[s1], %%ax\n\t"
        "addw %%ax, %w[s2]\n\t"
        "movb %b[ch1], %%al\n\t"
        "addb %%al, %b[ch2]\n\t"
        /* Force memory operations */
        "movl %[e], %[mem1]\n\t"
        "movl %[f], %[mem2]\n\t"
        /* Use computed values */
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c),
          [s2] "+r" (s2), [ch2] "+r" (ch2),
          [mem1] "=m" (global_array[10]),
          [mem2] "=m" (global_array[11])
        /* Many inputs with different constraints */
        : [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3), [r4] "r" (r4),
          [d] "r" (d), [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [s1] "r" (s1), [ch1] "r" (ch1),
          [d1] "r" ((int)d1),  /* Cast double to int to force mode change */
          [f1] "r" ((int)f1),  /* Cast float to int */
          [l1] "r" (l1),
          "i" (5), "i" (10)    /* Immediate constraints */
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    result = a + b + c + s2 + ch2;
    return result;
}

/* Test 2: Nested function calls in assembly operands */
int test_nested_calls(void) {
    int result = 0;
    int x = 100;
    int y = 200;
    double d = 5.5;
    
    /* Complex addressing with function calls */
    __asm__ __volatile__ (
        "movl %[func1], %%eax\n\t"
        "addl %[func2], %%eax\n\t"
        "addl %[idx1], %%eax\n\t"
        "addl %[idx2], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : [func1] "r" (compute_value(x)),
          [func2] "r" (compute_value(y)),
          [idx1] "r" (global_array[compute_value(1)]),  /* Non-constant index */
          [idx2] "r" (global_array[compute_value(2) + global_counter])
        : "rax", "cc", "memory"
    );
    
    /* Another with pointer arithmetic */
    int *ptr = global_array + 20;
    __asm__ __volatile__ (
        "movl (%[ptr], %[offset], 4), %%eax\n\t"
        "addl %%eax, %[result]\n\t"
        : [result] "+r" (result)
        : [ptr] "r" (ptr),
          [offset] "r" (compute_value(3))  /* Dynamic offset */
        : "rax", "cc", "memory"
    );
    
    return result;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    char c = 'X';
    short s = 1000;
    int i = 10000;
    long l = 100000;
    float f = 3.14f;
    double d = 2.71828;
    
    /* Force mode changes through casts */
    __asm__ __volatile__ (
        "movsbl %b[c], %%eax\n\t"      /* Sign extend char to int */
        "movswl %w[s], %%ebx\n\t"      /* Sign extend short to int */
        "addl %%eax, %%ebx\n\t"
        "addl %[i], %%ebx\n\t"
        /* Force double to int conversion */
        "cvttsd2sil %[d], %%eax\n\t"
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (result)
        : [c] "r" (c), [s] "r" (s), [i] "r" (i),
          [d] "r" (d)  /* Double in integer register - forces reload */
        : "rax", "rbx", "cc", "memory"
    );
    
    /* Another with explicit register constraints */
    register double dreg asm ("xmm0") = 1.5;  /* Try to use FP register */
    __asm__ __volatile__ (
        "movq %[dreg], %%rax\n\t"      /* Move from FP to integer reg */
        "addq %%rax, %[l]\n\t"
        : [l] "+r" (l)
        : [dreg] "x" (dreg)  /* 'x' constraint for SSE register */
        : "rax", "cc", "memory"
    );
    
    result += l;
    return result;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    
    /* Try to force moves between register classes */
    register int acc asm ("eax") = 100;
    register int rbx_var asm ("rbx") = 200;
    register int rcx_var asm ("rcx") = 300;
    
    /* Complex constraints that might need secondary reloads */
    __asm__ __volatile__ (
        "xchgl %%eax, %[val1]\n\t"
        "addl %%ebx, %[val2]\n\t"
        "movl %%ecx, %[val3]\n\t"
        : [val1] "+r" (acc),
          [val2] "=r" (rbx_var),
          [val3] "=m" (global_array[20])
        : "b" (rbx_var), "c" (rcx_var)  /* Fixed register constraints */
        : "cc", "memory"
    );
    
    /* Use 'a' constraint (accumulator) with computed value */
    int computed = compute_value(global_counter);
    __asm__ __volatile__ (
        "addl %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : "a" (computed),  /* Must be in eax */
          [counter] "r" (global_counter)
        : "cc"
    );
    
    return result + acc + rbx_var;
}

/* Test 5: Memory clobber and volatile chains */
int test_memory_clobber(void) {
    int result = 0;
    int values[10];
    
    for (int i = 0; i < 10; i++) {
        values[i] = i * 10;
    }
    
    /* Chain of volatile assembly with memory clobber */
    for (int i = 0; i < 5; i++) {
        int idx = compute_value(i) % 10;
        
        __asm__ __volatile__ (
            "movl (%[arr], %[idx], 4), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            "movl %[sum], (%[arr], %[idx], 4)\n\t"
            : [sum] "+r" (result)
            : [arr] "r" (values),
              [idx] "r" (idx)
            : "rax", "memory"  /* Full memory clobber */
        );
        
        /* Another dependent operation */
        __asm__ __volatile__ (
            "incl %[counter]\n\t"
            "addl %[counter], %[sum]\n\t"
            : [sum] "+r" (result),
              [counter] "+m" (global_counter)
            :
            : "cc", "memory"
        );
    }
    
    return result;
}

/* Test 6: Complex addressing with multiple index registers */
int test_complex_addressing(void) {
    int result = 0;
    int *base1 = global_array;
    int *base2 = global_array + 50;
    
    /* Force multiple index registers */
    for (int i = 0; i < 5; i++) {
        int idx1 = compute_value(i);
        int idx2 = compute_value(i + 10);
        
        __asm__ __volatile__ (
            "movl (%[b1], %[i1], 4), %%eax\n\t"
            "movl (%[b2], %[i2], 4), %%ebx\n\t"
            "addl %%eax, %%ebx\n\t"
            "movl %%ebx, %[res]\n\t"
            : [res] "=r" (result)
            : [b1] "r" (base1), [i1] "r" (idx1),
              [b2] "r" (base2), [i2] "r" (idx2)
            : "rax", "rbx", "cc", "memory"
        );
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    for (int i = 0; i < 50; i++) {
        global_darray[i] = i * 0.1;
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_many_operands();
    printf("Test 1 complete: checksum = %d\n", checksum);
    
    checksum += test_nested_calls();
    printf("Test 2 complete: checksum = %d\n", checksum);
    
    checksum += test_mixed_types();
    printf("Test 3 complete: checksum = %d\n", checksum);
    
    checksum += test_secondary_reloads();
    printf("Test 4 complete: checksum = %d\n", checksum);
    
    checksum += test_memory_clobber();
    printf("Test 5 complete: checksum = %d\n", checksum);
    
    checksum += test_complex_addressing();
    printf("Test 6 complete: checksum = %d\n", checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return deterministic value for testing */
    return checksum % 256;
}

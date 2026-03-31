/* reload_stress_test.c
 * 
 * This program is designed to stress GCC's reload pass by creating inline
 * assembly patterns that force the register allocator to generate numerous
 * reloads, including secondary reloads. The goal is to trigger the
 * initialization block in reload.cc's push_reload function (lines 1381-1399).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_buffer[256];
int *global_ptr = &global_int;

/* Function that returns a value, used in nested operands */
int get_value(int x) {
    return x * 2 + 1;
}

/* Another function with side effects */
double compute_double(int a, double b) {
    global_int += a;
    return b * global_int;
}

/* Test 1: Many operands with mixed constraints to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double din1 = 1.5, din2 = 2.5;
    char cin1 = 'A', cin2 = 'B';
    short sin1 = 1000, sin2 = 2000;
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = 999;
    register int reg_var2 asm ("r13") = 888;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Outputs with various constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        "mov %[reg1], %[out3]\n\t"
        "sub %[reg2], %[out3]\n\t"
        /* Force memory operations */
        "mov %[mem1], %%rax\n\t"
        "add %%rax, %[out4]\n\t"
        /* Mix in different sized operations */
        "movzx %[cin1], %%eax\n\t"
        "add %%eax, %[out1]\n\t"
        "movsx %[sin1], %%eax\n\t"
        "add %%eax, %[out2]\n\t"
        /* Clobber many registers */
        : [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4),
          [reg1] "r" (reg_var1), [reg2] "r" (reg_var2),
          [mem1] "m" (global_int),
          [cin1] "r" ((int)cin1), [sin1] "r" ((int)sin1),
          [din1] "r" ((long)din1)  /* Cast double to long to force mode change */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r14", "r15", "cc", "memory"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls within assembly operands */
int test_nested_calls(void) {
    int result1, result2;
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Complex addressing with function calls */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "add %[idx1], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "mov %[idx2], %%ebx\n\t"
        "imul %[global], %%ebx\n\t"
        "mov %%ebx, %[res2]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [call1] "r" (get_value(10)),
          [call2] "r" (get_value(20)),
          [idx1] "r" (array[get_value(1) % 10]),  /* Non-constant index */
          [idx2] "r" (array[global_int % 10]),    /* Global-dependent index */
          [global] "m" (global_int)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result1 + result2;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'X', c2 = 'Y';
    short s1 = 1234, s2 = 5678;
    int i1 = 10000, i2 = 20000;
    long l1 = 3000000000L, l2 = 4000000000L;
    float f1 = 1.234f, f2 = 5.678f;
    double d1 = 9.876, d2 = 5.432;
    
    int out_int;
    double out_double;
    
    /* Force mode changes by using different types in same assembly */
    __asm__ __volatile__ (
        /* Integer operations */
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "movsx %[s1], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "movzx %[c1], %%ecx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %[out_i]\n\t"
        /* Floating point via integer registers (with -mno-sse) */
        "mov %[d1_low], %%rax\n\t"
        "mov %[d1_high], %%rdx\n\t"
        "add %[d2_low], %%rax\n\t"
        "adc %[d2_high], %%rdx\n\t"
        "mov %%rax, %[out_d_low]\n\t"
        "mov %%rdx, %[out_d_high]\n\t"
        : [out_i] "=r" (out_int),
          [out_d_low] "=r" (((long long*)&out_double)[0]),
          [out_d_high] "=r" (((long long*)&out_double)[1])
        : [i1] "r" (i1), [i2] "r" (i2),
          [s1] "r" ((int)s1), [c1] "r" ((int)c1),
          [d1_low] "r" (((long long*)&d1)[0]),
          [d1_high] "r" (((long long*)&d1)[1]),
          [d2_low] "r" (((long long*)&d2)[0]),
          [d2_high] "r" (((long long*)&d2)[1]),
          [f1] "r" ((int)f1)  /* Cast float to int to force mode change */
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return out_int + (int)out_double;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int result = 0;
    int value1 = 111, value2 = 222;
    double dvalue = 33.33;
    
    /* Try to force moves between register classes */
    __asm__ __volatile__ (
        /* Force use of specific registers */
        "mov %[val1], %%eax\n\t"
        "mov %[val2], %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        /* Complex memory addressing that might need secondary reload */
        "mov %[mem], %%rcx\n\t"
        "add (%%rcx), %%eax\n\t"
        "mov %%eax, %[res]\n\t"
        : [res] "=a" (result)  /* Constrain output to eax */
        : [val1] "r" (value1),
          [val2] "r" (value2),
          [mem] "r" (&global_int)
        : "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Another attempt with flag register constraints */
    int flag_test;
    __asm__ __volatile__ (
        "test %[v1], %[v1]\n\t"
        "setg %%al\n\t"
        "movzx %%al, %[out]\n\t"
        : [out] "=r" (flag_test)
        : [v1] "r" (value1)
        : "rax", "cc"
    );
    
    return result + flag_test;
}

/* Test 5: Volatile assembly chains with interdependent operands */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x, y, z;
    
    /* Chain 1: a -> b -> c */
    __asm__ __volatile__ (
        "mov %[a], %[x]\n\t"
        "add %[b], %[x]\n\t"
        : [x] "=r" (x)
        : [a] "r" (a), [b] "r" (b)
        : "cc"
    );
    
    /* Chain 2: Uses result from chain 1 */
    __asm__ __volatile__ (
        "mov %[x], %[y]\n\t"
        "imul %[c], %[y]\n\t"
        "add %[d], %[y]\n\t"
        : [y] "=r" (y)
        : [x] "r" (x), [c] "r" (c), [d] "r" (d)
        : "cc"
    );
    
    /* Chain 3: Uses results from both previous chains */
    __asm__ __volatile__ (
        "mov %[x], %%eax\n\t"
        "add %[y], %%eax\n\t"
        "add %[e], %%eax\n\t"
        "mov %%eax, %[z]\n\t"
        : [z] "=r" (z)
        : [x] "r" (x), [y] "r" (y), [e] "r" (e)
        : "rax", "cc"
    );
    
    /* Memory barrier between chains */
    __asm__ __volatile__ ("" ::: "memory");
    
    return x + y + z;
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize global buffer */
    strcpy(global_buffer, "Test string for memory operations");
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests and accumulate results */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    
    /* One more complex test inline in main */
    {
        int final_val;
        register int r1 asm ("r10") = 1000;
        register int r2 asm ("r11") = 2000;
        
        __asm__ __volatile__ (
            "lea (%[r1], %[r2], 2), %[out]\n\t"
            "add %[glob], %[out]\n\t"
            "add %[call], %[out]\n\t"
            : [out] "=r" (final_val)
            : [r1] "r" (r1), [r2] "r" (r2),
              [glob] "m" (global_int),
              [call] "r" (get_value(50))
            : "cc", "memory"
        );
        
        checksum += final_val;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return deterministic value for verification */
    return checksum % 256;
}

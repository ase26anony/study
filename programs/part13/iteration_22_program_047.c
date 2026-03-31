/* reload_test.c - Test program to trigger specific reload.cc code paths */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Complex functions to force temporary values */
int compute_index(void) {
    static int idx = 0;
    return (idx++ * 37) & 0xFF;
}

int complex_expr(int a, int b) {
    return (a * b) + (a >> 3) - (b << 2);
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + g_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early-clobber */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    register int r1 asm("ebx") = 100;
    asm volatile (
        "addl %%ebx, %0\n\t"
        : "+r"(output1)
        : 
        : "ebx"
    );
    
    g_checksum += output1 + output2 + output3 + r1;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    int idx = compute_index();
    
    /* Force reload for builtin argument */
    int popcnt = __builtin_popcount(array[idx] + g_counter);
    
    /* Complex address computation */
    int atomic_val = __atomic_fetch_add(&array[complex_expr(idx, 3)], 1, __ATOMIC_RELAXED);
    
    /* Math builtin with complex argument */
    double x = 2.0 + (g_counter * 0.01);
    double root = __builtin_sqrt(x * x + 1.0);
    
    g_checksum += popcnt + atomic_val + (int)root;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("esi") = 1000;
    register int r2 asm("edi") = 2000;
    int result;
    
    /* Use in conflicting context */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)
        : "eax"
    );
    
    /* Force address-taking (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
    );
    
    g_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159 + g_counter;
    double d2 = 2.71828;
    double d3;
    
    /* Memory constraints that may need secondary reloads */
    asm volatile (
        "movsd %1, %0\n\t"
        : "=m"(d3)
        : "x"(d1)
        : 
    );
    
    /* Mixed size operands */
    long long ll1 = 0x123456789ABCDEF0LL + g_counter;
    int i1;
    
    asm volatile (
        "movq %1, %%rax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(i1)
        : "m"(ll1)
        : "rax"
    );
    
    g_checksum += (int)d3 + i1;
}

/* Test 5: Complex addressing modes */
void test_complex_addressing(void) {
    struct {
        int data[100];
        int extra;
    } s = {0};
    
    int idx = compute_index();
    int result;
    
    /* Complex address computation that may need reload */
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result)
        : "r"(&s.data), "r"(idx)
        : 
    );
    
    /* Force base register constraint if available */
#ifdef __i386__
    asm volatile (
        "movl %%ebx, %0\n\t"
        : "=b"(result)
        : 
        : 
    );
#endif
    
    g_checksum += result;
}

/* Test 6: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = g_counter + 1;
    int v2 = g_counter + 2;
    int v3 = g_counter + 3;
    int v4 = g_counter + 4;
    int v5 = g_counter + 5;
    int v6 = g_counter + 6;
    int v7 = g_counter + 7;
    int v8 = g_counter + 8;
    
    /* Chain of operations forcing spills and reloads */
    v1 = complex_expr(v1, v2);
    v2 = complex_expr(v2, v3);
    v3 = complex_expr(v3, v4);
    v4 = complex_expr(v4, v5);
    v5 = complex_expr(v5, v6);
    v6 = complex_expr(v6, v7);
    v7 = complex_expr(v7, v8);
    v8 = complex_expr(v8, v1);
    
    /* Use in inline asm to force specific constraints */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(v1), "+r"(v2)
        : 
        : 
    );
    
    g_checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < 1000; i++) {
        g_counter = i;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_complex_addressing();
        test_high_register_pressure();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}

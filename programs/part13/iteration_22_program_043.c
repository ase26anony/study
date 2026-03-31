/* Test program to trigger reload.cc's push_reload function */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function to create complex expressions */
int compute_index(int base) {
    return (base * 3 + 7) & 0xF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(int iterations) {
    int i;
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    for (i = 0; i < iterations; i++) {
        /* Force reload by requiring specific register for output 
           but providing complex input expression */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output1)          /* Must be in eax */
            : "mr"(input1 + i * 2)   /* Memory or register, complex expr */
            : 
        );
        
        /* Early-clobber constraint forces reload */
        asm volatile (
            "addl %2, %0\n\t"
            "movl %0, %1\n\t"
            : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
            : "r"(input2), "0"(output1)      /* Input tied to output2 */
            : 
        );
        
        /* Mix different sized operands */
        short s_input = 100;
        long long ll_output;
        asm volatile (
            "movswl %1, %%eax\n\t"
            "cltq\n\t"
            "movq %%rax, %0\n\t"
            : "=r"(ll_output)
            : "r"(s_input)
            : "%rax"
        );
        
        global_result ^= output1 ^ output2 ^ output3 ^ (int)ll_output;
    }
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(int iterations) {
    unsigned int arr[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Complex addressing in builtin operand */
        int idx = compute_index(i);
        int cnt = __builtin_popcount(arr[idx] + i);
        
        /* Builtin with function call in operand */
        int ctz = __builtin_ctz(compute_index(i) | 1);
        
        /* Multiple complex operands */
        int parity = __builtin_parity(arr[compute_index(i + 1)] ^ i);
        
        global_result += cnt + ctz + parity;
    }
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(int iterations) {
    int i;
    
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 100;
    r2 = 200;
    
    for (i = 0; i < iterations; i++) {
        int temp;
        
        /* Force conflict: use register variable in asm requiring different reg */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=r"(temp)
            : "r"(r1)        /* In ebx, but asm may need to move to another reg */
            : "%eax"
        );
        
        /* Take address (sort of) by using in memory context */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m"(global_counter)  /* Memory destination */
            : "r"(r2)               /* Register source - may need reload */
            : 
        );
        
        r1 += temp;
        r2 ^= i;
        global_result += r1 + r2;
    }
}

/* Test 4: Atomic operations with complex addresses */
void test_atomic_complex_address(int iterations) {
    struct {
        int data[8];
        int counter;
    } s = {{0}};
    
    int i;
    for (i = 0; i < iterations; i++) {
        /* Complex address computation for atomic operation */
        int idx = i & 7;
        
        /* This may require reloads for address computation */
        __atomic_fetch_add(&s.data[idx], i, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s.counter, 1, __ATOMIC_RELAXED);
        
        global_result += s.data[idx];
    }
}

/* Test 5: Floating point constraints that may need reloads */
void test_float_reloads(int iterations) {
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    double d1 = 1.234567, d2 = 9.876543;
    int i;
    
    for (i = 0; i < iterations; i++) {
        float f_result;
        double d_result;
        
        /* Mix float and int in constraints */
        asm volatile (
            "cvtsi2ssl %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=x"(f_result)        /* Must be in XMM register */
            : "r"(i), "x"(f1)       /* Integer i in reg, f1 in XMM */
            : "%xmm0"
        );
        
        /* Double precision with memory constraint */
        asm volatile (
            "addsd %1, %0\n\t"
            : "+x"(d_result)        /* Read-write in XMM register */
            : "m"(d2)               /* Memory operand */
            : 
        );
        
        /* Convert between float and double */
        d_result = (double)f_result + d1;
        
        global_result += (int)f_result + (int)d_result;
    }
}

/* Test 6: Secondary reload triggers (architecture specific) */
void test_secondary_reload_triggers(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Try to trigger secondary reloads through various means */
        
        /* Memory barrier that might need special handling */
        asm volatile ("" ::: "memory");
        
        /* Control register access (x86 specific - may need secondary reload) */
        unsigned long cr0;
        asm volatile (
            "mov %%cr0, %0\n\t"
            : "=r"(cr0)
            : 
            : 
        );
        
        /* String operations with explicit registers */
        char src[16] = "test";
        char dst[16];
        asm volatile (
            "movl $4, %%ecx\n\t"
            "leal %1, %%esi\n\t"
            "leal %0, %%edi\n\t"
            "rep movsb\n\t"
            : 
            : "r"(dst), "r"(src)
            : "%ecx", "%esi", "%edi", "memory"
        );
        
        global_result += cr0 & 0xFF;
        global_result += dst[0];
    }
}

/* Test 7: Multiple output constraints */
void test_multiple_outputs(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int out1, out2, out3;
        int in1 = i * 3;
        int in2 = i * 5 + 1;
        
        /* Multiple outputs with different constraints */
        asm volatile (
            "imull %2, %0\n\t"
            "addl %3, %1\n\t"
            "xorl %0, %1\n\t"
            : "=&a"(out1), "=r"(out2)  /* out1 must be eax, early clobber */
            : "r"(in1), "r"(in2)
            : 
        );
        
        /* Output in memory, input in register */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m"(global_counter)    /* Memory output */
            : "r"(out1)               /* Register input */
            : 
        );
        
        global_result += out1 + out2;
    }
}

/* Test 8: Volatile asm with many operands */
void test_volatile_many_operands(int iterations) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int r1, r2, r3, r4;
        
        /* Many operands to increase register pressure */
        asm volatile (
            "movl %4, %0\n\t"
            "addl %5, %1\n\t"
            "subl %6, %2\n\t"
            "imull %7, %3\n\t"
            : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4)
            : "r"(a + i), "r"(b + i), "r"(c + i), "r"(d + i)
            : 
        );
        
        /* Use results to prevent elimination */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(global_result)
            : "r"(r1 + r2 + r3 + r4)
            : 
        );
        
        a = b; b = c; c = d; d = e; e = a + i;
    }
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    test_asm_constraint_conflict(iterations);
    printf("Test 1 complete, result: %d\n", global_result);
    
    test_builtin_complex_operand(iterations);
    printf("Test 2 complete, result: %d\n", global_result);
    
    test_register_variable_abuse(iterations);
    printf("Test 3 complete, result: %d\n", global_result);
    
    test_atomic_complex_address(iterations);
    printf("Test 4 complete, result: %d\n", global_result);
    
    test_float_reloads(iterations);
    printf("Test 5 complete, result: %d\n", global_result);
    
    test_secondary_reload_triggers(iterations);
    printf("Test 6 complete, result: %d\n", global_result);
    
    test_multiple_outputs(iterations);
    printf("Test 7 complete, result: %d\n", global_result);
    
    test_volatile_many_operands(iterations);
    printf("Test 8 complete, result: %d\n", global_result);
    
    printf("All tests complete. Final result: %d\n", global_result);
    
    return global_result != 0 ? 0 : 1;
}

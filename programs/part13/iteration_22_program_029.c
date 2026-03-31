/* test_reload.c - Comprehensive test to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to force register pressure */
int compute_index(int i) {
    return (i * 37 + 123) % 256;
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
            : "mr"(input1 + i * 7)   /* Memory or register, complex expr */
            : 
        );
        
        /* Early-clobber constraint forces reload */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "=&r"(output2)         /* Early clobber */
            : "r"(input1), "r"(input2 + i)
            : 
        );
        
        /* Mixed register classes */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(output3)
            : "q"(input2)            /* Must be in a, b, c, or d register */
            : 
        );
        
        global_checksum += output1 + output2 + output3;
    }
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(int iterations) {
    int i;
    unsigned int values[256];
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        values[i] = i * 3 + 1;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Complex addressing in builtin operand */
        int cnt = __builtin_popcount(values[compute_index(i)]);
        
        /* Builtin with function call as argument */
        int trailing = __builtin_ctz(compute_index(i) + 1);
        
        /* Multiple complex operands */
        int parity = __builtin_parity(values[i % 256] + cnt);
        
        global_checksum += cnt + trailing + parity;
    }
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(int iterations) {
    int i;
    
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    for (i = 0; i < iterations; i++) {
        int temp;
        
        /* Force conflict: use register variable in asm requiring different reg */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(temp)
            : "r"(r1), "r"(r2 + i)   /* r1 is tied to ebx, but we use generic constraint */
            : "%eax"
        );
        
        /* Take address indirectly (GCC extension with warning) */
        int *ptr = &r3;
        *ptr += temp;
        
        /* Use in memory context */
        asm volatile (
            "addl %1, %0\n\t"
            : "+m"(*ptr)             /* Memory constraint on register variable */
            : "r"(temp)
            : 
        );
        
        global_checksum += r1 + r2 + r3;
    }
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(int iterations) {
    int i;
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d3, d4;
    
    for (i = 0; i < iterations; i++) {
        /* Memory constraints that may require secondary reloads */
        asm volatile (
            "movsd %1, %0\n\t"
            : "=m"(d3)               /* Memory output */
            : "x"(d1 + i)            /* SSE register input */
            : 
        );
        
        /* Complex memory addressing */
        struct {
            double arr[4];
        } s = {{1.1, 2.2, 3.3, 4.4}};
        
        asm volatile (
            "movsd %1, %0\n\t"
            : "=x"(d4)               /* SSE register output */
            : "m"(s.arr[i % 4])      /* Complex memory address */
            : 
        );
        
        /* Atomic operations with complex addresses */
        int atomic_var = 0;
        __atomic_fetch_add(&atomic_var, compute_index(i), __ATOMIC_SEQ_CST);
        
        global_checksum += (int)d3 + (int)d4 + atomic_var;
    }
}

/* Test 5: Mixed modes and sizes */
void test_mixed_modes(int iterations) {
    int i;
    char c = 'A';
    short s = 1000;
    int n = 100000;
    long long ll = 10000000000LL;
    
    for (i = 0; i < iterations; i++) {
        int result1, result2;
        long long result3;
        
        /* Mixed size operands */
        asm volatile (
            "movsbl %1, %0\n\t"
            : "=r"(result1)
            : "r"(c + i)             /* Char extended to int */
            : 
        );
        
        /* Different modes in same asm */
        asm volatile (
            "movzwl %1, %0\n\t"
            "addl %2, %0\n\t"
            : "=r"(result2)
            : "r"(s), "r"(n)         /* Short and int */
            : 
        );
        
        /* 64-bit on 32-bit architecture would force reloads */
        asm volatile (
            "addq %1, %0\n\t"
            : "+r"(ll)
            : "r"(100LL)
            : 
        );
        
        global_checksum += result1 + result2 + (int)ll;
    }
}

/* Test 6: High register pressure */
void test_high_register_pressure(int iterations) {
    int i;
    /* Many live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    for (i = 0; i < iterations; i++) {
        /* Many operations forcing spills and reloads */
        v1 = v2 + v3;
        v2 = v3 * v4;
        v3 = v4 - v5;
        v4 = v5 / (v6 + 1);
        v5 = v6 ^ v7;
        v6 = v7 | v8;
        v7 = v8 & v9;
        v8 = v9 << 2;
        v9 = v10 >> 1;
        v10 = v1 + i;
        
        /* Inline asm using many variables */
        asm volatile (
            "imull %1, %0\n\t"
            "addl %2, %0\n\t"
            "subl %3, %0\n\t"
            "orl %4, %0\n\t"
            : "+r"(v1)
            : "r"(v2), "r"(v3), "r"(v4), "r"(v5)
            : 
        );
        
        global_checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    printf("Starting reload stress test...\n");
    
    /* Run all tests multiple times */
    for (int run = 0; run < 10; run++) {
        test_asm_constraint_conflict(iterations);
        test_builtin_complex_operand(iterations);
        test_register_variable_abuse(iterations);
        test_secondary_reload_trigger(iterations);
        test_mixed_modes(iterations);
        test_high_register_pressure(iterations);
        
        global_counter++;
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed. Check reload.cc.gcov coverage.\n");
    
    return 0;
}

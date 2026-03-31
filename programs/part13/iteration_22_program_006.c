/* Test program to exercise GCC's reload pass push_reload function */
/* Compile with: gcc -O1 -fno-omit-frame-pointer test_reload.c -o test_reload */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Complex function to force register pressure */
int complex_function(int a, int b, int c) {
    return (a * b) + (c << 2) - (a / (b + 1));
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output1)      /* Output in any register */
        : "mr" (input1 + global_counter),  /* Memory or register with complex expr */
          "mr" (input2)
        : "%eax"              /* eax is clobbered */
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $100, %0"
        : "=&r" (output2)     /* Early-clobber: can't share reg with inputs */
        : "r" (output1),
          "m" (input2)        /* Memory constraint */
        : "cc"
    );
    
    /* Mixed register classes */
    asm volatile (
        "imull %1, %0"
        : "+r" (output3)
        : "rm" (complex_function(input1, input2, global_counter))
        : "cc"
    );
    
    global_result += output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int values[100];
    for (int i = 0; i < 100; i++) {
        values[i] = i * 3 + global_counter;
    }
    
    /* Complex addressing in builtin */
    int popcnt = __builtin_popcount(values[complex_function(1, 2, 3) % 100]);
    
    /* Builtin with function call as argument */
    int ctz = __builtin_ctz(complex_function(popcnt, 2, 3) | 1);
    
    /* Atomic operation with complex address */
    int atomic_var = 42;
    __atomic_fetch_add(&atomic_var, 
                      complex_function(ctz, popcnt, global_counter),
                      __ATOMIC_SEQ_CST);
    
    global_result += popcnt + ctz + atomic_var;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    r3 = 3000 + global_counter;
    
    int result;
    
    /* Force conflict: output requires eax but input is in ebx */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl $777, %%eax"
        : "=a" (result)       /* Must be in eax */
        : "b" (r1)            /* Must be in ebx */
        : "cc"
    );
    
    /* Use register variable in memory context */
    asm volatile (
        "addl %%ecx, %0"
        : "+m" (global_result)
        : "c" (r2)            /* Must be in ecx */
        : "cc"
    );
    
    /* Multiple register constraints in one asm */
    asm volatile (
        "imull %%edx, %%eax"
        : "+a" (result)
        : "d" (r3)
        : "cc"
    );
    
    global_result += result;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159 + global_counter;
    double d2 = 2.71828 + global_counter;
    double d3;
    
    /* Floating point operations often need secondary reloads */
    asm volatile (
        "addsd %1, %0"
        : "+x" (d3)           /* xmm register */
        : "xm" (d1 * d2)      /* Complex expression */
        : 
    );
    
    /* Memory constraint with offset */
    struct {
        double arr[10];
        int padding;
    } s;
    
    for (int i = 0; i < 10; i++) {
        s.arr[i] = i * 1.5;
    }
    
    asm volatile (
        "movsd %1, %0"
        : "=x" (d3)
        : "m" (s.arr[complex_function(1, 2, 3) % 10])
        : 
    );
    
    /* Integer to float conversion may need reloads */
    int int_val = complex_function(100, 200, 300);
    double d4;
    
    asm volatile (
        "cvtsi2sd %1, %0"
        : "=x" (d4)
        : "rm" (int_val)
        : 
    );
    
    global_result += (int)(d3 + d4);
}

/* Test 5: Mixed size operands and addressing modes */
void test_mixed_operands(void) {
    char c1 = 'A' + (global_counter % 26);
    short s1 = 32000 + global_counter;
    int i1 = 1000000 + global_counter;
    long long ll1 = 5000000000LL + global_counter;
    
    /* Mixed sizes in same asm */
    int result1;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result1)
        : "m" (c1),           /* char */
          "m" (s1),           /* short */
          "rm" (i1)           /* int */
        : "%eax", "cc"
    );
    
    /* 64-bit operations on 32-bit arch may need special handling */
    long long result2;
    asm volatile (
        "addq %1, %0"
        : "+r" (ll1)
        : "rm" ((long long)complex_function(1, 2, 3))
        : "cc"
    );
    
    /* Complex addressing mode */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 7 + global_counter;
    }
    
    int result3;
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r" (result3)
        : "r" (array),        /* Base */
          "r" (complex_function(10, 20, 30) % 25)  /* Index */
        : "memory"
    );
    
    global_result += result1 + (int)ll1 + result3;
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = complex_function(1, 2, 3);
    int v2 = complex_function(4, 5, 6);
    int v3 = complex_function(7, 8, 9);
    int v4 = complex_function(10, 11, 12);
    int v5 = complex_function(13, 14, 15);
    int v6 = complex_function(16, 17, 18);
    int v7 = complex_function(19, 20, 21);
    int v8 = complex_function(22, 23, 24);
    
    /* Chain of operations forcing temporary values */
    int r1 = v1 * v2 + v3;
    int r2 = v4 / (v5 + 1) - v6;
    int r3 = (v7 << 3) | (v8 >> 2);
    int r4 = r1 ^ r2 ^ r3;
    
    /* Use all in inline asm to prevent optimization */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        "subl %3, %0\n\t"
        "orl  %4, %0"
        : "+r" (r4)
        : "r" (r1), "r" (r2), "r" (r3), "rm" (global_counter)
        : "cc"
    );
    
    global_result += r4;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < 1000; i++) {
        global_counter = i;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_high_register_pressure();
        
        /* Prevent loop unrolling */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return 0;
}

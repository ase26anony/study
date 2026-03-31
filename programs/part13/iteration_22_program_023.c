/* test_reload_coverage.c - Comprehensive test for GCC reload pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_result = 0;
volatile int g_checksum = 0;

/* Global variables to force memory operands */
int g_array[100] = {0};
float g_float_array[50] = {0.0f};
double g_double_array[25] = {0.0};

/* Function to compute index with side effects */
int compute_index(void) {
    static int counter = 0;
    return (counter++ % 10) * 2;
}

/* Complex expression function */
int complex_expr(int a, int b) {
    return (a * b) + (a >> 3) - (b & 0xFF);
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(int iterations) {
    int i;
    int input, output;
    
    for (i = 0; i < iterations; i++) {
        input = i * 3 + 7;
        
        /* Force reload by requiring specific register for output
           while input is a complex expression */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output)          /* Must be in eax */
            : "mr"(input + g_array[i % 10] + 5)  /* Complex memory/register operand */
            : 
        );
        
        /* Another conflict: output in r, input requires memory */
        int input2 = complex_expr(i, output);
        int output2;
        asm volatile (
            "addl %1, %0\n\t"
            : "=r"(output2)         /* Any general register */
            : "m"(input2), "0"(i)   /* input2 must be memory, i in same reg as output */
            : "cc"
        );
        
        /* Early-clobber constraint forcing reload */
        int a = i + 1;
        int b = i + 2;
        int c = i + 3;
        asm volatile (
            "imull %2, %1\n\t"
            "addl %1, %0\n\t"
            : "=&r"(output), "=&r"(a)  /* Early clobber - can't share with inputs */
            : "r"(b), "0"(c), "1"(a)
            : "cc"
        );
        
        g_checksum += output + output2 + a;
    }
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* __builtin_popcount with complex argument */
        int popcnt = __builtin_popcount(
            g_array[compute_index()] + i * 255
        );
        
        /* __builtin_ctz with function call in argument */
        int ctz = __builtin_ctz(
            complex_expr(i, popcnt) | 1  /* Ensure non-zero */
        );
        
        /* Math built-in with memory access */
        double d = __builtin_sqrt(
            g_double_array[i % 10] + (double)i * 0.5
        );
        
        /* Atomic built-in with complex address */
        int atomic_val = __atomic_fetch_add(
            &g_array[(i * 7) % 20],
            complex_expr(i, popcnt),
            __ATOMIC_SEQ_CST
        );
        
        g_checksum += popcnt + ctz + (int)d + atomic_val;
    }
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(int iterations) {
    int i;
    
    /* Declare register variables for specific registers */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    for (i = 0; i < iterations; i++) {
        r1 = i * 2 + 1;
        r2 = i * 3 + 2;
        r3 = i * 5 + 3;
        
        /* Force conflict: use register variable in asm requiring different register */
        int result;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(result)
            : "r"(r1), "r"(r2)  /* r1/r2 in specific regs, but asm uses eax */
            : "eax", "cc"
        );
        
        /* Try to take address (will force to memory, causing constraint conflicts) */
        int *ptr;
        /* GCC allows this with warning - perfect for forcing reloads */
        ptr = (int*)&r3;
        *ptr += result;
        
        /* Use in memory constraint */
        int temp;
        asm volatile (
            "movl (%1), %0\n\t"
            : "=r"(temp)
            : "r"(ptr)  /* ptr in register, but constraint needs memory */
            : "memory"
        );
        
        g_checksum += r1 + r2 + r3 + result + temp;
    }
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Different operand sizes causing mode changes */
        char c = i & 0xFF;
        short s = i * 2;
        long long ll = (long long)i * 1000;
        
        /* Mixed size operands in asm */
        long long result;
        asm volatile (
            "movsx %%al, %%eax\n\t"
            "movsx %%si, %%edx\n\t"
            "addl %%edx, %%eax\n\t"
            "cltd\n\t"
            "addl %%eax, %A0\n\t"
            "adcl %%edx, %B0\n\t"
            : "+r"(ll)
            : "a"(c), "S"(s)  /* c in al, s in si */
            : "edx", "cc"
        );
        result = ll;
        
        /* Memory operand with offset that might need secondary reload */
        int index = compute_index();
        int mem_result;
        asm volatile (
            "movl %c1(%2), %0\n\t"
            : "=r"(mem_result)
            : "i"(index * sizeof(int)), "r"(g_array)
            : "memory"
        );
        
        /* Floating point to integer move requiring secondary reload on some archs */
        float f = g_float_array[i % 10] + i;
        int int_from_float;
        asm volatile (
            "movd %1, %0\n\t"
            : "=r"(int_from_float)
            : "x"(f)  /* xmm register */
            : 
        );
        
        g_checksum += (int)result + mem_result + int_from_float;
    }
}

/* Test 5: Multiple output operands with conflicting constraints */
void test_multiple_outputs(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int out1, out2, out3;
        int in1 = i + 100;
        int in2 = i + 200;
        int in3 = i + 300;
        
        /* Three outputs with different register classes */
        asm volatile (
            "movl %3, %0\n\t"
            "movl %4, %1\n\t"
            "addl %5, %1\n\t"
            "movl %1, %2\n\t"
            : "=a"(out1), "=r"(out2), "=b"(out3)  /* Different specific registers */
            : "r"(in1), "r"(in2), "r"(in3)
            : "cc"
        );
        
        /* Output tied to input but with early clobber */
        int a = in1, b = in2;
        asm volatile (
            "leal (%1, %2), %0\n\t"
            : "=&r"(out1)  /* Early clobber */
            : "r"(a), "r"(b)
            : 
        );
        
        g_checksum += out1 + out2 + out3;
    }
}

/* Test 6: Volatile asm with many operands to increase register pressure */
void test_high_register_pressure(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Many live variables to force spilling and reloading */
        int v1 = i * 1;
        int v2 = i * 2;
        int v3 = i * 3;
        int v4 = i * 4;
        int v5 = i * 5;
        int v6 = i * 6;
        int v7 = i * 7;
        int v8 = i * 8;
        int v9 = i * 9;
        int v10 = i * 10;
        
        /* Series of asm statements using different subsets of variables */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            : "+r"(v1)
            : "r"(v2), "r"(v3)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            : "+r"(v4)
            : "r"(v5)
            : "cc"
        );
        
        asm volatile (
            "orl %1, %0\n\t"
            "andl %2, %0\n\t"
            : "+r"(v6)
            : "r"(v7), "r"(v8)
            : "cc"
        );
        
        asm volatile (
            "xchgl %0, %1\n\t"
            : "+r"(v9), "+r"(v10)
            : 
        );
        
        /* Use all variables to prevent dead code elimination */
        g_checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
}

/* Main function orchestrating all tests */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    printf("Starting reload coverage tests...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i * 3;
        if (i < 50) g_float_array[i] = i * 1.5f;
        if (i < 25) g_double_array[i] = i * 2.5;
    }
    
    /* Run all tests multiple times */
    for (int run = 0; run < 10; run++) {
        test_asm_constraint_conflict(iterations);
        test_builtin_complex_operand(iterations);
        test_register_variable_abuse(iterations);
        test_secondary_reload_trigger(iterations);
        test_multiple_outputs(iterations);
        test_high_register_pressure(iterations);
        
        printf("Run %d complete, checksum = %d\n", run + 1, g_checksum);
    }
    
    g_volatile_result = g_checksum;
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed successfully.\n");
    
    return g_checksum != 0 ? 0 : 1;
}

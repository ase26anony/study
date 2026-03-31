/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization in reload.cc lines 1381-1399
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_trigger
 * For debugging: gcc -O2 -fomit-frame-pointer -fdump-rtl-reload -march=x86-64 reload_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t helper_func(int64_t a, int64_t b, int64_t c, int64_t d, 
                          int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to prevent optimization */
    return (a * b) + (c * d) - (e * f) + (g * h) - (a + b + c + d + e + f + g + h);
}

/* Function to create maximum register pressure */
__attribute__((noinline))
static int64_t create_register_pressure(void) {
    /* Declare many local variables to exhaust registers */
    volatile int64_t v1 = 1;   /* volatile prevents optimization */
    int64_t v2 = 2;
    int64_t v3 = 3;
    int64_t v4 = 4;
    int64_t v5 = 5;
    int64_t v6 = 6;
    int64_t v7 = 7;
    int64_t v8 = 8;
    int64_t v9 = 9;
    int64_t v10 = 10;
    int64_t v11 = 11;
    int64_t v12 = 12;
    int64_t v13 = 13;
    int64_t v14 = 14;
    int64_t v15 = 15;
    int64_t v16 = 16;
    int64_t v17 = 17;
    int64_t v18 = 18;
    int64_t v19 = 19;
    int64_t v20 = 20;
    
    /* Use explicit register variables to pin values */
    register int64_t r12_var asm ("r12") = 100;
    register int64_t r13_var asm ("r13") = 200;
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Move values using specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v1)          /* Output in register */
        : [in1] "r" (v2),          /* Input in register */
          [in2] "r" (v3)           /* Another input in register */
        : "rax", "rbx", "rcx", "rdx"  /* Clobber specific registers */
    );
    
    /* Another inline asm with mismatched constraints */
    {
        int32_t small_int = 42;
        int64_t large_int = 1000;
        
        /* Mixed mode operation forcing conversion */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (large_int)
            : "r" (small_int), "r" (large_int)
            : "rax", "cc"
        );
        v4 = large_int;
    }
    
    /* Complex expression using most variables - creates register pressure */
    int64_t result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 + v9 * v10 + 
        v11 - v12 * v13 + v14 / (v15 + 1) +
        v16 * v17 - v18 + v19 * v20 +
        r12_var - r13_var;
    
    /* More complex operations with mixed types */
    {
        /* Force mode mismatches */
        char c1 = 'A';
        short s1 = 1000;
        int i1 = 100000;
        double d1 = 3.14159;
        float f1 = 2.71828f;
        
        /* Operations requiring conversions */
        i1 = i1 + c1;      /* char to int promotion */
        i1 = i1 + s1;      /* short to int promotion */
        v1 = v1 + i1;      /* int to int64_t */
        
        /* Floating point operations that might need different registers */
        d1 = d1 + f1;      /* float to double promotion */
        result += (int64_t)d1;
    }
    
    /* Complex array indexing - forces address reloads */
    {
        int64_t arr[100];
        for (int i = 0; i < 20; i++) {
            /* Complex index expression requiring multiple registers */
            arr[v1 + v2 * i - v3] = v4 + v5 * i;
        }
        
        /* Nested struct with address taken */
        struct inner {
            int64_t a;
            int64_t b;
            int64_t c[5];
        };
        
        struct outer {
            struct inner in;
            int64_t x;
        } outer_var;
        
        /* Taking address of nested member */
        int64_t *ptr = &outer_var.in.c[2];
        *ptr = result;
        result += *ptr;
    }
    
    /* Call helper function with many arguments - forces register parameter passing */
    result += helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
    result += helper_func(v9, v10, v11, v12, v13, v14, v15, v16);
    
    /* Use volatile in condition to prevent dead code elimination */
    volatile int check = (result > 1000);
    if (check) {
        /* More operations if condition is true */
        result = result * 2 - v17 + v18 / (v19 + 1);
    }
    
    /* Final complex expression using all variables */
    result = 
        ((v1 * v2) + (v3 * v4) - (v5 * v6) + (v7 * v8) - 
         (v9 * v10) + (v11 * v12) - (v13 * v14) + (v15 * v16) -
         (v17 * v18) + (v19 * v20) + r12_var - r13_var + result);
    
    return result;
}

/* Another function with different patterns */
__attribute__((noinline))
static int64_t create_more_reloads(void) {
    /* Create register pressure with different variable types */
    int8_t b1 = 1, b2 = 2, b3 = 3;
    int16_t s1 = 100, s2 = 200, s3 = 300;
    int32_t i1 = 1000, i2 = 2000, i3 = 3000;
    int64_t l1 = 10000, l2 = 20000, l3 = 30000;
    
    /* Mixed type operations forcing conversions */
    l1 = l1 + i1 + s1 + b1;
    l2 = l2 + i2 + s2 + b2;
    l3 = l3 + i3 + s3 + b3;
    
    /* Inline asm with memory constraints */
    asm volatile (
        "movq (%1), %%rax\n\t"
        "addq (%2), %%rax\n\t"
        "movq %%rax, (%0)\n\t"
        : 
        : "r" (&l1), "r" (&l2), "r" (&l3)
        : "rax", "memory"
    );
    
    return l1 + l2 + l3;
}

int main(void) {
    int64_t result1 = create_register_pressure();
    int64_t result2 = create_more_reloads();
    
    /* Use results to prevent optimization */
    printf("Result 1: %ld\n", (long)result1);
    printf("Result 2: %ld\n", (long)result2);
    
    /* Final complex expression in main too */
    volatile int64_t final_result = result1 * result2 - result1 + result2;
    
    return (final_result > 0) ? 0 : 1;
}

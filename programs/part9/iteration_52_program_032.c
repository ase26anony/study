/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization code
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d,
                            int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to create register pressure */
    return a + b * c - d / (e + 1) + f * g - h;
}

/* Another helper to force more register usage */
__attribute__((noinline))
static void modify_vars(int64_t *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + i;
    }
}

int main(void) {
    /* Create register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile int64_t seed = 42;
    
    /* Declare many local variables to exhaust registers */
    register int64_t v1 asm ("r12") = seed + 1;
    int64_t v2 = seed * 2;
    int64_t v3 = seed / 3;
    int64_t v4 = seed - 4;
    int64_t v5 = seed + 5;
    int64_t v6 = seed * 6;
    int64_t v7 = seed / 7;
    int64_t v8 = seed - 8;
    int64_t v9 = seed + 9;
    int64_t v10 = seed * 10;
    int64_t v11 = seed / 11;
    int64_t v12 = seed - 12;
    int64_t v13 = seed + 13;
    int64_t v14 = seed * 14;
    int64_t v15 = seed / 15;
    int64_t v16 = seed - 16;
    int64_t v17 = seed + 17;
    int64_t v18 = seed * 18;
    int64_t v19 = seed / 19;
    int64_t v20 = seed - 20;
    
    /* Force mode mismatches */
    int32_t small1 = 1000;
    int64_t large1 = 5000;
    float f1 = 3.14f;
    double d1 = 2.71828;
    
    /* Complex array indexing to force address reloads */
    int64_t arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and clobbers */
    asm volatile (
        /* Move values between registers, clobbering specific ones */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v1)      /* Output in register */
        : [input1] "r" (v2),      /* Input in register */
          [input2] "r" (v3)       /* Another input in register */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobber specific registers */
    );
    
    /* More inline assembly with different constraints */
    int64_t result;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %%rcx, %[b]\n\t"
        : [b] "+r" (v4), [result] "=r" (result)
        : [a] "r" (v5)
        : "rcx", "cc"
    );
    
    /* Complex expression using most variables - creates register pressure */
    /* Mix different types to force mode conversions */
    v6 = ((v7 * v8) / (v9 + 1)) + (v10 - v11) * (v12 + v13);
    v14 = v15 + (int64_t)f1 * v16 - (int64_t)d1;
    v17 = (v18 << 3) | (v19 & 0xFF);
    
    /* Use volatile variable in condition to prevent elimination */
    if (seed > 0) {
        /* Even more complex computation */
        v20 = v1 + v2 * v3 - v4 / (v5 + 1) + v6 * v7 - v8 +
              v9 + v10 * v11 - v12 / (v13 + 1) + v14 * v15 - v16 +
              v17 + v18 * v19 - v20;
    }
    
    /* Force address computation reload with complex indexing */
    int64_t sum = 0;
    for (int i = v1 % 10; i < v2 % 20; i += v3 % 3 + 1) {
        sum += arr[i + (v4 % 5) * (v5 % 7)];
    }
    
    /* Call function that uses System V ABI register passing */
    int64_t func_result = use_registers(v1, v2, v3, v4, v5, v6, v7, v8);
    
    /* Modify array through function call */
    modify_vars(arr, 50);
    
    /* Final complex expression using all variables */
    int64_t final_result = 
        v1 + v2 - v3 * v4 / (v5 + 1) +
        v6 + v7 - v8 * v9 / (v10 + 1) +
        v11 + v12 - v13 * v14 / (v15 + 1) +
        v16 + v17 - v18 * v19 / (v20 + 1) +
        func_result + sum + (int64_t)(f1 * d1) + small1 + large1;
    
    /* Use result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}

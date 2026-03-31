/* reload_trigger.c
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_test
 * For debugging: gcc -O2 -fomit-frame-pointer -fdump-rtl-reload -march=x86-64 reload_trigger.c 2>&1 | grep -A5 -B5 "push_reload"
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static int64_t use_registers(int64_t a, int64_t b, int64_t c, int64_t d,
                             int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Complex expression to create register pressure */
    return a + b * c - d / (e + 1) + (f << 2) - (g >> 3) + h * h;
}

/* Another helper to force more register usage */
__attribute__((noinline))
static void clobber_helper(int64_t *arr, int n) {
    volatile int64_t sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    /* Prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Create massive register pressure with many live variables */
    register int64_t r1 asm("r12") = 1;
    register int64_t r2 asm("r13") = 2;
    int64_t v1 = rand() % 100;
    volatile int64_t v2 = rand() % 100;  /* volatile prevents optimization */
    int64_t v3 = rand() % 100;
    int64_t v4 = rand() % 100;
    int64_t v5 = rand() % 100;
    int64_t v6 = rand() % 100;
    int64_t v7 = rand() % 100;
    int64_t v8 = rand() % 100;
    int64_t v9 = rand() % 100;
    int64_t v10 = rand() % 100;
    int64_t v11 = rand() % 100;
    int64_t v12 = rand() % 100;
    int64_t v13 = rand() % 100;
    int64_t v14 = rand() % 100;
    int64_t v15 = rand() % 100;
    int64_t v16 = rand() % 100;
    int64_t v17 = rand() % 100;
    int64_t v18 = rand() % 100;
    int64_t v19 = rand() % 100;
    int64_t v20 = rand() % 100;
    
    /* Complex addressing mode - forces address reloads */
    int64_t arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Move values using specific registers */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v1)        /* Output operand */
        : [input1] "r" (v2),        /* Input operand 1 */
          [input2] "r" (v3),        /* Input operand 2 */
          "0" (v1)                  /* Matching constraint */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobber specific registers */
    );
    
    /* Another asm with mismatched modes */
    {
        int32_t small = 42;
        int64_t large;
        /* Force mode conversion reload */
        asm volatile (
            "movslq %1, %0\n\t"
            : "=r" (large)
            : "r" (small)
            : "cc"
        );
        v4 = large;
    }
    
    /* Complex expression using most variables - maximizes live ranges */
    int64_t result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        (v6 << (v7 % 8)) - (v8 >> (v9 % 8)) + 
        v10 * v11 - v12 * v13 + 
        v14 / (v15 | 1) + v16 % (v17 | 1) + 
        v18 ^ v19 ^ v20;
    
    /* Mix float and double to force mode conversions */
    {
        float f = v1 * 0.5f;
        double d = v2 * 0.5;
        /* Force conversion between floating point modes */
        d = d + (double)f;
        result += (int64_t)d;
    }
    
    /* Use pinned register variables in computation */
    result += r1 * r2;
    
    /* Complex array indexing - forces address computation reloads */
    int idx = (v3 + v4 * v5 - v6) % 50;
    result += arr[idx + v7 * 2 - v8 / 4];
    
    /* Force function call with many parameters - uses calling convention registers */
    result += use_registers(v9, v10, v11, v12, v13, v14, v15, v16);
    
    /* More register pressure with another complex expression */
    volatile int64_t temp = 0;
    for (int i = 0; i < 10; i++) {
        temp += arr[(v17 + i) % 100] * arr[(v18 - i) % 100];
    }
    
    /* Call helper that uses array - forces more register saves/restores */
    clobber_helper(arr, 20);
    
    /* Final conditional using volatile to prevent elimination */
    if (temp > 1000) {
        result += temp;
    }
    
    printf("Result: %ld\n", result);
    return (int)(result % 256);
}

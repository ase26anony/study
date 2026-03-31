/* reload_trigger.c - Forces GCC to generate reloads for register allocation */

/* Prevent inlining to force register-based parameter passing */
#define NOINLINE __attribute__((noinline))

/* Helper function that uses many registers */
NOINLINE long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long prevent_opt = 0;
    long result = a + b * c - d / (e + 1) + f;
    
    /* Inline asm with fixed register constraints */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (result), "r" (a)
        : "%rax", "%rbx", "%rcx", "%rdx", "memory"
    );
    
    return result + prevent_opt;
}

/* Another helper with mismatched types */
NOINLINE double mixed_mode_helper(int a, double b, long c, float d) {
    /* Mixing types forces mode conversions */
    double result = (double)a + b * (double)c + (double)d;
    
    /* Complex addressing mode */
    volatile double array[100];
    int idx = a % 100;
    result += array[idx + (int)(b * 10)];  /* Forces address reload */
    
    return result;
}

int main() {
    /* Create massive register pressure with many live variables */
    register long r1 asm ("r12") = 1;   /* Pin to specific register */
    register long r2 asm ("r13") = 2;
    
    volatile int v1 = 1;   /* volatile prevents optimization */
    int v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14;
    int v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    float f1 = 4.4f, f2 = 5.5f;
    
    /* Complex expression using most variables - creates long live ranges */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 - v8 * v9 + v10;
    v2 = v11 + v12 - v13 * v14 / v15 + v16 - v17 + v18 * v19 - v20;
    
    /* Mix 32-bit and 64-bit operations */
    l1 = (long)v1 * l2 + (long)v2 * l3 - l4 / (l5 + 1);
    
    /* Inline assembly with fixed register constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movq %3, %%rbx\n\t"
        "imulq %4, %%rbx\n\t"
        : "=r" (v3), "=r" (v4)
        : "r" (v5), "r" (l1), "r" (l2), "0" (v3), "1" (v4)
        : "%rax", "%rbx", "%rcx", "%rdx", "memory", "cc"
    );
    
    /* More register pressure */
    d1 = (double)v1 + d2 * (double)v2 - d3 / (double)(v3 + 1);
    
    /* Call helper with many parameters - forces register allocation for args */
    long helper_result = helper_func(l1, l2, l3, l4, l5, (long)d1);
    
    /* Mixed mode function call */
    double mixed_result = mixed_mode_helper(v1, d1, l1, f1);
    
    /* Complex array access with multiple index variables */
    volatile int arr[1000];
    int idx1 = v1 % 1000;
    int idx2 = v2 % 100;
    int idx3 = v3 % 10;
    
    /* This complex addressing may require reloads */
    int arr_val = arr[idx1 + idx2 * idx3 + v4 * v5 - v6];
    
    /* Use all variables in final computation to prevent dead code elimination */
    volatile long final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        l1 + l2 + l3 + l4 + l5 +
        (long)d1 + (long)d2 + (long)d3 +
        (long)f1 + (long)f2 +
        helper_result + (long)mixed_result + arr_val + r1 + r2;
    
    /* Prevent compiler from optimizing everything away */
    if (final_result > 0) {
        return (int)(final_result % 1000);
    }
    
    return 0;
}

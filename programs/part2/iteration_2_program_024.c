/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_helper(int x) {
    volatile static int counter = 0;
    counter += x;
    return counter;
}

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    char v6 = seed & 0xFF;
    short v7 = seed & 0xFFFF;
    int *v8 = &v2;
    float *v9 = &v4;
    double v10 = seed * 10.0;
    int v11 = seed + 11;
    long long v12 = seed * 12LL;
    float v13 = seed * 13.0f;
    double v14 = seed * 14.0;
    int v15 = seed + 15;
    int v16 = seed + 16;
    int v17 = seed + 17;
    int v18 = seed + 18;
    int v19 = seed + 19;
    int v20 = seed + 20;
    
    /* Key intermediate result with mixed-type computation */
    int key_result = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7;
    
    /* Complex conditional structure with deeply nested blocks */
    if (v1 > 0) {
        if (v2 % 3 == 0) {
            switch (v3 % 5) {
                case 0: {
                    /* Deeply nested conditional use of key_result */
                    volatile int cond = v4 > 0.0f;
                    if (cond) {
                        /* Use key_result in inline assembly with many clobbers */
                        int temp = key_result;
                        asm volatile (
                            "add %[val], %[val], #1\n\t"
                            : [val] "+r" (temp)
                            : 
                            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                              "memory", "cc"
                        );
                        key_result = temp;
                        
                        /* Call non-inlineable function */
                        key_result = external_helper(key_result);
                    }
                    break;
                }
                case 1: {
                    /* Another conditional path with different mode usage */
                    double dbl_key = (double)key_result;
                    asm volatile (
                        "fadd d0, d0, d0\n\t"
                        : 
                        : 
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
                          "memory"
                    );
                    key_result = (int)(dbl_key * 2.0);
                    break;
                }
                default: {
                    /* Use short mode */
                    short short_key = (short)key_result;
                    asm volatile (
                        "add w0, w0, w0\n\t"
                        : 
                        : 
                        : "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7",
                          "w8", "w9", "w10", "w11", "w12", "w13", "w14", "w15",
                          "memory"
                    );
                    key_result = (int)short_key;
                    break;
                }
            }
        } else if (v2 % 7 == 0) {
            /* More conditional complexity */
            volatile int flag = v5 > 100.0;
            if (flag) {
                /* Use long long mode */
                long long ll_key = (long long)key_result * v3;
                asm volatile (
                    "add x0, x0, x0\n\t"
                    : 
                    : 
                    : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                      "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                      "memory"
                );
                key_result = (int)(ll_key >> 32);
            }
        }
    }
    
    /* Label addresses for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile_jump_target = labels[seed % 4];
    
    /* Use key_result before potential jump */
    key_result += v6 * v7;
    
    /* Opaque control flow jump */
    if (v8 != NULL && *v8 > 100) {
        goto *volatile_jump_target;
    }
    
    /* Continue normal execution */
    key_result *= 2;
    
label1:
    key_result += v8 ? *v8 : 0;
    goto join_point;
    
label2:
    key_result -= v9 ? (int)*v9 : 0;
    goto join_point;
    
label3:
    key_result |= 0xFF;
    goto join_point;
    
label4:
    key_result &= 0xFFFF;
    /* fall through */
    
join_point:
    /* Use key_result again after conditional blocks */
    for (int i = 0; i < 5; i++) {
        key_result += v10 + v11 + v12 + v13 + v14;
        key_result = external_helper(key_result);
    }
    
    /* Final computation using all variables to keep them live */
    int final_result = key_result 
        + v15 + v16 + v17 + v18 + v19 + v20
        + (int)v3 + (int)v4 + (int)v5
        + v6 + v7 + (v8 ? *v8 : 0)
        + (v9 ? (int)*v9 : 0);
    
    return final_result;
}

/* Additional variant for 32-bit compilation */
__attribute__((noinline, optimize("Os")))
int stress_function_32bit(int seed) {
    /* Different mix of types and modes */
    volatile char c1 = seed;
    volatile short s1 = seed * 2;
    volatile int i1 = seed * 3;
    volatile long long ll1 = seed * 4LL;
    volatile float f1 = seed * 5.0f;
    volatile double d1 = seed * 6.0;
    
    /* Complex computation with mode mixing */
    int result = c1 + s1 + i1 + (int)ll1 + (int)f1 + (int)d1;
    
    /* Nested conditionals with volatile controls */
    volatile int cond1 = c1 > 0;
    volatile int cond2 = s1 < 1000;
    
    if (cond1 && cond2) {
        /* Use different modes in conditional block */
        double temp = (double)result * d1;
        asm volatile (
            "fld1\n\t"
            "faddp %%st(1), %%st\n\t"
            : 
            : 
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
              "memory", "cc"
        );
        result = (int)temp;
    } else if (cond1 || cond2) {
        /* Use long long mode */
        long long lltemp = (long long)result * ll1;
        asm volatile (
            "movl %%eax, %%eax\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "memory", "cc"
        );
        result = (int)(lltemp >> 16);
    }
    
    /* Use result after conditional */
    for (volatile int i = 0; i < 3; i++) {
        result = external_helper(result);
    }
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call stress functions multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        if (i % 10 == 0) {
            total += stress_function_32bit(i);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

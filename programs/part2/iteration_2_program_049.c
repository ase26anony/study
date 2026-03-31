/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func3(double x) { return x / 2.0; }

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    float v3 = seed * 1.1f;
    double v4 = seed * 2.2;
    long long v5 = seed * 3LL;
    short v6 = seed & 0xFFFF;
    char v7 = seed & 0xFF;
    int v8 = seed ^ 0x12345678;
    float v9 = v3 * 2.0f;
    double v10 = v4 / 3.0;
    int v11 = 0, v12 = 0, v13 = 0, v14 = 0, v15 = 0;
    int v16 = 0, v17 = 0, v18 = 0, v19 = 0, v20 = 0;
    
    /* Key intermediate result - this is what we want to rematerialize */
    int key_result = v1 + v2 + (int)v3 + (int)v4 + v7;
    
    /* Complex conditional computation with deeply nested blocks */
    if (v1 > 100) {
        v11 = external_func1(v1);
        if (v2 % 3 == 0) {
            v12 = v11 * v2;
            v3 = external_func2(v3);
            
            /* Nested switch inside if */
            switch (v7 % 4) {
                case 0:
                    v13 = key_result * 2;  /* Use key_result conditionally */
                    /* Inline assembly with many clobbered registers */
                    asm volatile (
                        "mov %0, %0\n\t"
                        :
                        : "r" (v13)
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    break;
                case 1:
                    v14 = key_result + v11;
                    /* More register pressure */
                    asm volatile (
                        "add %0, %1, %2\n\t"
                        : "=r" (v14)
                        : "r" (key_result), "r" (v11)
                        : "r0", "r1", "r2", "r3", "memory"
                    );
                    break;
                case 2:
                    v15 = key_result - v12;
                    v4 = external_func3(v4);
                    break;
                default:
                    v16 = key_result ^ v11;
                    break;
            }
            
            /* Another level of nesting */
            if (v3 > 0.0f) {
                v17 = key_result / (v2 + 1);
                /* Mixed type computation */
                v5 = (long long)key_result * v5;
            }
        } else if (v2 % 5 == 0) {
            v18 = key_result | v1;
            /* Different mode usage - char/short */
            v6 = (short)(key_result & 0xFFFF);
            v7 = (char)(key_result & 0xFF);
        }
    } else {
        v19 = external_func1(v1 * 2);
        if (v4 < 100.0) {
            v20 = key_result & v19;
            /* Use key_result with float conversion */
            v9 = (float)key_result + v9;
        }
    }
    
    /* Use goto with computed labels for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile_jump_target = labels[v1 % 4];
    
    if (v1 % 3 == 0) {
        goto *volatile_jump_target;
    }
    
label1:
    /* Use key_result again after conditional blocks */
    int final_result = key_result + v11 + v12 + v13 + v14 + v15;
    final_result += v16 + v17 + v18 + v19 + v20;
    
    /* More mixed-type computations to generate different RTL modes */
    float float_sum = v3 + v9 + (float)key_result;
    double double_sum = v4 + v10 + (double)key_result;
    long long ll_sum = v5 + (long long)key_result;
    
    /* Force all values to be used */
    asm volatile ("" : : "r" (float_sum), "r" (double_sum), "r" (ll_sum));
    
    return final_result + (int)float_sum + (int)double_sum + (int)ll_sum;

label2:
    /* Alternative path with different register usage */
    key_result = key_result * 3;
    goto label1;

label3:
    /* Another path */
    key_result = key_result - external_func1(key_result);
    goto label1;

label4:
    /* Yet another path */
    key_result = key_result ^ 0xAAAAAAAA;
    goto label1;
}

/* Additional function to create more register pressure in caller */
__attribute__((noinline))
int caller_with_pressure(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables across the loop */
        int a = i * 2;
        float b = i * 3.14f;
        double c = i * 6.28;
        long long d = i * 100LL;
        int e = i ^ 0x55AA55AA;
        int f = i & 0x0F0F0F0F;
        int g = i | 0x00FF00FF;
        int h = i + 0x11223344;
        
        /* Call stress function with all these live variables */
        sum += stress_function(i + a + e);
        
        /* Keep all variables live across the call */
        asm volatile ("" : : "r" (a), "r" (b), "r" (c), "r" (d), 
                      "r" (e), "r" (f), "r" (g), "r" (h));
    }
    
    return sum;
}

int main() {
    int result = 0;
    
    /* Create varying inputs to explore different paths */
    for (int i = 0; i < 100; i++) {
        result += caller_with_pressure(10);
        
        /* Alternate between different seed patterns */
        if (i % 2 == 0) {
            result += stress_function(i * 100);
        } else {
            result += stress_function(i * 50 + 12345);
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}

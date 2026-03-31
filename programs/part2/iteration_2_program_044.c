/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func3(double x) { return x / 3.14159; }

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    short v3 = (short)(seed + 2);
    char v4 = (char)(seed + 3);
    long long v5 = (long long)seed * 1000;
    float v6 = (float)seed * 0.5f;
    double v7 = (double)seed * 2.71828;
    int *v8 = &v2;
    float *v9 = &v6;
    double *v10 = &v7;
    
    int i, j, k;
    int result = 0;
    
    /* Complex intermediate computation - this will be our target register */
    int intermediate = v1 * v2 + v3 - v4;
    intermediate = external_func1(intermediate);
    
    /* Deeply nested conditional blocks */
    if (v1 > 100) {
        if (v2 < 200) {
            switch (v3 & 0x3) {
                case 0:
                    /* Use intermediate in inline asm with many clobbers */
                    asm volatile (
                        "add %[val], %[val], #1\n\t"
                        "mov r0, %[val]\n\t"
                        "mov r1, %[val]\n\t"
                        "mov r2, %[val]\n\t"
                        : [val] "+r" (intermediate)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    break;
                case 1:
                    intermediate += v5 & 0xFF;
                    break;
                case 2:
                    intermediate -= (int)v6;
                    break;
                default:
                    intermediate ^= 0x12345678;
            }
            
            /* More register pressure */
            float temp_f = v6 * 2.0f;
            double temp_d = v7 / 2.0;
            v6 = external_func2(temp_f);
            v7 = external_func3(temp_d);
            
            /* Use goto with computed label for irreducible CFG */
            void *labels[] = { &&label1, &&label2, &&label3 };
            volatile_jump_target = labels[v4 & 0x1];
            
            if (v4 & 0x2) {
                goto *volatile_jump_target;
            }
        }
    }
    
    /* Continue using intermediate after conditional block */
    intermediate = intermediate * 3 + 7;
    
    /* More mixed-type computations to use different machine modes */
    for (i = 0; i < 5; i++) {
        char c = (char)(intermediate + i);
        short s = (short)(intermediate * i);
        int t = intermediate + c + s;
        long long ll = (long long)t * v5;
        float f = (float)t + v6;
        double d = (double)t + v7;
        
        /* More inline asm with clobbers */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            : 
            : "r" (t), "r" (i)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        
        result += t + (int)f + (int)d + (int)(ll & 0xFFFFFFFF);
    }
    
    return result + intermediate;

label1:
    intermediate += 100;
    goto label3;
    
label2:
    intermediate -= 50;
    /* fall through */
    
label3:
    intermediate *= 2;
    return result + intermediate;
}

/* Another layer of complexity */
__attribute__((noinline))
int wrapper_function(int iterations) {
    int total = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Create even more register pressure with nested loops */
        int a = i * 2;
        int b = i * 3;
        int c = i * 5;
        float d = (float)i * 1.1f;
        double e = (double)i * 1.2;
        long long f = (long long)i * 1000;
        
        for (j = 0; j < 3; j++) {
            a += j;
            b -= j;
            c ^= j;
            d += (float)j;
            e -= (double)j;
            f += j;
            
            /* Conditional use of variables */
            if ((i + j) & 1) {
                /* Use in asm with clobbers */
                asm volatile (
                    "add %0, %0, %1\n\t"
                    : "+r" (a)
                    : "r" (b)
                    : "r0", "r1", "r2", "r3", "r4", "memory"
                );
            }
        }
        
        total += stress_function(a + b + c);
    }
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    
    int result = wrapper_function(iterations);
    printf("Result: %d\n", result);
    
    /* Use result to prevent optimization */
    volatile int sink = result;
    return sink != 0 ? 0 : 1;
}

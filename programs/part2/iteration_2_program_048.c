/* early_remat_trigger.c
 * Program designed to trigger GCC's early rematerialization privatization logic
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int dummy = x;
    return dummy * 2;
}

__attribute__((noinline)) float external_func2(float x) {
    volatile float dummy = x;
    return dummy * 3.14f;
}

__attribute__((noinline)) double external_func3(double x) {
    volatile double dummy = x;
    return dummy * 2.71828;
}

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    int v3 = seed + 2;
    int v4 = seed + 3;
    int v5 = seed + 4;
    int v6 = seed + 5;
    int v7 = seed + 6;
    int v8 = seed + 7;
    int v9 = seed + 8;
    int v10 = seed + 9;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 1.2f;
    float f3 = seed * 1.3f;
    float f4 = seed * 1.4f;
    
    double d1 = seed * 1.5;
    double d2 = seed * 1.6;
    double d3 = seed * 1.7;
    double d4 = seed * 1.8;
    
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000;
    
    int *p1 = &v1;
    int *p2 = &v2;
    float *p3 = &f1;
    double *p4 = &d1;
    
    /* Key intermediate result that will be used conditionally */
    int intermediate_result = 0;
    float float_intermediate = 0.0f;
    double double_intermediate = 0.0;
    
    /* Complex computation creating register pressure */
    intermediate_result = v1 * v2 + v3 - v4;
    intermediate_result = intermediate_result * v5 / (v6 + 1);
    
    float_intermediate = f1 * f2 + f3 - f4;
    double_intermediate = d1 * d2 + d3 - d4;
    
    /* Create label addresses for goto jumps */
    void *label1 = &&LABEL_1;
    void *label2 = &&LABEL_2;
    void *label3 = &&LABEL_3;
    void *label_end = &&LABEL_END;
    
    /* Store in volatile pointer to prevent optimization */
    volatile void *volatile_label_ptr = label1;
    
    /* Volatile control variables to prevent dead code elimination */
    volatile int control1 = 1;
    volatile int control2 = 0;
    volatile int control3 = 1;
    
    /* Complex nested conditional structure */
    if (control1) {
        /* First level if */
        intermediate_result = external_func1(intermediate_result);
        
        if (v7 % 2 == 0) {
            /* Second level if */
            float_intermediate = external_func2(float_intermediate);
            
            switch (v8 % 4) {
                case 0:
                    /* Deeply nested case 0 */
                    double_intermediate = external_func3(double_intermediate);
                    
                    /* Use intermediate_result in inline asm with many clobbers */
                    /* This creates a conditional use that might need privatization */
                    asm volatile (
                        "/* Using intermediate_result: %0 */"
                        : 
                        : "r" (intermediate_result)
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory", "cc"
                    );
                    
                    /* More computations to keep variables live */
                    v9 = intermediate_result * 2;
                    f3 = float_intermediate * 2.0f;
                    d3 = double_intermediate * 2.0;
                    break;
                    
                case 1:
                    /* Different path using different modes */
                    asm volatile (
                        "/* Using float_intermediate */"
                        : 
                        : "r" (*(int*)&float_intermediate)
                        : "r0", "r1", "r2", "memory", "cc"
                    );
                    break;
                    
                case 2:
                    /* Use double with different mode */
                    asm volatile (
                        "/* Using double_intermediate */"
                        : 
                        : "r" ((int)(double_intermediate)), "r" ((int)(double_intermediate >> 32))
                        : "r0", "r1", "r2", "r3", "memory", "cc"
                    );
                    break;
                    
                case 3:
                    /* Use char and short types */
                    c1 = (char)(intermediate_result & 0xFF);
                    s1 = (short)(intermediate_result & 0xFFFF);
                    asm volatile (
                        "/* Using char and short: %0 %1 */"
                        : 
                        : "r" (c1), "r" (s1)
                        : "r0", "r1", "memory", "cc"
                    );
                    break;
            }
            
            /* Conditional goto based on volatile */
            if (control2) {
                goto *volatile_label_ptr;
            }
        } else {
            /* Alternative else path */
            intermediate_result = intermediate_result ^ v10;
            
            /* Another inline asm with clobbers */
            asm volatile (
                "/* Alternative path using intermediate_result: %0 */"
                : 
                : "r" (intermediate_result)
                : "r0", "r1", "r2", "r3", "r4", "memory", "cc"
            );
        }
        
        /* Loop to create more pressure */
        for (int i = 0; i < 3; i++) {
            intermediate_result += i;
            float_intermediate += i * 0.5f;
            double_intermediate += i * 0.25;
            
            /* Mix types in computation */
            ll1 = intermediate_result * (long long)float_intermediate;
        }
    }
    
    LABEL_1:
    /* Use intermediate_result after conditional block */
    v1 = intermediate_result * 3;
    
    /* More mixed type operations */
    f1 = (float)intermediate_result * 0.123f;
    d1 = (double)intermediate_result * 0.456;
    
    if (control3) {
        goto *label2;
    }
    
    LABEL_2:
    /* Another use of the intermediate results */
    intermediate_result = intermediate_result + (int)float_intermediate + (int)double_intermediate;
    
    /* Pointer arithmetic mixing types */
    *p1 = intermediate_result;
    *p3 = float_intermediate;
    *p4 = double_intermediate;
    
    /* Complex expression using all variables */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        c1 + s1 + (int)(ll1 & 0xFFFFFFFF);
    
    /* Final inline asm with many clobbers */
    asm volatile (
        "/* Final computation: %0 */"
        : "+r" (final_result)
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    goto *label_end;
    
    LABEL_3:
    /* Unreachable path to create control flow complexity */
    intermediate_result = intermediate_result * 999;
    goto *label1;
    
    LABEL_END:
    return final_result;
}

/* Another function with switch statement for more mode variety */
__attribute__((noinline, optimize("no-tree-vectorize")))
int switch_function(int mode) {
    int result = 0;
    
    switch (mode) {
        case 0: {
            char c = 100;
            short s = 200;
            result = c + s;
            asm volatile ("" : "+r" (result) : : "memory");
            break;
        }
        case 1: {
            int i = 300;
            long long ll = 400;
            result = i + (int)ll;
            asm volatile ("" : "+r" (result) : : "r0", "r1", "memory");
            break;
        }
        case 2: {
            float f = 500.0f;
            double d = 600.0;
            result = (int)f + (int)d;
            asm volatile ("" : "+r" (result) : : "r0", "r1", "r2", "memory");
            break;
        }
        case 3: {
            /* Mixed mode computation */
            int a = 700;
            float b = 800.0f;
            double c = 900.0;
            result = a + (int)b + (int)c;
            
            /* Complex asm with many clobbers */
            asm volatile (
                "/* Switch case 3: %0 %1 %2 */"
                : "+r" (result)
                : "r" (a), "r" (*(int*)&b)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "memory", "cc"
            );
            break;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int result1 = stress_function(seed);
    int result2 = stress_function(seed + 1);
    int result3 = switch_function(seed % 4);
    int result4 = switch_function((seed + 1) % 4);
    
    /* Use results to prevent elimination */
    volatile int final_result = 
        result1 + result2 + result3 + result4;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}

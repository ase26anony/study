/* early-remat-trigger.c
 * Designed to trigger early_remat::privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -c early-remat-trigger.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func3(double x) { return x / 3.14159; }

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-tree-vectorize", "no-gcse")))
int stress_function(int seed) {
    /* Create high register pressure with many variables of different types */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    long long v5 = (long long)seed * 1000LL;
    long long v6 = v5 + 123456789LL;
    float f1 = (float)seed * 0.5f;
    float f2 = f1 + 3.14f;
    double d1 = (double)seed * 2.71828;
    double d2 = d1 / 1.41421;
    char c1 = (char)(seed & 0xFF);
    short s1 = (short)(seed * 3);
    int *p1 = &v2;
    float *p2 = &f1;
    double *p3 = &d1;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 + v2 + v3 + v4;
    float float_critical = f1 * f2;
    double double_critical = d1 + d2;
    
    /* Label addresses for complex control flow */
    void* label1 = &&LABEL_1;
    void* label2 = &&LABEL_2;
    void* label3 = &&LABEL_3;
    void* label4 = &&LABEL_4;
    
    /* Store label address in volatile pointer */
    volatile_jump_target = (jump_func_t)label2;
    
    /* Complex nested conditionals */
    if (v1 > 0) {
        if (v2 % 3 == 0) {
            /* Use critical_value in inline asm with many clobbers */
            asm volatile (
                "/* Using critical_value: %0 */"
                : 
                : "r" (critical_value)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory", "cc"
            );
            
            /* Call non-inlineable function with critical value */
            critical_value = external_func1(critical_value);
            
            switch (v3 & 0x3) {
                case 0:
                    /* Another conditional use with different mode (float) */
                    float_critical = external_func2(float_critical);
                    asm volatile (
                        "/* Float operation: %0 */"
                        : "+t" (float_critical)
                        :: "memory"
                    );
                    break;
                case 1:
                    /* Double mode use */
                    double_critical = external_func3(double_critical);
                    asm volatile (
                        "/* Double operation: %0 */"
                        : "+f" (double_critical)
                        :: "memory"
                    );
                    break;
                case 2:
                    /* Char/short mode mixing */
                    c1 = (char)(critical_value & 0xFF);
                    s1 = (short)(critical_value * 2);
                    asm volatile (
                        "/* Char/Short mix: %0, %1 */"
                        : 
                        : "r" ((int)c1), "r" ((int)s1)
                        : "memory"
                    );
                    break;
                default:
                    /* Pointer mode use */
                    *p1 = critical_value;
                    *p2 = float_critical;
                    asm volatile (
                        "/* Pointer store */"
                        :
                        : "r" (p1), "r" (p2)
                        : "memory"
                    );
            }
            
            /* Conditional goto via volatile pointer */
            if (v4 > 100) {
                goto *volatile_jump_target;
            }
        } else if (v2 % 5 == 0) {
            /* Alternative path with different register usage */
            critical_value *= 2;
            float_critical *= 2.0f;
            double_critical *= 2.0;
            
            /* More inline asm with clobbers */
            asm volatile (
                "/* Alternative path asm */"
                :
                : "r" (critical_value), "t" (float_critical), "f" (double_critical)
                : "r0", "r1", "r2", "r3", "r4", "r5", "memory", "cc"
            );
        }
    }
    
    /* Loop to increase pressure and create more live ranges */
    for (int i = 0; i < 10; i++) {
        volatile int loop_var = i * critical_value;
        
        /* Nested switch inside loop */
        switch (i & 0x7) {
            case 0: v5 += loop_var; break;
            case 1: v6 -= loop_var; break;
            case 2: f1 += (float)loop_var; break;
            case 3: f2 -= (float)loop_var; break;
            case 4: d1 += (double)loop_var; break;
            case 5: d2 -= (double)loop_var; break;
            case 6: 
                /* Another conditional asm use */
                asm volatile (
                    "/* Loop case 6: %0, %1 */"
                    : 
                    : "r" (critical_value), "r" (loop_var)
                    : "r8", "r9", "r10", "memory"
                );
                break;
            default:
                /* Mixed mode operation */
                long long temp = v5 + v6 + (long long)critical_value;
                asm volatile (
                    "/* Default case: %0 */"
                    : "+r" (temp)
                    :: "memory"
                );
        }
        
        /* Conditional jump out of loop */
        if (i == 5 && v1 > 1000) {
            volatile_jump_target = (jump_func_t)label3;
            goto *volatile_jump_target;
        }
    }
    
LABEL_1:
    /* Use critical_value again after complex region */
    int result1 = critical_value * 3;
    
LABEL_2:
    /* More uses with different modes */
    float result2 = float_critical + f1 + f2;
    
LABEL_3:
    double result3 = double_critical * d1 * d2;
    
    /* Final aggregation to prevent elimination */
    int final_result = result1 + (int)result2 + (int)result3;
    final_result += (int)(v5 >> 32) + (int)(v6 >> 32);
    final_result += c1 + s1;
    
    /* One more conditional asm with critical_value */
    if (final_result % 2 == 0) {
        asm volatile (
            "/* Final asm: %0, %1 */"
            : 
            : "r" (critical_value), "r" (final_result)
            : "r0", "r1", "r2", "memory", "cc"
        );
    }
    
LABEL_4:
    return final_result + critical_value;
}

/* Wrapper with different compilation modes */
#ifdef TEST_32BIT
__attribute__((target("arch=i686")))
#endif
int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += stress_function(seed + i);
    }
    
    /* Use result to prevent elimination */
    volatile int sink = total;
    return sink % 256;
}

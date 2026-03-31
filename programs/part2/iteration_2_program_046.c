/* early-remat-test.c
 * Designed to trigger early_remat::privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early-remat-test.c -o test
 * Also try: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_helper(int x, int y) {
    volatile int result = x * y;
    return result + (result >> 3);
}

/* Another non-inlineable function */
__attribute__((noinline))
double fp_helper(double a, double b) {
    volatile double tmp = a;
    return tmp * b - a;
}

/* Function pointer with volatile to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = external_helper;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    short v3 = (short)(seed + 100);
    char v4 = (char)(seed % 256);
    long long v5 = (long long)seed * 1000LL;
    float v6 = (float)seed * 1.5f;
    double v7 = (double)seed * 2.5;
    int *v8 = &v1;
    float *v9 = &v6;
    
    /* Additional variables for more pressure */
    int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    int a5 = seed + 5, a6 = seed + 6, a7 = seed + 7, a8 = seed + 8;
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    double d1 = seed * 0.01, d2 = seed * 0.02, d3 = seed * 0.03;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = 0;
    
    /* Complex conditional computation with deeply nested if-else */
    if (v1 > 100) {
        critical_value = v2 * 3;
        
        if (v3 < 50) {
            critical_value += v4 * 2;
            
            /* Use volatile variable to prevent optimization */
            volatile int vol_check = v1;
            if (vol_check % 3 == 0) {
                /* Nested switch statement */
                switch (v4 % 4) {
                    case 0:
                        critical_value = external_helper(critical_value, v2);
                        /* Inline assembly with many clobbered registers */
                        __asm__ volatile (
                            "mov %0, %%eax\n\t"
                            "add $1, %%eax\n\t"
                            "mov %%eax, %0\n\t"
                            : "+r" (critical_value)
                            : 
                            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                              "memory", "cc"
                        );
                        break;
                    case 1:
                        critical_value = volatile_func_ptr(critical_value, v3);
                        break;
                    case 2:
                        /* Use the critical value in floating point computation */
                        v6 = (float)critical_value * 0.5f;
                        critical_value = (int)v6;
                        break;
                    case 3:
                    default:
                        /* More complex path with mixed types */
                        v5 = (long long)critical_value * v5;
                        critical_value = (int)(v5 % 1000);
                        break;
                }
            } else {
                /* Another path with different register usage */
                v7 = fp_helper((double)critical_value, d1);
                critical_value = (int)v7;
            }
        } else {
            /* Different computation path */
            critical_value = v2 - v3 + v4;
        }
    } else {
        /* Alternative computation */
        critical_value = v1 * v2 - v3;
    }
    
    /* Use goto with computed labels for irreducible control flow */
    void *label_table[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile int label_selector = v1 % 4;
    
    /* Force the compiler to keep all variables live across the jump */
    int pre_jump_sum = v1 + v2 + v3 + v4 + a1 + a2 + a3 + a4;
    
    /* Opaque jump based on volatile */
    goto *label_table[label_selector];
    
label1:
    /* Use critical_value after jump */
    critical_value += pre_jump_sum % 100;
    /* Fall through */
    
label2:
    /* More computation with mixed types */
    f1 = (float)critical_value * 0.25f;
    critical_value += (int)f1;
    goto label4;
    
label3:
    /* Different computation path */
    d2 = (double)critical_value * 0.33;
    critical_value -= (int)d2;
    /* Fall through */
    
label4:
    /* Final computation using all variables to keep them live */
    int final_result = critical_value;
    final_result += v1 + v2 + v3 + v4;
    final_result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    final_result += (int)f1 + (int)f2 + (int)f3;
    final_result += (int)d1 + (int)d2 + (int)d3;
    
    /* Use inline assembly that clobbers many registers */
    __asm__ volatile (
        "/* Clobber many registers to increase pressure */\n\t"
        "push %%eax\n\t"
        "push %%ebx\n\t"
        "push %%ecx\n\t"
        "push %%edx\n\t"
        "push %%esi\n\t"
        "push %%edi\n\t"
        "mov %0, %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        "pop %%edi\n\t"
        "pop %%esi\n\t"
        "pop %%edx\n\t"
        "pop %%ecx\n\t"
        "pop %%ebx\n\t"
        "pop %%eax\n\t"
        : "+r" (final_result)
        : 
        : "memory", "cc"
    );
    
    return final_result;
}

/* Another function with different data type patterns */
__attribute__((noinline))
long long mixed_type_stress(int base) {
    /* Variables with different modes */
    char c1 = base & 0xFF;
    short s1 = base * 2;
    int i1 = base * 3;
    long long ll1 = (long long)base * 100LL;
    float f1 = (float)base * 1.1f;
    double d1 = (double)base * 2.2;
    
    /* Complex conditional with switch */
    volatile int selector = base % 5;
    long long result = 0;
    
    switch (selector) {
        case 0:
            result = (long long)c1 * s1;
            /* Force mode mixing */
            f1 = (float)result;
            result = (long long)f1;
            break;
        case 1:
            result = i1 + ll1;
            d1 = (double)result;
            result = (long long)d1;
            break;
        case 2:
            /* Use in inline asm with specific register constraints */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "imul %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r" (i1)
                : "r" (i1), "r" (s1)
                : "eax", "cc"
            );
            result = i1;
            break;
        case 3:
            /* Nested conditionals */
            if (c1 > 50) {
                result = ll1 / (c1 + 1);
                if (s1 < 100) {
                    result += f1;
                }
            }
            break;
        case 4:
        default:
            result = (long long)(d1 * f1);
            break;
    }
    
    /* Use result after switch in another computation */
    for (int i = 0; i < 3; i++) {
        result += c1 + s1 + i1;
        /* Volatile to prevent loop optimization */
        volatile int loop_check = i;
        if (loop_check == 1) {
            result *= 2;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call stress functions multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
        total += mixed_type_stress(seed + i) % 1000;
    }
    
    printf("Result: %d\n", total);
    return total % 256;
}

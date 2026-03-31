/* early_remat_test.c
 * Test program to trigger early rematerialization's privatize_cond_register_use
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

__attribute__((noinline)) float external_func2(float x) {
    volatile float dummy = x;
    return dummy * 2.0f;
}

__attribute__((noinline)) double external_func3(double x) {
    volatile double dummy = x;
    return dummy / 3.0;
}

/* Function pointer with volatile to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    float f1 = seed * 1.5f;
    float f2 = seed * 2.5f;
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    char c1 = seed & 0xFF;
    short s1 = seed & 0x7FFF;
    long long ll1 = (long long)seed * 1000000LL;
    int *ptr1 = &v2;
    float *ptr2 = &f1;
    double *ptr3 = &d1;
    
    /* Intermediate result that will be used conditionally */
    int intermediate_result = 0;
    float float_intermediate = 0.0f;
    double double_intermediate = 0.0;
    
    /* Complex computation creating register pressure */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * 1103515245 + 12345;
        v2 = v2 ^ (v1 >> 16);
        v3 = v3 + (v2 & 0xFF);
        v4 = v4 | (v3 << 8);
        f1 = f1 * 1.1f + (float)v1;
        f2 = f2 / 1.2f - (float)v2;
        d1 = d1 * 1.01 + (double)v3;
        d2 = d2 / 1.02 - (double)v4;
        c1 = c1 + (char)i;
        s1 = s1 - (short)(i * 100);
        ll1 = ll1 + (long long)(v1 * v2);
        
        /* Accumulate intermediate results */
        intermediate_result += v1 + v2 + v3 + v4;
        float_intermediate += f1 + f2;
        double_intermediate += d1 + d2;
    }
    
    /* Store label addresses for complex control flow */
    void *label1 = &&LABEL_1;
    void *label2 = &&LABEL_2;
    void *label3 = &&LABEL_3;
    void *label4 = &&LABEL_4;
    
    /* Complex conditional structure with deeply nested blocks */
    volatile int control = seed % 7;
    
    /* Use intermediate_result in conditional blocks */
    switch (control) {
        case 0: {
            /* Deeply nested if-else chain */
            if (v1 > 1000) {
                if (v2 < 500) {
                    if (f1 > 50.0f) {
                        /* Critical conditional use of intermediate_result */
                        int temp = intermediate_result;
                        /* Inline assembly that clobbers many registers */
                        __asm__ volatile (
                            "mov %0, %0\n\t"
                            :
                            : "r" (temp)
                            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                              "r8", "r9", "r10", "r11", "r12", "memory"
                        );
                        intermediate_result = external_func1(temp);
                    }
                }
            }
            break;
        }
        case 1: {
            /* Different mode: float */
            float ftemp = float_intermediate;
            /* Force register use with volatile */
            volatile float vftemp = ftemp;
            if (vftemp > 0.0f) {
                /* Use in external function call */
                float_intermediate = external_func2(vftemp);
                /* More inline assembly with clobber */
                __asm__ volatile (
                    "fmov s0, %s0\n\t"
                    :
                    : "w" (float_intermediate)
                    : "s0", "s1", "s2", "s3", "s4", "s5", "memory"
                );
            }
            break;
        }
        case 2: {
            /* Double mode */
            double dtemp = double_intermediate;
            if (dtemp != 0.0) {
                /* Complex computation using dtemp */
                for (int j = 0; j < 5; j++) {
                    dtemp = dtemp * 1.5 - (double)j;
                }
                double_intermediate = external_func3(dtemp);
                
                /* Assembly with many clobbered registers */
                __asm__ volatile (
                    "mov x0, %0\n\t"
                    "mov x1, x0\n\t"
                    :
                    : "r" ((long long)dtemp)
                    : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                      "x8", "x9", "x10", "x11", "x12", "memory"
                );
            }
            break;
        }
        case 3: {
            /* Short/char modes */
            short stemp = s1;
            char ctemp = c1;
            if (stemp > 100) {
                /* Mix of different sized operations */
                int mixed = (int)stemp * (int)ctemp;
                intermediate_result += mixed;
                
                /* Assembly with mixed size registers */
                __asm__ volatile (
                    "mov w0, %w0\n\t"
                    "mov w1, w0\n\t"
                    :
                    : "r" (mixed)
                    : "w0", "w1", "w2", "w3", "w4", "w5", "memory"
                );
            }
            break;
        }
        default: {
            /* Long long mode */
            long long lltemp = ll1;
            if (lltemp > 1000000LL) {
                /* Complex chain of operations */
                lltemp = lltemp >> 4;
                lltemp = lltemp * 3;
                lltemp = lltemp + intermediate_result;
                
                /* Use volatile function pointer to create opaque control flow */
                volatile_jump_target = (jump_func_t)label1;
                if (volatile_jump_target) {
                    /* This creates complex control flow for the compiler */
                    goto *label1;
                }
            }
            break;
        }
    }
    
    /* Labels for goto-based control flow */
    LABEL_1:
    intermediate_result += 1000;
    goto *label2;
    
    LABEL_2:
    float_intermediate += 500.0f;
    goto *label3;
    
    LABEL_3:
    double_intermediate *= 2.0;
    goto *label4;
    
    LABEL_4:
    /* Use intermediate results again after conditional blocks */
    int final_result = intermediate_result;
    final_result += (int)float_intermediate;
    final_result += (int)double_intermediate;
    
    /* More register pressure */
    for (int k = 0; k < 20; k++) {
        v1 = (v1 * 3) / 2;
        v2 = v2 ^ k;
        v3 = v3 + v1 - v2;
        v4 = v4 | (v3 & 0xFFFF);
        
        /* Use all variables to keep them live */
        f1 = f1 + (float)v1 / 100.0f;
        f2 = f2 - (float)v2 / 100.0f;
        d1 = d1 + (double)v3 / 1000.0;
        d2 = d2 - (double)v4 / 1000.0;
        c1 = c1 + (char)(k & 0xFF);
        s1 = s1 - (short)k;
        ll1 = ll1 + (long long)(v1 * v3);
        
        /* Final use of intermediate results in loop */
        final_result += v1 + v2 + v3 + v4;
    }
    
    /* Additional complex conditional with function pointers */
    void *labels[] = {&&END_LOOP, &&CONTINUE_LOOP};
    volatile int label_index = (final_result > 0) ? 0 : 1;
    
    CONTINUE_LOOP:
    /* More computations to extend live ranges */
    final_result = final_result * 2 - 1;
    final_result = final_result ^ 0xAAAAAAAA;
    final_result = abs(final_result);
    
    if (final_result & 1) {
        goto *labels[label_index];
    }
    
    END_LOOP:
    /* Return aggregated result to prevent elimination */
    return final_result + v1 + v2 + v3 + v4 + (int)f1 + (int)f2 + 
           (int)d1 + (int)d2 + c1 + s1 + (int)ll1;
}

/* Another stress function with different patterns */
__attribute__((noinline, optimize("no-tree-loop-optimize")))
int stress_function2(int base) {
    int a = base, b = base + 1, c = base + 2, d = base + 3;
    int e = base + 4, f = base + 5, g = base + 6, h = base + 7;
    int i = base + 8, j = base + 9, k = base + 10, l = base + 11;
    int m = base + 12, n = base + 13, o = base + 14, p = base + 15;
    
    /* Complex conditional computation */
    int result = 0;
    volatile int cond = base % 3;
    
    if (cond == 0) {
        /* Chain of dependent computations */
        result = a * b;
        result += c * d;
        result += e * f;
        result += g * h;
        
        /* Conditional use inside nested block */
        if (result > 1000) {
            int temp = result;
            /* Assembly that forces register use */
            __asm__ volatile (
                "add %0, %0, #1\n\t"
                : "+r" (temp)
                :
                : "cc", "memory", "r0", "r1", "r2", "r3"
            );
            result = temp;
        }
    } else if (cond == 1) {
        /* Different computation pattern */
        result = i ^ j;
        result |= k & l;
        result = result << (m & 0xF);
        result = result >> (n & 0x7);
        
        /* Deeply nested condition */
        if (o > p) {
            if ((o - p) > 10) {
                if ((o * p) < 1000) {
                    /* Critical conditional register use */
                    volatile int vtemp = result;
                    result = external_func1(vtemp);
                }
            }
        }
    } else {
        /* Mixed type computations */
        float f1 = (float)a / (float)b;
        double d1 = (double)c * (double)d;
        result = (int)(f1 * 100.0f) + (int)(d1 / 10.0);
        
        /* Loop with conditional break */
        for (int x = 0; x < 10; x++) {
            result += x;
            if (result > 500) {
                /* Use goto for complex control flow */
                goto *&&LOOP_EXIT;
            }
        }
        LOOP_EXIT:
        result += 100;
    }
    
    /* Use all variables at the end */
    return result + a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main(int argc, char **argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing early rematerialization privatization...\n");
    
    /* Call stress functions multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += stress_function(seed + i);
        total += stress_function2(seed + i * 2);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

/* early-remat-trigger.c
 * Program designed to trigger early rematerialization's privatization logic
 * for conditionally-used registers (lines 930-937 in early-remat.cc)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline, noclone))
int external_helper(int x, int y) {
    volatile int result = x * y;
    return result + (result >> 3);
}

/* Another non-inlineable function */
__attribute__((noinline, noclone))
double fp_helper(double a, double b) {
    volatile double temp = a + b;
    return temp * 0.5;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    long long v5 = (long long)seed * seed;
    float v6 = seed * 0.5f;
    double v7 = seed * 0.25;
    char v8 = seed & 0xFF;
    short v9 = seed & 0x7FFF;
    int v10 = seed ^ 0x1234;
    int v11 = seed | 0xABCD;
    float v12 = v6 * 2.0f;
    double v13 = v7 * 3.0;
    int v14 = v2 + v3;
    long long v15 = v5 + v14;
    
    /* Pointer variables */
    int *p1 = &v1;
    volatile int *p2 = &v2;
    float *p3 = &v6;
    double *p4 = &v7;
    
    /* Complex intermediate computation - this will be our target register */
    int intermediate = v1 + v2 + v3 + v4;
    intermediate = intermediate * 3 - 7;
    
    /* Label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = NULL;
    
    /* Volatile control variable */
    volatile int control = seed % 5;
    
    /* Deeply nested conditional structure */
    if (control > 0) {
        intermediate += external_helper(v1, v2);
        
        switch (control) {
            case 1:
                /* Nested if inside switch */
                if (v3 > 100) {
                    intermediate *= 2;
                    
                    /* Use intermediate in inline asm with many clobbers */
                    /* This should trigger privatization consideration */
                    asm volatile (
                        "add %[val], %[val], #5\n\t"
                        "mov r0, %[val]\n\t"
                        "mov r1, %[val]\n\t"
                        "mov r2, %[val]\n\t"
                        : [val] "+r" (intermediate)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    
                    v6 = fp_helper(v6, intermediate);
                } else {
                    intermediate /= 2;
                }
                break;
                
            case 2:
                for (int i = 0; i < 3; i++) {
                    intermediate += i;
                    if (i == 1) {
                        /* Another conditional use with different mode (float) */
                        float temp_f = (float)intermediate;
                        asm volatile (
                            "fmsr s0, %[val]\n\t"
                            "fadds s0, s0, s0\n\t"
                            "fmrs %[val], s0\n\t"
                            : [val] "+r" (temp_f)
                            :
                            : "s0", "s1", "s2", "s3", "s4", "s5", "memory"
                        );
                        v12 = temp_f;
                    }
                }
                break;
                
            case 3:
                /* Use label address for opaque control flow */
                volatile_label_ptr = labels[2];
                goto *volatile_label_ptr;
                
            case 4:
                /* Mixed type computation */
                double temp_d = (double)intermediate;
                intermediate = (int)(temp_d * 1.5);
                break;
                
            default:
                intermediate = ~intermediate;
        }
        
        /* Use intermediate again after conditional block */
        v10 = intermediate + v10;
    } else {
        intermediate -= external_helper(v4, v3);
    }
    
    /* More complex control flow with goto */
    if (v5 > 1000) {
        volatile_label_ptr = labels[1];
    } else {
        volatile_label_ptr = labels[3];
    }
    
    /* Jump to label based on volatile pointer */
    if (control == 3) {
        goto *volatile_label_ptr;
    }
    
label1:
    /* Use intermediate with different mode (char) */
    v8 = (char)(intermediate & 0xFF);
    intermediate = intermediate >> 8;
    
label2:
    /* Use intermediate with different mode (short) */
    v9 = (short)(intermediate & 0x7FFF);
    
    /* Another conditional block */
    if (v7 > 0.0) {
        /* Inline asm that uses intermediate and clobbers many registers */
        asm volatile (
            "mov r0, %[val]\n\t"
            "add r0, r0, #100\n\t"
            "mov %[val], r0\n\t"
            : [val] "+r" (intermediate)
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "memory"
        );
    }
    
label3:
    /* Final computation using all variables to prevent elimination */
    long long result = (long long)intermediate;
    result += v1 + v2 + v3 + v4;
    result += (long long)(v6 * 100.0f);
    result += (long long)(v7 * 200.0);
    result += v8 + v9 + v10 + v11;
    result += (long long)(v12 * 50.0f);
    result += (long long)(v13 * 75.0);
    result += v14 + v15;
    result += *p1 + *p2;
    result += (long long)(*p3 * 10.0f);
    result += (long long)(*p4 * 20.0);
    
    return (int)(result & 0x7FFFFFFF);
    
label4:
    /* Alternative path */
    intermediate = intermediate * 2 + 1;
    goto label3;
    
label5:
    /* Another alternative path */
    intermediate = intermediate / 2 - 1;
    goto label2;
}

/* Secondary stress function with different patterns */
__attribute__((noinline, optimize("no-tree-loop-optimize")))
double mixed_mode_stress(int iterations) {
    volatile double accumulator = 0.0;
    float f_acc = 0.0f;
    int i_acc = 0;
    long long ll_acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create register pressure with many live variables */
        int a = i * 2;
        int b = i + 1;
        float c = i * 0.5f;
        double d = i * 0.25;
        long long e = (long long)i * i;
        
        /* Intermediate with complex computation */
        int intermediate = a * b - i;
        
        /* Deep conditional with mixed mode usage */
        if (i % 3 == 0) {
            /* Use as float */
            float f_temp = (float)intermediate;
            asm volatile (
                "vmov.f32 s0, %[val]\n\t"
                "vadd.f32 s0, s0, s0\n\t"
                "vmov.f32 %[val], s0\n\t"
                : [val] "+r" (f_temp)
                :
                : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "memory"
            );
            c = f_temp;
            intermediate = (int)f_temp;
        } else if (i % 3 == 1) {
            /* Use as double */
            double d_temp = (double)intermediate;
            asm volatile (
                "vmov.f64 d0, %[val]\n\t"
                "vadd.f64 d0, d0, d0\n\t"
                "vmov.f64 %[val], d0\n\t"
                : [val] "+r" (d_temp)
                :
                : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "memory"
            );
            d = d_temp;
            intermediate = (int)d_temp;
        } else {
            /* Use as long long */
            long long ll_temp = (long long)intermediate;
            asm volatile (
                "add %[val], %[val], #1000\n\t"
                : [val] "+r" (ll_temp)
                :
                : "r0", "r1", "r2", "r3", "memory"
            );
            e = ll_temp;
            intermediate = (int)ll_temp;
        }
        
        /* Use intermediate after conditional block */
        i_acc += intermediate;
        f_acc += c;
        accumulator += d;
        ll_acc += e;
        
        /* Opaque control flow every 10 iterations */
        if (i % 10 == 0) {
            void* my_labels[] = { &&loop_continue, &&loop_skip };
            volatile void* jump_target = my_labels[i % 2];
            goto *jump_target;
            
        loop_skip:
            intermediate += 1000;
            continue;
            
        loop_continue:
            intermediate -= 500;
        }
    }
    
    return accumulator + f_acc + i_acc + (double)ll_acc;
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing early rematerialization privatization trigger...\n");
    
    /* Call stress functions multiple times with different parameters */
    int result1 = stress_function(seed);
    printf("Stress function result 1: %d\n", result1);
    
    int result2 = stress_function(seed * 2);
    printf("Stress function result 2: %d\n", result2);
    
    double result3 = mixed_mode_stress(50);
    printf("Mixed mode stress result: %f\n", result3);
    
    /* Use volatile function pointer to prevent optimization */
    volatile_func_ptr = (void(*)())stress_function;
    if (seed % 2 == 0) {
        ((int(*)(int))volatile_func_ptr)(seed * 3);
    }
    
    return (result1 + (int)result3) & 0xFF;
}

/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 3.14159f; }
__attribute__((noinline)) double external_func3(double x) { return x / 2.71828; }

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    float f1 = seed * 0.5f;
    float f2 = seed * 1.5f;
    double d1 = seed * 0.25;
    double d2 = seed * 0.75;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000LL;
    int *p1 = &v1;
    float *p2 = &f1;
    double *p3 = &d1;
    
    /* Intermediate result that will be used conditionally */
    int critical_value = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Complex computation with mixed types */
    critical_value = v1 + v2 + v3;
    float_result = f1 + f2 + (float)critical_value;
    double_result = d1 + d2 + (double)float_result;
    
    /* Labels for goto jumps */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    
    /* Volatile variable to prevent dead code elimination */
    volatile int control = seed % 5;
    
    /* Use volatile_jump_target to create opaque control flow */
    if (volatile_jump_target) {
        volatile_jump_target();
    }
    
    /* Deeply nested conditional blocks */
    if (control > 0) {
        switch (control) {
            case 1: {
                /* Nested if-else chain */
                if (v1 > 100) {
                    /* Use critical_value in inline assembly with many clobbers */
                    asm volatile (
                        "add %[val], %[val], #1\n\t"
                        "mov r0, %[val]\n\t"
                        "mov r1, %[val]\n\t"
                        "mov r2, %[val]\n\t"
                        "mov r3, %[val]\n\t"
                        : [val] "+r" (critical_value)
                        :
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    
                    /* Call non-inlineable function */
                    critical_value = external_func1(critical_value);
                    
                    /* Another level of nesting */
                    if (f1 > 50.0f) {
                        asm volatile (
                            "eor %[val], %[val], #0xFF\n\t"
                            : [val] "+r" (critical_value)
                            :
                            : "cc", "memory"
                        );
                    }
                } else {
                    /* Different path using float_result */
                    float_result = external_func2(float_result);
                    asm volatile (
                        "fadds s0, s0, s0\n\t"
                        : "+w" (float_result)
                        :
                        : "s0", "s1", "s2", "s3", "memory"
                    );
                }
                break;
            }
            
            case 2: {
                /* Use double_result with mixed operations */
                double_result = external_func3(double_result);
                
                /* Complex expression with multiple uses */
                ll1 = (ll1 * critical_value) / (s1 + 1);
                
                /* Inline assembly that clobbers many registers */
                asm volatile (
                    "mov x0, %[ll]\n\t"
                    "mov x1, %[ll]\n\t"
                    "mov x2, %[ll]\n\t"
                    "add x0, x0, x1\n\t"
                    : 
                    : [ll] "r" (ll1)
                    : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                      "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                      "memory"
                );
                break;
            }
            
            case 3: {
                /* Pointer arithmetic creating aliasing */
                *p1 = critical_value + *p2 + *p3;
                
                /* Jump via label address */
                goto *labels[control];
                
            label3:
                /* More register pressure */
                v2 = v3 * critical_value;
                f1 = f2 * float_result;
                d1 = d2 + double_result;
                break;
            }
            
            case 4: {
                /* Loop with conditional break */
                for (int i = 0; i < 10; i++) {
                    if (i == critical_value % 10) {
                        /* Use in conditional block inside loop */
                        asm volatile (
                            "mul %[val], %[val], %[val]\n\t"
                            : [val] "+r" (critical_value)
                            :
                            : "cc", "memory"
                        );
                        break;
                    }
                    v1 += i;
                }
                break;
            }
        }
        
        /* After switch, use critical_value again */
        critical_value = external_func1(critical_value);
        
        /* More mixed-type operations */
        float_result = (float)critical_value * 0.5f;
        double_result = (double)float_result * 2.0;
        
        /* Conditional goto based on volatile */
        if (v1 & 1) {
            goto *labels[0];
        }
    }
    
    /* Common code path after conditional blocks */
    v2 = critical_value * 2;
    v3 = critical_value + v2;
    
    /* Final aggregation to prevent elimination */
    int result = critical_value + (int)float_result + (int)double_result + v1 + v2 + v3 + c1 + s1 + (int)ll1;
    
    return result;

/* Label definitions */
label1:
    v1 += 1000;
    goto label5;
    
label2:
    v2 += 2000;
    goto label4;
    
label4:
    v3 += 3000;
    critical_value = v1 + v2 + v3;
    goto after_labels;
    
label5:
    critical_value *= 2;
    /* fall through */
    
after_labels:
    return critical_value;
}

/* Main function with varying inputs */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call stress function with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Occasionally change the volatile jump target */
        if (i % 23 == 0) {
            volatile_jump_target = (jump_func_t)&&dummy_label;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(total) : "memory");
    
    return total % 256;

dummy_label:
    return 0;
}

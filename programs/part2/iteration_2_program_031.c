/* early-remat-test.c
 * Designed to trigger early rematerialization's privatize_cond_register_use
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) 
int external_helper(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

__attribute__((noinline))
double external_double_helper(double x) {
    volatile double dummy = x;
    return dummy * 2.0;
}

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_early_remat(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v2 + 7;
    int v4 = v3 - seed;
    int v5 = v4 * 3;
    int v6 = v5 / 2;
    int v7 = v6 << 3;
    int v8 = v7 ^ 0xFF;
    int v9 = v8 | 0xAA;
    int v10 = v9 & 0x55;
    
    /* Mixed data types for different machine modes */
    char c1 = (char)(v1 & 0xFF);
    short s1 = (short)(v2 * 2);
    long long ll1 = (long long)v3 * 1000000LL;
    float f1 = (float)v4 * 1.5f;
    double d1 = (double)v5 * 2.71828;
    void* ptr1 = &v1;
    
    /* Additional variables for more pressure */
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f2, f3, f4;
    double d2, d3, d4;
    char c2, c3, c4;
    short s2, s3, s4;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 * v2 + v3 - v4;
    
    /* Complex nested conditional structure */
    volatile int control = seed % 7;
    
    /* Define labels for goto jumps */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, 
                      &&label5, &&label6, &&label7 };
    
    /* Store label addresses in volatile pointer */
    volatile_jump_target = (jump_func_t)labels[control % 7];
    
    /* First level of conditionals */
    if (control > 0) {
        /* Nested switch inside if */
        switch (control) {
            case 1:
            case 2:
                /* Use critical_value in inline asm with many clobbers */
                asm volatile (
                    "/* Using critical_value: %0 */"
                    : 
                    : "r" (critical_value)
                    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                      "memory", "cc"
                );
                
                /* Call non-inlineable function with critical_value */
                critical_value = external_helper(critical_value);
                break;
                
            case 3:
            case 4:
                /* Different use of critical_value with float conversion */
                f1 = (float)critical_value / 3.14f;
                asm volatile (
                    "/* Float operation with %0 */"
                    : 
                    : "r" (critical_value)
                    : "r0", "r1", "r2", "memory"
                );
                break;
                
            default:
                /* Deeply nested if-else chain */
                if (seed % 3 == 0) {
                    if (seed % 5 == 0) {
                        /* Use critical_value in double conversion */
                        d1 = external_double_helper((double)critical_value);
                        asm volatile (
                            "/* Double conversion */"
                            :
                            : "r" (critical_value)
                            : "r0", "r1", "r2", "r3", "memory"
                        );
                    } else {
                        /* Use in pointer arithmetic */
                        ptr1 = (char*)ptr1 + critical_value;
                    }
                } else if (seed % 3 == 1) {
                    /* Use in mixed-type computation */
                    ll1 = (long long)critical_value * 100LL;
                    asm volatile (
                        "/* Long long operation */"
                        :
                        : "r" (critical_value)
                        : "r0", "r1", "r2", "r3", "memory"
                    );
                }
                break;
        }
        
        /* Conditional goto based on volatile pointer */
        if (volatile_jump_target != NULL) {
            goto *((void*)volatile_jump_target);
        }
    }
    
    /* Define the labels for jumping */
    label1:
        v11 = critical_value + 100;
        critical_value = v11 * 2;
        goto after_labels;
        
    label2:
        v12 = critical_value - 50;
        critical_value = v12 | 0x0F;
        goto after_labels;
        
    label3:
        v13 = critical_value * 3;
        critical_value = v13 ^ 0xCC;
        goto after_labels;
        
    label4:
        v14 = critical_value / 2;
        critical_value = v14 + 777;
        goto after_labels;
        
    label5:
        v15 = critical_value << 2;
        critical_value = v15 & 0x3F;
        goto after_labels;
        
    label6:
        v16 = critical_value % 17;
        critical_value = v16 * v16;
        goto after_labels;
        
    label7:
        v17 = critical_value + 999;
        critical_value = v17 - 333;
        goto after_labels;
        
    after_labels:
    
    /* More computations to keep variables live */
    v18 = v1 + v2 + v3 + v4 + v5;
    v19 = v6 + v7 + v8 + v9 + v10;
    
    /* Use critical_value again after conditional blocks */
    int final_result = critical_value;
    
    /* Force all variables to be used to prevent elimination */
    final_result += v18 + v19;
    final_result += (int)c1 + (int)s1;
    final_result += (int)(ll1 & 0xFFFFFFFF);
    final_result += (int)f1;
    final_result += (int)d1;
    final_result += (int)((intptr_t)ptr1 & 0xFF);
    
    /* Additional register pressure with inline asm */
    asm volatile (
        "/* Final computation with many clobbers */"
        : 
        : "r" (final_result), "r" (v18), "r" (v19)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    return final_result;
}

/* Second stress function with different patterns */
__attribute__((noinline, optimize("no-tree-loop-optimize")))
int stress_mixed_modes(int base) {
    /* Variables with different modes */
    char c = base & 0xFF;
    short s = base * 2;
    int i = base * 3;
    long long ll = (long long)base * 1000LL;
    float f = (float)base * 1.1f;
    double d = (double)base * 2.2;
    
    volatile int control = base % 11;
    int result = 0;
    
    /* Complex switch with mixed-type operations */
    switch (control) {
        case 0:
            /* char mode operations */
            result = c * 2;
            asm volatile ("/* char mode */" : : "r" (c) : "r0", "r1", "memory");
            break;
            
        case 1:
            /* short mode operations */
            result = s + 1000;
            asm volatile ("/* short mode */" : : "r" (s) : "r0", "r1", "r2", "memory");
            break;
            
        case 2:
            /* int mode operations */
            result = i - 500;
            asm volatile ("/* int mode */" : : "r" (i) : "r0", "r1", "r2", "r3", "memory");
            break;
            
        case 3:
            /* long long mode operations */
            result = (int)(ll >> 16);
            asm volatile ("/* long long mode */" : : "r" (ll) : "r0", "r1", "r2", "r3", "memory");
            break;
            
        case 4:
            /* float mode operations */
            result = (int)f;
            asm volatile ("/* float mode */" : : "r" (result) : "r0", "r1", "memory");
            break;
            
        case 5:
            /* double mode operations */
            result = (int)d;
            asm volatile ("/* double mode */" : : "r" (result) : "r0", "r1", "r2", "memory");
            break;
            
        default:
            /* Mixed mode computation */
            result = c + s + i + (int)(ll & 0xFFFF) + (int)f + (int)d;
            asm volatile ("/* mixed modes */" : : "r" (result) : 
                         "r0", "r1", "r2", "r3", "r4", "r5", "memory");
            break;
    }
    
    /* Use result in loop to extend live range */
    for (int j = 0; j < 3; j++) {
        result += external_helper(result);
    }
    
    return result;
}

/* Main function that calls stress functions */
int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int total = 0;
    
    /* Call stress functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        total += stress_early_remat(seed + i);
        total += stress_mixed_modes(seed + i * 7);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

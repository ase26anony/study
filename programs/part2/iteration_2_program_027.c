/* early_remat_trigger.c
 * Program designed to trigger early rematerialization's privatize_cond_register_use
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int y = x * 2;
    return y + 1;
}

__attribute__((noinline)) float external_func2(float x) {
    volatile float y = x * 3.14f;
    return y - 1.0f;
}

__attribute__((noinline)) double external_func3(double x) {
    volatile double y = x / 2.71828;
    return y + 1.0;
}

/* Volatile function pointer to prevent inlining */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 7;
    volatile int v4 = seed - 3;
    volatile int v5 = seed * 3;
    volatile int v6 = seed / 2;
    
    volatile float f1 = seed * 1.1f;
    volatile float f2 = seed * 2.2f;
    volatile float f3 = seed * 3.3f;
    volatile float f4 = seed * 4.4f;
    
    volatile double d1 = seed * 1.111;
    volatile double d2 = seed * 2.222;
    volatile double d3 = seed * 3.333;
    volatile double d4 = seed * 4.444;
    
    volatile char c1 = seed & 0xFF;
    volatile short s1 = seed & 0xFFFF;
    volatile long long ll1 = (long long)seed * 1000000LL;
    volatile long long ll2 = (long long)seed * 2000000LL;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile float *p2 = &f1;
    volatile double *p3 = &d1;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Complex computation creating register pressure */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * v2 + v3;
        v2 = v2 - v4 * v5;
        v3 = v3 + v6 / (v1 + 1);
        
        f1 = f1 * f2 + f3;
        f2 = f2 - f4 * 0.5f;
        f3 = f3 + f1 / (f2 + 1.0f);
        
        d1 = d1 * d2 + d3;
        d2 = d2 - d4 * 0.25;
        d3 = d3 + d1 / (d2 + 1.0);
        
        /* Mix types to force mode conversions */
        ll1 = ll1 + (long long)v1;
        ll2 = ll2 + (long long)(f1 * 1000.0f);
        
        /* Use inline assembly to clobber registers */
        __asm__ volatile (
            "# Clobber many registers to increase pressure\n\t"
            "mov %0, %0\n\t"
            : "+r" (v1), "+r" (v2), "+r" (v3), "+r" (v4)
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    /* Compute key intermediate results */
    key_result = v1 * v2 + v3 * v4 - v5 * v6;
    float_result = f1 * f2 - f3 / f4;
    double_result = d1 + d2 * d3 - d4;
    
    /* Labels for goto with computed addresses */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = NULL;
    
    /* Complex nested conditional structure */
    volatile int control = seed % 10;
    
    /* Deeply nested if-else chain */
    if (control < 2) {
        /* Use key_result in conditional block with inline assembly */
        int temp = key_result;
        __asm__ volatile (
            "# Conditional use of key_result in mode SImode\n\t"
            "add %0, %0, #1\n\t"
            : "+r" (temp)
            : 
            : "cc"
        );
        key_result = temp;
        
        /* Call non-inlineable function */
        key_result = external_func1(key_result);
        
        volatile_label_ptr = labels[0];
    } else if (control < 4) {
        /* Different mode usage - float */
        float temp = float_result;
        __asm__ volatile (
            "# Conditional use of float_result in mode SFmode\n\t"
            "fadds %0, %0, %0\n\t"
            : "+w" (temp)
            : 
            : "cc"
        );
        float_result = temp;
        
        float_result = external_func2(float_result);
        
        volatile_label_ptr = labels[1];
    } else if (control < 6) {
        /* Different mode usage - double */
        double temp = double_result;
        __asm__ volatile (
            "# Conditional use of double_result in mode DFmode\n\t"
            "faddd %0, %0, %0\n\t"
            : "+w" (temp)
            : 
            : "cc"
        );
        double_result = temp;
        
        double_result = external_func3(double_result);
        
        volatile_label_ptr = labels[2];
    } else if (control < 8) {
        /* Mixed mode usage - char/short */
        char c_temp = c1;
        short s_temp = s1;
        
        __asm__ volatile (
            "# Mixed mode conditional use\n\t"
            "add %0, %0, #1\n\t"
            "add %1, %1, #1\n\t"
            : "+r" (c_temp), "+r" (s_temp)
            : 
            : "cc"
        );
        
        c1 = c_temp;
        s1 = s_temp;
        
        volatile_label_ptr = labels[3];
    } else {
        /* Long long mode usage */
        long long temp = ll1;
        __asm__ volatile (
            "# Conditional use of ll1 in mode DImode\n\t"
            "add %0, %0, #1\n\t"
            : "+r" (temp)
            : 
            : "cc"
        );
        ll1 = temp;
        
        volatile_label_ptr = labels[4];
    }
    
    /* Switch statement for additional complexity */
    switch (control) {
        case 0:
        case 1:
            /* Use key_result with different mode */
            {
                short temp = (short)key_result;
                __asm__ volatile (
                    "# Switch case use in HImode\n\t"
                    "add %0, %0, #1\n\t"
                    : "+r" (temp)
                    : 
                    : "cc"
                );
                key_result += temp;
            }
            break;
            
        case 2:
        case 3:
            /* Float mode in switch */
            {
                float temp = float_result;
                __asm__ volatile (
                    "# Switch case float use\n\t"
                    "fadds %0, %0, %1\n\t"
                    : "+w" (temp)
                    : "w" (1.0f)
                    : "cc"
                );
                float_result = temp;
            }
            break;
            
        case 4:
        case 5:
            /* Double mode in switch */
            {
                double temp = double_result;
                __asm__ volatile (
                    "# Switch case double use\n\t"
                    "faddd %0, %0, %1\n\t"
                    : "+w" (temp)
                    : "w" (1.0)
                    : "cc"
                );
                double_result = temp;
            }
            break;
            
        default:
            /* Mixed operations */
            key_result = key_result * 2 + (int)float_result;
            break;
    }
    
    /* Conditional goto using computed label address */
    if (volatile_label_ptr != NULL) {
        goto *volatile_label_ptr;
    }
    
label1:
    /* Use key_result after conditional block */
    key_result = key_result * 3 + v1;
    goto after_labels;
    
label2:
    /* Different computation path */
    float_result = float_result * 2.0f + f1;
    key_result += (int)float_result;
    goto after_labels;
    
label3:
    /* Another path */
    double_result = double_result * 1.5 + d1;
    key_result += (int)double_result;
    goto after_labels;
    
label4:
    /* Mixed type path */
    key_result += c1 + s1;
    goto after_labels;
    
label5:
    /* Long long path */
    key_result += (int)(ll1 & 0xFFFFFFFF);
    goto after_labels;
    
after_labels:
    
    /* Final computation using all results */
    int final_result = key_result;
    final_result += (int)float_result;
    final_result += (int)double_result;
    final_result += c1;
    final_result += s1;
    final_result += (int)(ll1 & 0xFFFFFFFF);
    final_result += (int)(ll2 & 0xFFFFFFFF);
    
    /* More register pressure */
    for (int i = 0; i < 5; i++) {
        v1 = v1 ^ v2;
        v2 = v2 | v3;
        v3 = v3 & v4;
        v4 = v4 + v5;
        v5 = v5 - v6;
        
        /* Inline assembly with many clobbers */
        __asm__ volatile (
            "# Final clobbering\n\t"
            "mov %0, %0\n\t"
            "mov %1, %1\n\t"
            : "+r" (v1), "+r" (v2), "+r" (v3), "+r" (v4), "+r" (v5)
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    return final_result;
}

/* Another complex function with different patterns */
__attribute__((noinline, optimize("no-tree-vectorize")))
int complex_control_flow(int base) {
    volatile int a = base;
    volatile int b = base * 2;
    volatile int c = base * 3;
    volatile int d = base * 4;
    
    /* Result that will be used conditionally */
    int conditional_result = a + b + c + d;
    
    /* Very deep nesting */
    if (a > 0) {
        if (b > 10) {
            if (c > 20) {
                if (d > 30) {
                    /* Use conditional_result with inline asm */
                    __asm__ volatile (
                        "# Deep nested use\n\t"
                        "add %0, %0, #100\n\t"
                        : "+r" (conditional_result)
                        : 
                        : "cc"
                    );
                    
                    /* Function call barrier */
                    conditional_result = external_func1(conditional_result);
                } else {
                    __asm__ volatile (
                        "# Else path use\n\t"
                        "sub %0, %0, #50\n\t"
                        : "+r" (conditional_result)
                        : 
                        : "cc"
                    );
                }
            } else {
                volatile int temp = conditional_result;
                __asm__ volatile (
                    "# Another nested level\n\t"
                    "mul %0, %0, %1\n\t"
                    : "+r" (temp)
                    : "r" (2)
                    : "cc"
                );
                conditional_result = temp;
            }
        }
    }
    
    /* Use result after all conditionals */
    return conditional_result * 2;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing early rematerialization trigger...\n");
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += stress_function(seed + i);
        total += complex_control_flow(seed + i * 2);
    }
    
    printf("Result: %d\n", total);
    printf("If you see this, the program completed without crashing.\n");
    
    return total != 0 ? 0 : 1;
}

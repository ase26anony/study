/* early_remat_trigger.c
 * Program designed to trigger early rematerialization's privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early_remat_trigger.c -o early_remat_trigger
 * Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat early_remat_trigger.c -o early_remat_trigger_32
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
    volatile double temp = a * b;
    return temp / (temp + 1.0);
}

/* Function pointer with unknown target */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = external_helper;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    long long v5 = seed * 1000LL;
    long long v6 = seed * 2000LL;
    float f1 = seed * 0.5f;
    float f2 = seed * 1.5f;
    double d1 = seed * 0.25;
    double d2 = seed * 0.75;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    unsigned int u1 = seed * 3;
    unsigned long long u2 = seed * 5ULL;
    
    /* Pointer variables */
    int *p1 = &v2;
    int *p2 = &v3;
    float *fp1 = &f1;
    double *dp1 = &d1;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double fp_key_result = 0.0;
    
    /* Complex computation with mixed types */
    key_result = v1 + v2 + v3 + v4;
    fp_key_result = d1 + d2 + f1 + f2;
    
    /* Create label addresses for goto */
    void *labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void *volatile_label_ptr = labels[seed % 5];
    
    /* First conditional block - deeply nested */
    if (v1 > 100) {
        /* Use inline assembly to clobber many registers */
        asm volatile (
            "# Clobber many registers\n"
            "mov %0, %0\n"
            :
            : "r" (key_result)
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        if (v2 < 200) {
            /* Nested conditional with function call */
            int temp = volatile_func_ptr(key_result, v3);
            
            if (temp > 50) {
                /* Use key_result in complex computation */
                for (int i = 0; i < 10; i++) {
                    key_result += external_helper(i, v4);
                }
                
                /* Switch statement inside nested if */
                switch (key_result % 4) {
                    case 0:
                        key_result += v5;
                        /* Use key_result in inline asm with different mode */
                        asm volatile (
                            "add %0, %0, #1\n"
                            : "+r" (key_result)
                            :
                            : "cc"
                        );
                        break;
                    case 1:
                        key_result += v6;
                        break;
                    case 2:
                        key_result *= 2;
                        /* Another inline asm use */
                        asm volatile (
                            "mul %0, %0, %1\n"
                            : "+r" (key_result)
                            : "r" (v3)
                            : "cc"
                        );
                        break;
                    case 3:
                        key_result /= 2;
                        break;
                }
            }
        }
        
        /* Use goto with computed label */
        if (v1 & 1) {
            goto *volatile_label_ptr;
        }
    }
    
label1:
    /* Second conditional path */
    if (v2 > 150) {
        /* Use fp_key_result with double mode */
        fp_key_result = fp_helper(fp_key_result, d1);
        
        /* Mixed type computation */
        key_result += (int)fp_key_result;
        
        /* Inline asm with floating point */
        asm volatile (
            "# Floating point operation\n"
            "fadd d0, d0, d1\n"
            :
            : 
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8",
              "d9", "d10", "d11", "d12", "d13", "d14", "d15", "d16"
        );
    }
    
label2:
    /* Loop with conditional use of key_result */
    for (int i = 0; i < 20; i++) {
        if (i & 1) {
            /* Conditional use inside loop */
            key_result += i * v3;
            
            /* Use different data types */
            s1 += (short)key_result;
            c1 += (char)(key_result & 0xFF);
        } else {
            key_result -= i * v4;
        }
        
        /* More register pressure */
        u1 += u2;
        f1 = f2 * 1.1f;
        d1 = d2 * 1.01;
    }
    
label3:
    /* Another deeply nested conditional */
    if (v3 < 300) {
        if (v4 > 0) {
            if (f1 > 0.0f) {
                /* Use key_result with char mode */
                char temp_char = (char)key_result;
                asm volatile (
                    "and %0, %0, #0xFF\n"
                    : "+r" (temp_char)
                    :
                    : "cc"
                );
                key_result = temp_char + v1;
            }
        }
    }
    
label4:
    /* Switch statement with multiple cases */
    switch (key_result % 6) {
        case 0:
            key_result += *p1 + *p2;
            break;
        case 1:
            key_result += (int)(*fp1 * 100.0f);
            break;
        case 2:
            key_result += (int)(*dp1 * 50.0);
            break;
        case 3:
            /* Function call that uses key_result */
            key_result = external_helper(key_result, v2);
            break;
        case 4:
            /* Complex computation with many variables */
            key_result = v1 + v2 + v3 + v4 + (int)v5 + (int)v6;
            key_result += (int)f1 + (int)f2 + (int)d1 + (int)d2;
            break;
        case 5:
            /* Use all variables to keep them live */
            key_result = v1 * v2 - v3 + v4;
            key_result += (int)(u1 * u2);
            key_result += s1 + c1;
            break;
    }
    
label5:
    /* Final use of key_result outside all conditionals */
    int final_result = key_result;
    
    /* Use all variables one more time to prevent elimination */
    final_result += v1 + v2 + v3 + v4;
    final_result += (int)v5 + (int)v6;
    final_result += (int)f1 + (int)f2 + (int)d1 + (int)d2;
    final_result += s1 + c1 + u1 + (int)u2;
    final_result += *p1 + *p2;
    
    return final_result;
}

/* Main function with multiple calls */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Alternate function pointer target */
        if (i % 3 == 0) {
            volatile_func_ptr = external_helper;
        }
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different data type sizes */
    {
        /* Test with 8-bit mode */
        volatile char c = 100;
        char result = c * 2;
        
        /* Test with 16-bit mode */
        volatile short s = 1000;
        short s_result = s / 2;
        
        /* Test with 64-bit mode */
        volatile long long ll = 1000000LL;
        long long ll_result = ll * 3;
        
        /* Mix them in computation */
        total += result + s_result + (int)ll_result;
    }
    
    return total > 0 ? 0 : 1;
}

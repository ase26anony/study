/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile char v1 = seed & 0xFF;
    volatile short v2 = seed * 2;
    volatile int v3 = seed + 1000;
    volatile long long v4 = (long long)seed * 3000;
    volatile float v5 = seed * 1.5f;
    volatile double v6 = seed * 2.71828;
    int *volatile ptr1 = (int*)&v3;
    float *volatile ptr2 = (float*)&v5;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    long long ll_result = 0;
    double fp_result = 0.0;
    
    /* Labels for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label_end };
    volatile void* volatile_label_ptr = labels[0];
    
    /* More variables to increase pressure */
    int a1 = seed, a2 = seed*2, a3 = seed*3, a4 = seed*4;
    int b1 = seed+1, b2 = seed+2, b3 = seed+3, b4 = seed+4;
    float f1 = seed*1.1f, f2 = seed*1.2f, f3 = seed*1.3f;
    double d1 = seed*1.01, d2 = seed*1.02, d3 = seed*1.03;
    
    /* Complex computation creating key_result */
    for (int i = 0; i < 4; i++) {
        key_result += (a1 * b1 + a2 * b2 - a3 * b3) >> (i + 1);
        ll_result += (v4 * i) / (seed + 1);
        fp_result += f1 * d1 + f2 * d2;
        
        /* Use inline assembly to clobber registers */
        __asm__ volatile (
            "# Clobber many registers\n"
            :
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory"
        );
    }
    
    /* Volatile control variable */
    volatile int control = seed % 8;
    
    /* Deeply nested conditional blocks */
    if (control > 0) {
        if (control > 2) {
            switch (control) {
                case 3:
                case 4: {
                    /* Inside nested block, use key_result in complex way */
                    int temp = key_result;
                    
                    /* Use different modes/types */
                    short s_temp = (short)temp;
                    char c_temp = (char)(temp & 0xFF);
                    float f_temp = (float)temp;
                    
                    /* Call non-inlineable function with key_result */
                    temp = external_func(temp);
                    
                    /* Inline assembly using the value - this creates a use
                       that might need privatization */
                    __asm__ volatile (
                        "# Conditional use of key_result\n"
                        "add %0, %0, #1\n"
                        : "+r" (temp)
                        :
                        : "cc", "memory"
                    );
                    
                    /* More mixed-type operations */
                    ll_result += (long long)temp * s_temp;
                    fp_result += (double)f_temp * c_temp;
                    
                    /* Jump to label via volatile pointer */
                    if (v1 > 100) {
                        goto *volatile_label_ptr;
                    }
                    break;
                }
                case 5:
                case 6: {
                    /* Alternative path with different register usage */
                    double d_temp = fp_result;
                    __asm__ volatile (
                        "# FP register pressure\n"
                        "fadd d0, d0, d0\n"
                        :
                        : 
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
                    );
                    fp_result = d_temp * 2.0;
                    break;
                }
                default: {
                    /* Use key_result in another way */
                    long long ll_temp = ll_result + key_result;
                    __asm__ volatile (
                        "# Mixed mode use\n"
                        :
                        : "r" (key_result), "r" ((int)ll_temp)
                        : "memory"
                    );
                }
            }
            
            /* Another level of nesting */
            if (v2 & 0x10) {
                volatile int inner_control = v3 % 3;
                for (int j = 0; j < inner_control; j++) {
                    /* Use key_result inside loop with function call */
                    key_result = external_func(key_result + j);
                    
                    /* More register pressure */
                    __asm__ volatile (
                        "# Inner loop clobber\n"
                        :
                        :
                        : "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                }
            }
        } else {
            /* Else path still uses key_result */
            key_result = key_result * 2 - v3;
        }
        
        /* Function pointer call creating opaque control flow */
        if (volatile_func_ptr) {
            volatile_func_ptr();
        }
    }
    
label1:
    /* Use key_result after conditional block with different mode */
    short s_result = (short)key_result;
    __asm__ volatile (
        "# Post-conditional use (short mode)\n"
        "add %0, %0, #5\n"
        : "+r" (s_result)
        :
        : "cc"
    );
    key_result += s_result;
    
label2:
    /* More mixed-type operations */
    float float_conv = (float)key_result;
    __asm__ volatile (
        "# Float conversion use\n"
        "fcvt s0, %w0\n"
        : 
        : "r" (key_result)
        : "s0", "s1", "s2"
    );
    
label3:
    /* Use key_result in 64-bit context */
    long long big_calc = (long long)key_result * ll_result;
    __asm__ volatile (
        "# 64-bit use\n"
        "smull x0, w0, w1\n"
        : 
        : "r" (key_result), "r" ((int)ll_result)
        : "x0", "x1"
    );
    
label4:
    /* Final aggregation preventing elimination */
    int final_result = key_result + 
                      (int)ll_result + 
                      (int)fp_result + 
                      v1 + v2 + v3 + 
                      (int)v4 + 
                      (int)v5 + 
                      (int)v6;
    
    /* One more volatile jump */
    if (v1 < 50) {
        volatile_label_ptr = labels[4];
        goto *volatile_label_ptr;
    }
    
label_end:
    return final_result + a1 + a2 + a3 + a4 + b1 + b2 + b3 + b4;
}

/* Main function with different compilation contexts */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += stress_function(seed + i);
        
        /* Alternate function pointer target */
        if (i % 3 == 0) {
            volatile_func_ptr = (void(*)())stress_function;
        } else {
            volatile_func_ptr = 0;
        }
    }
    
    /* Use result to prevent elimination */
    volatile int output = total;
    return output % 256;
}

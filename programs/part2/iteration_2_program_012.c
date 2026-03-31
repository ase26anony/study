/* early-remat-test.c
 * Test program to trigger early rematerialization's privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_helper(int x) {
    volatile int dummy = x;
    return dummy * 2;
}

/* Another non-inlineable function */
__attribute__((noinline)) double external_double_helper(double x) {
    volatile double dummy = x;
    return dummy * 3.14159;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    short v3 = (short)(seed + 2);
    char v4 = (char)(seed + 3);
    long long v5 = (long long)seed * 100;
    float v6 = (float)seed * 1.5f;
    double v7 = (double)seed * 2.5;
    int *v8 = &v1;
    float v9 = v6 * 2.0f;
    double v10 = v7 * 3.0;
    int v11 = v2 * 3;
    short v12 = v3 * 4;
    char v13 = v4 * 5;
    long long v14 = v5 * 6;
    float v15 = v9 * 1.1f;
    double v16 = v10 * 1.2;
    
    /* More variables for additional pressure */
    int v17 = 0, v18 = 0, v19 = 0, v20 = 0;
    float v21 = 0.0f, v22 = 0.0f;
    double v23 = 0.0, v24 = 0.0;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double double_key = 0.0;
    
    /* Complex conditional computation */
    if (v1 > 0) {
        /* Nested if-else chain */
        if (v2 % 2 == 0) {
            key_result = v2 * v11;
            double_key = (double)key_result * 1.234;
            
            /* Use inline assembly to clobber registers */
            __asm__ volatile (
                "# Clobber many registers\n"
                "mov %0, %%eax\n"
                "mov %1, %%ebx\n"
                "mov %2, %%ecx\n"
                "mov %3, %%edx\n"
                "mov %4, %%esi\n"
                "mov %5, %%edi\n"
                :
                : "r" (v1), "r" (v2), "r" (v3), "r" (v4), "r" (v5), "r" (v6)
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
            
            /* Use key_result in conditional block */
            v17 = external_helper(key_result);
            v21 = (float)key_result * 0.5f;
        } else {
            key_result = v3 * v12;
            double_key = (double)key_result * 2.345;
            
            /* Different register clobbering pattern */
            __asm__ volatile (
                "# Clobber more registers\n"
                "mov %0, %%r8d\n"
                "mov %1, %%r9d\n"
                "mov %2, %%r10d\n"
                "mov %3, %%r11d\n"
                :
                : "r" (v7), "r" (v8), "r" (v9), "r" (v10)
                : "r8", "r9", "r10", "r11", "memory"
            );
            
            v18 = external_helper(key_result + 1);
            v22 = (float)key_result * 0.75f;
        }
        
        /* Switch statement for additional complexity */
        switch (v4 % 4) {
            case 0:
                key_result += v13 * 7;
                double_key += external_double_helper(double_key);
                break;
            case 1:
                key_result += v14 % 100;
                double_key *= 1.1;
                break;
            case 2:
                key_result -= v15 * 2;
                double_key /= 2.0;
                break;
            case 3:
                key_result ^= 0xABCD;
                double_key = -double_key;
                break;
        }
    } else {
        /* Alternative path with different computations */
        key_result = v5 % 1000;
        double_key = (double)v16 * 0.987;
        
        /* More register pressure */
        __asm__ volatile (
            "# Generic clobber\n"
            :
            :
            : "memory"
        );
        
        v19 = external_helper(key_result * 2);
        v23 = double_key * 2.0;
    }
    
    /* Use volatile variables in control flow */
    volatile int control_var = v1 * v2;
    volatile double double_control = v7 * v10;
    
    /* Complex nested conditionals with mixed types */
    if (control_var > 100) {
        if (double_control > 50.0) {
            key_result += (int)(double_key * 10);
            v20 = key_result * 3;
            
            /* Use key_result in inline asm with specific mode requirements */
            __asm__ volatile (
                "# Use key_result with specific mode\n"
                "addl %1, %0\n"
                : "+r" (key_result)
                : "ri" (v20)
                : "cc"
            );
        } else {
            key_result -= (int)(double_key * 5);
            v24 = double_key * 3.0;
            
            /* Different mode usage */
            __asm__ volatile (
                "# Use double_key\n"
                "fldl %1\n"
                "fstpl %0\n"
                : "=m" (v24)
                : "m" (double_key)
                : "st", "st(1)"
            );
        }
        
        /* Deep nesting */
        for (int i = 0; i < 3; i++) {
            volatile int loop_control = i * key_result;
            if (loop_control % 2 == 0) {
                key_result += v17 + v18;
                
                /* Mixed mode operations */
                short temp_short = (short)key_result;
                __asm__ volatile (
                    "# Use short mode\n"
                    "movw %w1, %w0\n"
                    : "=r" (temp_short)
                    : "r" (temp_short)
                );
                key_result = temp_short * 2;
            }
        }
    }
    
    /* Label addresses for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile int label_index = control_var % 4;
    
    /* Conditional goto using computed label */
    goto *labels[label_index];
    
label1:
    key_result += 1000;
    /* Use key_result after label */
    v17 = external_helper(key_result);
    goto after_labels;
    
label2:
    key_result += 2000;
    /* Different use pattern */
    __asm__ volatile (
        "# Use in different context\n"
        "imull %1, %0\n"
        : "+r" (key_result)
        : "r" (v19)
        : "cc"
    );
    goto after_labels;
    
label3:
    key_result += 3000;
    /* Use with floating point */
    v21 = (float)key_result * 0.333f;
    goto after_labels;
    
label4:
    key_result += 4000;
    /* Complex use with multiple modes */
    char temp_char = (char)(key_result & 0xFF);
    __asm__ volatile (
        "# Char mode operation\n"
        "movb %b1, %b0\n"
        : "=r" (temp_char)
        : "r" (temp_char)
    );
    key_result = (key_result & ~0xFF) | temp_char;
    goto after_labels;
    
after_labels:
    
    /* Final aggregation to prevent elimination */
    int final_result = key_result + v17 + v18 + v19 + v20;
    final_result += (int)v21 + (int)v22 + (int)v23 + (int)v24;
    final_result += (int)(double_key * 100);
    
    /* Use all variables one more time */
    __asm__ volatile (
        "# Final use of all variables\n"
        "addl %1, %0\n"
        "addl %2, %0\n"
        "addl %3, %0\n"
        : "+r" (final_result)
        : "r" (v1), "r" (v2), "r" (v3)
        : "cc"
    );
    
    return final_result;
}

/* Another function with different mode patterns */
__attribute__((noinline, optimize("no-tree-vectorize")))
long long mixed_mode_function(int base) {
    volatile char c1 = (char)base;
    volatile short s1 = (short)(base * 2);
    volatile int i1 = base * 3;
    volatile long long ll1 = (long long)base * 4;
    volatile float f1 = (float)base * 1.5f;
    volatile double d1 = (double)base * 2.5;
    
    long long result = 0;
    
    /* Complex conditional with mixed mode operations */
    if (c1 > 0) {
        result = (long long)s1 * i1;
        
        /* Use with different modes in conditional block */
        __asm__ volatile (
            "# Mixed mode operations\n"
            "movsx %w1, %k0\n"
            "imull %2, %k0\n"
            : "=r" (i1)
            : "r" (c1), "r" (s1)
            : "cc"
        );
        
        result += i1;
    } else {
        result = ll1 / 2;
        
        /* Floating point operations */
        __asm__ volatile (
            "# Double mode\n"
            "fldl %1\n"
            "fmull %2\n"
            "fstpl %0\n"
            : "=m" (d1)
            : "m" (d1), "m" (f1)
            : "st", "st(1)"
        );
        
        result += (long long)d1;
    }
    
    /* Switch with mode variations */
    switch (base % 3) {
        case 0:
            /* Char mode */
            __asm__ volatile (
                "movb %b1, %b0\n"
                : "=r" (c1)
                : "r" (result)
            );
            result = c1;
            break;
        case 1:
            /* Short mode */
            __asm__ volatile (
                "movw %w1, %w0\n"
                : "=r" (s1)
                : "r" (result)
            );
            result = s1;
            break;
        case 2:
            /* Int mode */
            result = i1 * 2;
            break;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
        total += mixed_mode_function(seed + i) % 1000;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

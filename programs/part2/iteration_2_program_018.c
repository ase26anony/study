/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Alternative: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x, int y) {
    volatile int result = x ^ y;
    return result;
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = &external_func;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile char v1 = seed & 0xFF;
    volatile short v2 = seed >> 8;
    volatile int v3 = seed * 3;
    volatile long long v4 = (long long)seed * 7;
    volatile float v5 = (float)seed * 1.5f;
    volatile double v6 = (double)seed * 2.5;
    volatile int* v7 = (int*)&seed;
    
    /* Additional variables for more pressure */
    int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    float f1 = v5 + 1.0f, f2 = v5 + 2.0f, f3 = v5 + 3.0f;
    double d1 = v6 + 1.0, d2 = v6 + 2.0, d3 = v6 + 3.0;
    long long l1 = v4 + 1, l2 = v4 + 2, l3 = v4 + 3;
    
    /* Key intermediate result with mixed-type computation */
    int key_result = 0;
    key_result = v3 + (int)v4 + (int)v5 + (int)v6;
    
    /* Complex conditional structure with deeply nested blocks */
    volatile int control = seed % 7;
    
    /* Label addresses for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, 
                      &&label5, &&label6, &&label7 };
    volatile void* volatile_label_ptr = labels[control % 7];
    
    /* First conditional block - uses key_result conditionally */
    if (control > 0) {
        /* Nested if-else chain */
        if (control & 1) {
            /* Use key_result in inline asm with many clobbers */
            asm volatile (
                "add %0, %0, #1\n\t"
                : "+r" (key_result)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory", "cc"
            );
            
            /* Call through volatile function pointer */
            key_result = volatile_func_ptr(key_result, v3);
        } else if (control & 2) {
            /* Different computation path */
            key_result *= 2;
            
            /* More register pressure */
            asm volatile (
                "mov %0, %0, lsl #2\n\t"
                : "+r" (key_result)
                :
                : "r0", "r1", "r2", "memory"
            );
        }
        
        /* Switch inside conditional block */
        switch (control % 4) {
            case 0:
                key_result += v1;
                /* Fall through */
            case 1:
                key_result += v2;
                /* Use mixed types */
                key_result += (int)(f1 * 2.0f);
                break;
            case 2:
                key_result += (int)d1;
                /* Another asm with clobbers */
                asm volatile (
                    "sub %0, %0, #5\n\t"
                    : "+r" (key_result)
                    :
                    : "r0", "r1", "r2", "r3", "memory"
                );
                break;
            case 3:
                /* Complex computation using all variables */
                key_result = key_result * a1 + a2 - a3 + (int)l1;
                break;
        }
        
        /* Goto-based control flow */
        goto *volatile_label_ptr;
    }
    
label1:
    /* Second conditional block with different mode usage */
    if (control < 4) {
        /* Use key_result with float conversion */
        float temp_f = (float)key_result;
        asm volatile (
            "fcvt s0, %w0\n\t"
            "fcvt %w0, s0\n\t"
            : "+r" (key_result)
            : 
            : "s0", "s1", "s2", "s3", "s4", "s5", "memory"
        );
        
        /* Nested loop to extend live ranges */
        for (int i = 0; i < 3; i++) {
            key_result += i * v3;
            if (i == 1) {
                /* Conditional use inside loop */
                short temp_s = (short)key_result;
                key_result = temp_s * 2;
            }
        }
    }
    
label2:
    /* Third conditional path */
    if (control % 3 == 0) {
        /* Use key_result with double precision */
        double temp_d = (double)key_result;
        asm volatile (
            "fcvt d0, %w0\n\t"
            "fcvt %w0, d0\n\t"
            : "+r" (key_result)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "memory"
        );
        
        /* Call external function */
        key_result = external_func(key_result, v3);
    }
    
label3:
    /* Use all variables to keep them live */
    int sum = key_result + a1 + a2 + a3 + a4;
    sum += (int)f1 + (int)f2 + (int)f3;
    sum += (int)d1 + (int)d2 + (int)d3;
    sum += (int)l1 + (int)l2 + (int)l3;
    
    /* More conditional blocks */
    switch (seed % 5) {
        case 0:
            sum += v1 * 2;
            break;
        case 1:
            sum += v2 * 3;
            /* Another asm with many clobbers */
            asm volatile (
                "and %0, %0, #0xFF\n\t"
                : "+r" (sum)
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "memory", "cc"
            );
            break;
        case 2:
            sum += v3 / 2;
            break;
        case 3:
            sum += (int)v4;
            break;
        case 4:
            sum += (int)v5 + (int)v6;
            break;
    }
    
label4:
    /* Final computation using key_result again */
    int final_result = key_result * 2 + sum;
    
    /* Ensure all variables are used */
    final_result += *v7;
    
    return final_result;

label5:
    /* Alternative path */
    key_result -= 100;
    goto label3;

label6:
    /* Another alternative path */
    key_result += 200;
    goto label2;

label7:
    /* Yet another path */
    key_result *= 3;
    goto label1;
}

/* Main function to call stress function multiple times */
int main(int argc, char** argv) {
    int total = 0;
    
    /* Call with different seeds to exercise different paths */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
    }
    
    /* Use result to prevent elimination */
    volatile int result = total;
    
    return result % 256;
}

/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int y = x;
    return y * 2;
}

/* Function pointer with unknown target */
int (*volatile func_ptr)(int) = external_func;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    volatile short v2 = seed * 2;
    volatile char v3 = seed + 1;
    volatile float v4 = seed * 3.14f;
    volatile double v5 = seed * 2.71828;
    volatile long long v6 = seed * 1000LL;
    volatile int* v7 = (int*)&seed;
    
    /* Additional variables for more pressure */
    int a1 = v1 + 1, a2 = v1 * 2, a3 = v1 / 3, a4 = v1 - 4;
    float f1 = v4 * 2.0f, f2 = v4 / 3.0f, f3 = v4 + 1.0f;
    double d1 = v5 * 2.0, d2 = v5 / 3.0, d3 = v5 - 1.0;
    long long ll1 = v6 + 100, ll2 = v6 * 2, ll3 = v6 / 3;
    
    /* Key intermediate result - this is what we want to rematerialize */
    int key_result = v1 * v2 * v3;
    key_result += external_func(key_result);
    
    /* Complex nested conditional structure */
    volatile int control = seed % 7;
    
    /* Label addresses for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[control % 5];
    
    /* First conditional block */
    if (control > 0) {
        if (control < 3) {
            /* Use key_result in conditional block with inline assembly */
            int temp = key_result;
            asm volatile (
                "mov %0, %0\n\t"
                "add %0, #1\n\t"
                : "+r" (temp)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "memory"
            );
            key_result = temp;
            
            /* Nested switch inside if */
            switch (control) {
                case 1:
                    key_result += func_ptr(key_result);
                    break;
                case 2:
                    key_result *= 2;
                    /* Another level of nesting */
                    if (v4 > 0.0f) {
                        asm volatile ("" : : "r" (key_result) : "memory");
                        key_result += (int)v4;
                    }
                    break;
                default:
                    goto *volatile_label_ptr;
            }
        } else if (control < 5) {
            /* Different mode usage - promote to long long */
            long long promoted = key_result;
            asm volatile (
                "mov %0, %0\n\t"
                : "+r" (promoted)
                :
                : "r0", "r1", "r2", "r3", "memory"
            );
            key_result = (int)promoted + v3;
            
            /* Call through function pointer */
            key_result = func_ptr(key_result);
        } else {
            /* Use float mode */
            float float_version = (float)key_result;
            asm volatile (
                "fmov s0, %0\n\t"
                "fadd s0, s0, s0\n\t"
                "fmov %0, s0\n\t"
                : "+w" (float_version)
                :
                : "s0", "s1", "s2", "s3", "memory"
            );
            key_result = (int)float_version;
        }
        
        label1:
        key_result += a1;
    } else {
        label2:
        key_result -= a2;
    }
    
    /* Irreducible control flow using computed goto */
    if (seed % 11 == 0) {
        goto *volatile_label_ptr;
    }
    
    label3:
    /* Use key_result again after conditional blocks */
    int final_compute = key_result;
    
    /* More register pressure */
    for (int i = 0; i < 10; i++) {
        a1 += v2;
        a2 *= v3;
        f1 += v4;
        d1 -= v5;
        ll1 ^= v6;
        
        /* Mix types to force different modes */
        switch (i % 4) {
            case 0:
                final_compute += (int)f1;
                break;
            case 1:
                final_compute += (int)d1;
                break;
            case 2:
                final_compute += (int)ll1;
                break;
            case 3:
                final_compute += a1 * a2;
                break;
        }
    }
    
    label4:
    /* Another conditional use with different mode */
    if (v4 > 10.0f) {
        double double_result = (double)final_compute;
        asm volatile (
            "fmov d0, %0\n\t"
            "fmul d0, d0, d0\n\t"
            "fmov %0, d0\n\t"
            : "+w" (double_result)
            :
            : "d0", "d1", "d2", "d3", "memory"
        );
        final_compute = (int)double_result;
    }
    
    label5:
    /* Final aggregation to prevent elimination */
    return final_compute + a1 + a2 + a3 + a4 + 
           (int)f1 + (int)f2 + (int)f3 + 
           (int)d1 + (int)d2 + (int)d3 + 
           (int)ll1 + (int)ll2 + (int)ll3;
}

/* Main function with varying inputs */
int main() {
    int total = 0;
    
    /* Call stress function with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Alternate function pointer target */
        if (i % 3 == 0) {
            func_ptr = &external_func;
        } else if (i % 3 == 1) {
            /* Different function with same signature */
            func_ptr = (int (*)(int))stress_function;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output = total;
    return output % 256;
}

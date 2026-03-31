/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing -m32 -march=i686 -fno-dse -c test.c */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with unknown target */
int (*volatile func_ptr)(int) = external_func;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    char v2 = seed & 0xFF;
    short v3 = seed * 2;
    int v4 = seed + 1000;
    long long v5 = seed * 100LL;
    float v6 = seed * 0.5f;
    double v7 = seed * 0.25;
    int *v8 = (int*)&v1;
    float v9 = v6 * 2.0f;
    double v10 = v7 / 2.0;
    int v11 = v4 ^ 0x1234;
    char v12 = v2 + 1;
    short v13 = v3 - 50;
    long long v14 = v5 + 999999LL;
    float v15 = v9 + 3.14f;
    double v16 = v10 * 1.618;
    
    /* Key intermediate result with mixed-type computation */
    int key_result = v1 + v2 + v3 + v4 + (int)v5 + (int)v6 + (int)v7;
    key_result += *v8 + (int)v9 + (int)v10 + v11 + v12 + v13 + (int)v14;
    
    /* Complex conditional structure with deeply nested blocks */
    volatile int cond1 = key_result & 1;
    volatile int cond2 = (key_result >> 1) & 1;
    volatile int cond3 = (key_result >> 2) & 1;
    
    /* Label addresses for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* First conditional use of key_result */
    if (cond1) {
        if (cond2) {
            switch (seed % 4) {
                case 0:
                    /* Use key_result in inline assembly with many clobbers */
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        "mov r0, %0\n\t"
                        "mov r1, %0\n\t"
                        "mov r2, %0\n\t"
                        "mov r3, %0\n\t"
                        "mov r4, %0\n\t"
                        "mov r5, %0\n\t"
                        "mov r6, %0\n\t"
                        "mov r7, %0\n\t"
                        "mov r8, %0\n\t"
                        "mov r9, %0\n\t"
                        "mov r10, %0\n\t"
                        "mov r11, %0\n\t"
                        "mov r12, %0\n\t"
                        : 
                        : "r" (key_result)
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    key_result = func_ptr(key_result);
                    break;
                case 1:
                    /* Different mode usage - double */
                    double temp_d = (double)key_result;
                    asm volatile (
                        "vmov.f64 d0, %0\n\t"
                        "vmov.f64 d1, d0\n\t"
                        "vmov.f64 d2, d0\n\t"
                        "vmov.f64 d3, d0\n\t"
                        "vmov.f64 d4, d0\n\t"
                        "vmov.f64 d5, d0\n\t"
                        "vmov.f64 d6, d0\n\t"
                        "vmov.f64 d7, d0\n\t"
                        : 
                        : "w" (temp_d)
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "memory"
                    );
                    break;
                case 2:
                    /* Float mode usage */
                    float temp_f = (float)key_result;
                    asm volatile (
                        "vmov.f32 s0, %0\n\t"
                        "vmov.f32 s1, s0\n\t"
                        "vmov.f32 s2, s0\n\t"
                        "vmov.f32 s3, s0\n\t"
                        "vmov.f32 s4, s0\n\t"
                        "vmov.f32 s5, s0\n\t"
                        "vmov.f32 s6, s0\n\t"
                        "vmov.f32 s7, s0\n\t"
                        : 
                        : "t" (temp_f)
                        : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "memory"
                    );
                    break;
                case 3:
                    /* Long long mode usage */
                    long long temp_ll = (long long)key_result;
                    asm volatile (
                        "mov r0, %0\n\t"
                        "mov r1, %1\n\t"
                        "mov r2, r0\n\t"
                        "mov r3, r1\n\t"
                        : 
                        : "r" ((int)(temp_ll >> 32)), "r" ((int)temp_ll)
                        : "r0", "r1", "r2", "r3", "memory"
                    );
                    break;
            }
            
            /* Nested conditional with goto */
            if (cond3) {
                goto *volatile_label_ptr;
            }
        } else {
            /* Another conditional path using key_result */
            volatile int temp = key_result;
            for (int i = 0; i < 3; i++) {
                key_result += temp * i;
                asm volatile ("" : : "r" (key_result) : "memory");
            }
        }
    } else {
        /* Alternative path with different computation */
        key_result -= v11 * 2;
    }
    
label1:
    /* Use key_result after conditional block with mixed types */
    v6 += (float)key_result;
    v7 += (double)key_result;
    
    /* More complex control flow with switch */
    switch (key_result % 5) {
        case 0:
            v4 = key_result | 0xABCD;
            break;
        case 1:
            v5 = (long long)key_result * v5;
            break;
        case 2:
            v9 = (float)key_result / 3.0f;
            /* Jump to another label */
            goto *labels[2];
            break;
        case 3:
            v10 = (double)key_result * 2.71828;
            break;
        case 4:
            v14 = (long long)key_result << 3;
            break;
    }
    
label2:
    /* Second use of key_result with different mode */
    short key_short = (short)key_result;
    asm volatile (
        "movw %0, %1\n\t"
        "add %0, %0, #100\n\t"
        : "=r" (key_short)
        : "r" (key_short)
        : "cc", "memory"
    );
    
label3:
    /* More register pressure */
    int sum = v1 + v2 + v3 + v4 + (int)v5;
    sum += (int)v6 + (int)v7 + *v8 + (int)v9 + (int)v10;
    sum += v11 + v12 + v13 + (int)v14 + (int)v15 + (int)v16;
    
    /* Final computation using key_result */
    int final_result = sum + key_result + key_short;
    
    /* Irreducible loop with goto */
    volatile int loop_counter = 2;
loop_start:
    if (loop_counter-- > 0) {
        final_result += external_func(key_result);
        goto loop_start;
    }
    
label4:
    return final_result;

label5:
    /* Alternative return path */
    return key_result ^ 0xDEADBEEF;
}

/* Main function to call stress function */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        result += stress_function(argc + i * 17);
    }
    
    /* Prevent dead code elimination */
    volatile int output = result;
    return output;
}

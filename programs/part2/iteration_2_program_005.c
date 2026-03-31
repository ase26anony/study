/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Alternative: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x, int y) {
    volatile int result = x * y;
    return result + 1;
}

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_fptr)(void);

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    float v3 = seed * 3.14f;
    double v4 = seed * 2.71828;
    char v5 = seed & 0xFF;
    short v6 = seed * 4;
    long long v7 = seed * 1000LL;
    int *v8 = &v2;
    float v9 = v3 * 2.0f;
    double v10 = v4 / 2.0;
    int v11 = v1 + v2;
    float v12 = v3 + v9;
    double v13 = v4 + v10;
    char v14 = v5 + 1;
    short v15 = v6 * 2;
    long long v16 = v7 + 1000LL;
    int v17 = external_func(v1, v2);
    float v18 = v12 * 3.14f;
    double v19 = v13 * 2.71828;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = v1 + v2 + v11 + v17;
    float float_key = v3 + v9 + v12 + v18;
    double double_key = v4 + v10 + v13 + v19;
    long long ll_key = v7 + v16;
    
    /* Create label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile_fptr = labels[seed % 5];
    
    /* Complex nested conditionals with mixed types */
    if (v1 > 100) {
        if (v2 < 200) {
            switch (v5 % 4) {
                case 0:
                    /* Use key_result in inline asm with many clobbers */
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        "mov r0, %0\n\t"
                        "mov r1, %0\n\t"
                        "mov r2, %0\n\t"
                        "mov r3, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    v3 = float_key * 2.0f;
                    break;
                    
                case 1:
                    /* Use float_key with different mode */
                    asm volatile (
                        "vadd.f32 s0, %s0, %s0\n\t"
                        "vmov.f32 s1, s0\n\t"
                        "vmov.f32 s2, s0\n\t"
                        : "+w" (float_key)
                        :
                        : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                          "s8", "s9", "s10", "s11", "s12", "memory"
                    );
                    v4 = double_key / 2.0;
                    break;
                    
                case 2:
                    /* Use double_key with DImode/DFmode */
                    asm volatile (
                        "fadd.d %0, %0, %0\n\t"
                        : "+f" (double_key)
                        :
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                          "d8", "d9", "d10", "d11", "memory"
                    );
                    v7 = ll_key >> 1;
                    break;
                    
                case 3:
                    /* Use ll_key with long long operations */
                    asm volatile (
                        "add x0, %0, %0\n\t"
                        "mov x1, x0\n\t"
                        "mov x2, x0\n\t"
                        : "+r" (ll_key)
                        :
                        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                          "x8", "x9", "x10", "x11", "x12", "memory"
                    );
                    v5 = (char)(key_result & 0xFF);
                    break;
            }
            
            /* Nested if inside switch */
            if (v3 > 10.0f) {
                volatile int temp = external_func(key_result, v6);
                key_result += temp;
                
                /* Another level of nesting */
                for (int i = 0; i < 3; i++) {
                    if (v4 > 5.0) {
                        /* Use key_result again in deeply nested context */
                        asm volatile (
                            "mul %0, %0, #2\n\t"
                            : "+r" (key_result)
                            :
                            : "cc", "memory"
                        );
                        v15 = (short)(key_result % 1000);
                    }
                }
            }
        } else {
            /* Alternative path using goto with computed label */
            goto *volatile_fptr;
        }
    } else {
        /* Another complex path */
        for (int j = 0; j < 5; j++) {
            switch (j) {
                case 0: key_result *= 2; break;
                case 1: float_key += 1.0f; break;
                case 2: double_key -= 1.0; break;
                case 3: ll_key <<= 1; break;
                case 4: 
                    /* Mixed mode use */
                    asm volatile (
                        "add %0, %0, %1\n\t"
                        : "+r" (key_result)
                        : "r" ((int)float_key)
                        : "cc", "memory"
                    );
                    break;
            }
        }
    }
    
label1:
    /* Use key_result after conditional block with different mode */
    double_key += (double)key_result;
    
label2:
    /* More register pressure */
    int v20 = external_func(key_result, v15);
    float v21 = float_key * (float)v20;
    
label3:
    /* Complex expression using all key variables */
    long long final_ll = ll_key + (long long)key_result;
    double final_double = double_key + (double)float_key;
    
label4:
    /* Conditional jump back */
    if (v1 % 2) {
        volatile_fptr = &&label3;
        goto *volatile_fptr;
    }
    
label5:
    /* Final computation using all variables to prevent elimination */
    int final_result = key_result + (int)float_key + (int)double_key + 
                       (int)ll_key + v20 + (int)v21 + (int)final_double + 
                       (int)(final_ll & 0xFFFFFFFF);
    
    /* More inline asm to clobber registers */
    asm volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        : 
        : "r" (key_result), "r" (final_result), "r" (v20)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    return final_result;
}

/* Main function with multiple calls to increase pressure */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Additional volatile operations to prevent optimization */
        volatile int temp = i * 2;
        asm volatile ("" : : "r" (temp) : "memory");
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}

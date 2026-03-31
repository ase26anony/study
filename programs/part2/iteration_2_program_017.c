/* Test case for early-remat.cc privatization logic */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline, noclone))
int external_func(int x) {
    volatile static int counter = 0;
    return x + (++counter);
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_func;

/* Stress function that creates complex register usage patterns */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed + 1;
    long long ll1 = seed * 3LL;
    float f1 = seed * 0.5f;
    double d1 = seed * 0.25;
    int i2 = seed + 2;
    int i3 = seed + 3;
    int i4 = seed + 4;
    int i5 = seed + 5;
    int i6 = seed + 6;
    int i7 = seed + 7;
    int i8 = seed + 8;
    int i9 = seed + 9;
    int i10 = seed + 10;
    float f2 = seed * 1.5f;
    double d2 = seed * 2.25;
    long long ll2 = seed * 5LL;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex computation with mixed types */
    key_result = v1 + c1 + s1 + i1;
    key_result += (int)(f1 * 10.0f);
    key_result += (int)(d1 * 20.0);
    key_result += (int)(ll1 & 0xFFFFFFFF);
    
    /* Create label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile void* volatile_label_ptr = labels[seed % 4];
    
    /* Deeply nested conditional blocks */
    if (v1 > 100) {
        if (c1 < 50) {
            switch (s1 % 4) {
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
                    
                    /* Call through volatile function pointer */
                    key_result = volatile_func_ptr(key_result);
                    goto *volatile_label_ptr;
                    
                label1:
                    i2 = key_result * 2;
                    break;
                    
                case 1:
                    /* Different mode usage - double */
                    {
                        double temp = d1 + key_result;
                        asm volatile (
                            "vmov.f64 d0, %P0\n\t"
                            "vmov.f64 d1, %P0\n\t"
                            : 
                            : "w" (temp)
                            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
                              "memory"
                        );
                        d2 = temp;
                    }
                    goto *volatile_label_ptr;
                    
                label2:
                    i3 = key_result * 3;
                    break;
                    
                case 2:
                    /* Long long mode usage */
                    {
                        long long temp = ll1 + key_result;
                        asm volatile (
                            "mov x0, %0\n\t"
                            "mov x1, %0\n\t"
                            : 
                            : "r" (temp)
                            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                              "memory"
                        );
                        ll2 = temp;
                    }
                    goto *volatile_label_ptr;
                    
                label3:
                    i4 = key_result * 4;
                    break;
                    
                default:
                    /* Float mode usage */
                    {
                        float temp = f1 + key_result;
                        asm volatile (
                            "vmov.f32 s0, %0\n\t"
                            "vmov.f32 s1, %0\n\t"
                            : 
                            : "t" (temp)
                            : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                              "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",
                              "memory"
                        );
                        f2 = temp;
                    }
                    goto *volatile_label_ptr;
                    
                label4:
                    i5 = key_result * 5;
                    break;
            }
        } else {
            /* Another conditional path using key_result */
            for (int j = 0; j < c1; j++) {
                key_result += j;
                if (key_result % 7 == 0) {
                    asm volatile (
                        "add %0, %0, #100\n\t"
                        : "+r" (key_result)
                        : 
                        : "cc", "memory"
                    );
                }
            }
        }
    } else {
        /* Alternative path with different computation */
        key_result = key_result * 2 - v1;
    }
    
    /* Use key_result again outside conditional blocks */
    int final_result = key_result;
    
    /* Use all variables to keep them live */
    final_result += i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    final_result += (int)f2;
    final_result += (int)d2;
    final_result += (int)(ll2 & 0xFFFFFFFF);
    
    /* More register pressure */
    asm volatile (
        "mov %0, %0, lsl #2\n\t"
        : "+r" (final_result)
        : 
        : "cc"
    );
    
    return final_result;
}

/* Main function to drive the test */
int main() {
    int total = 0;
    
    /* Call with different seeds to explore various paths */
    for (int i = 0; i < 1000; i++) {
        total += stress_function(i);
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r"(total) : "memory");
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = total;
    
    return result % 256;
}

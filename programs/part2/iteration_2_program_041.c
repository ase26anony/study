/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func3(double x) { return x / 3.14159; }

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_fptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile int v1 = seed;  /* volatile to prevent elimination */
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    float f1 = seed * 0.5f;
    float f2 = seed * 1.5f;
    double d1 = seed * 2.5;
    double d2 = seed * 3.5;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000LL;
    int *ptr1 = &v1;
    float *ptr2 = &f1;
    
    /* Intermediate result that will be used conditionally */
    int intermediate = v1 + v2 + v3 + v4;
    float float_intermediate = f1 + f2;
    double double_intermediate = d1 + d2;
    
    /* Labels for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* Complex nested conditionals */
    if (v1 > 0) {
        if (v2 < 1000) {
            switch (v3 % 4) {
                case 0:
                    /* Use intermediate in inline asm with many clobbers */
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
                        : "+r" (intermediate)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    break;
                    
                case 1:
                    /* Use float intermediate with different mode */
                    float_intermediate = external_func2(float_intermediate);
                    asm volatile (
                        "fadds s0, s0, s0\n\t"
                        "fmov s1, s0\n\t"
                        "fmov s2, s0\n\t"
                        "fmov s3, s0\n\t"
                        : "+w" (float_intermediate)
                        :
                        : "s0", "s1", "s2", "s3", "memory"
                    );
                    break;
                    
                case 2:
                    /* Use double intermediate with DFmode */
                    double_intermediate = external_func3(double_intermediate);
                    asm volatile (
                        "fadd d0, d0, d0\n\t"
                        "fmov d1, d0\n\t"
                        "fmov d2, d0\n\t"
                        : "+w" (double_intermediate)
                        :
                        : "d0", "d1", "d2", "memory"
                    );
                    break;
                    
                case 3:
                    /* Mixed mode use with char/short */
                    c1 = (c1 + intermediate) & 0xFF;
                    s1 = (s1 + intermediate) & 0xFFFF;
                    asm volatile (
                        "add %0, %0, #10\n\t"
                        "add %1, %1, #20\n\t"
                        : "+r" (c1), "+r" (s1)
                        :
                        : "memory"
                    );
                    break;
            }
            
            /* Deeply nested conditional */
            if (float_intermediate > 0.0f) {
                for (int i = 0; i < 10; i++) {
                    if (i % 2 == 0) {
                        intermediate += i;
                        /* Opaque control flow via function pointer */
                        if (volatile_fptr) {
                            volatile_fptr();
                        }
                    } else {
                        intermediate -= i;
                    }
                    
                    /* Use goto with computed label */
                    if (i == 5) {
                        goto *volatile_label_ptr;
                    }
                }
                
                label1:
                intermediate += 100;
                
                label2:
                intermediate += 200;
            }
        } else {
            /* Another path using intermediate */
            intermediate = external_func1(intermediate);
            
            label3:
            intermediate *= 2;
        }
    } else {
        intermediate = -intermediate;
        
        label4:
        intermediate |= 0xFF;
    }
    
    label5:
    /* Use intermediate again after conditional blocks */
    int result = intermediate;
    
    /* More computations to keep variables live */
    result += (int)float_intermediate;
    result += (int)double_intermediate;
    result += c1;
    result += s1;
    result += (int)(ll1 % 1000);
    
    /* Create artificial register pressure with many live values */
    asm volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        "mov r3, %3\n\t"
        "mov r4, %4\n\t"
        "mov r5, %5\n\t"
        "mov r6, %6\n\t"
        "mov r7, %7\n\t"
        "mov r8, %8\n\t"
        "mov r9, %9\n\t"
        : 
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4),
          "r" (result), "r" (f1), "r" (f2), 
          "r" (d1), "r" (d2), "r" (ll1)
        : "r0", "r1", "r2", "r3", "r4", "r5", 
          "r6", "r7", "r8", "r9", "memory"
    );
    
    return result;
}

/* Additional variant for 32-bit compilation */
__attribute__((noinline, optimize("O2")))
int stress_function_32bit(int seed) {
    volatile int v1 = seed;
    int v2 = seed * 3;
    int v3 = seed + 200;
    float f1 = seed * 1.1f;
    double d1 = seed * 2.2;
    
    int intermediate = v1 + v2 + v3;
    
    /* Complex switch with mixed modes */
    switch (seed % 8) {
        case 0: {
            char c = intermediate & 0xFF;
            asm volatile (
                "add %0, %0, #5\n\t"
                : "+r" (c)
                :
                : "memory"
            );
            intermediate += c;
            break;
        }
        case 1: {
            short s = intermediate & 0xFFFF;
            asm volatile (
                "add %0, %0, #10\n\t"
                : "+r" (s)
                :
                : "memory"
            );
            intermediate += s;
            break;
        }
        case 2:
            intermediate = (int)f1 + intermediate;
            break;
        case 3:
            intermediate = (int)d1 + intermediate;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            /* Nested if-else chain */
            if (intermediate > 1000) {
                if (intermediate < 2000) {
                    intermediate += 500;
                } else if (intermediate < 3000) {
                    intermediate += 1000;
                } else {
                    intermediate += 2000;
                }
            } else {
                if (intermediate > 500) {
                    intermediate += 250;
                } else {
                    intermediate += 100;
                }
            }
            break;
    }
    
    return intermediate + (int)f1 + (int)d1;
}

int main() {
    int total = 0;
    
    /* Call stress functions multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        total += stress_function_32bit(i);
        
        /* Change volatile function pointer occasionally */
        if (i % 10 == 0) {
            volatile_fptr = (void (*)(void))stress_function;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}

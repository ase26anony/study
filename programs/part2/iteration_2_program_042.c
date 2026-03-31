/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x) {
    volatile int y = x;
    return y + 1;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile char c1 = seed & 0xFF;
    volatile short s1 = seed * 2;
    volatile int i1 = seed + 1000;
    volatile long long ll1 = (long long)seed * 1000000;
    volatile float f1 = seed * 3.14f;
    volatile double d1 = seed * 2.71828;
    volatile int *ptr1 = (int*)&i1;
    
    int i2 = seed * 3;
    int i3 = seed * 4;
    int i4 = seed * 5;
    int i5 = seed * 6;
    int i6 = seed * 7;
    int i7 = seed * 8;
    int i8 = seed * 9;
    int i9 = seed * 10;
    int i10 = seed * 11;
    
    float f2 = seed * 1.1f;
    float f3 = seed * 1.2f;
    float f4 = seed * 1.3f;
    
    double d2 = seed * 1.5;
    double d3 = seed * 1.6;
    
    /* Key intermediate result with complex computation */
    int key_result = i1 * i2 + i3 - i4;
    key_result = key_result * (seed % 7 + 1);
    
    /* Label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* Complex nested conditional structure */
    if (c1 > 0) {
        if (s1 < 1000) {
            switch (seed % 4) {
                case 0:
                    /* Deeply nested conditional use of key_result */
                    if (f1 > 0.0f) {
                        /* Inline assembly that clobbers many registers */
                        asm volatile (
                            "# Complex assembly block\n"
                            "mov %0, %%eax\n"
                            "add $1, %%eax\n"
                            : 
                            : "r" (key_result)
                            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                              "memory", "cc"
                        );
                        
                        /* Call non-inlineable function */
                        key_result = external_func(key_result);
                    }
                    break;
                    
                case 1:
                    if (d1 > 0.0) {
                        /* Another conditional use with different mode */
                        short temp = (short)key_result;
                        asm volatile (
                            "# Short mode use\n"
                            "movw %w0, %%ax\n"
                            : 
                            : "r" (temp)
                            : "ax", "bx", "cx", "memory"
                        );
                    }
                    break;
                    
                case 2:
                    /* Float mode use */
                    float f_temp = (float)key_result;
                    asm volatile (
                        "# Float mode use\n"
                        "flds %0\n"
                        "fadd %%st(0), %%st(0)\n"
                        : 
                        : "m" (f_temp)
                        : "st", "st(1)", "st(2)", "memory"
                    );
                    break;
                    
                case 3:
                    /* Double mode use */
                    double d_temp = (double)key_result;
                    asm volatile (
                        "# Double mode use\n"
                        "fldl %0\n"
                        : 
                        : "m" (d_temp)
                        : "st", "st(1)", "memory"
                    );
                    break;
            }
            
            /* Unpredictable goto using volatile pointer */
            if (i5 > 500) {
                goto *volatile_label_ptr;
            }
        }
    }
    
label1:
    /* Use key_result after conditional block with different computation */
    key_result = key_result + i6 * i7;
    
label2:
    /* More computations to keep variables live */
    i8 = key_result / (i9 + 1);
    
label3:
    /* Mix with floating point */
    f2 = f2 + (float)key_result;
    
label4:
    /* Pointer arithmetic */
    *ptr1 = key_result + i10;
    
label5:
    /* Final aggregation to prevent elimination */
    int final_result = key_result + i8 + (int)f2 + *ptr1;
    
    /* More inline assembly to increase pressure */
    asm volatile (
        "# Final clobber\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", 
          "r10", "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1",
          "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
    );
    
    return final_result;
}

/* Another layer of complexity */
__attribute__((noinline))
int wrapper_function(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Loop with varying conditions */
        if (i % 3 == 0) {
            total += stress_function(i);
        } else if (i % 3 == 1) {
            /* Create more register pressure in alternative path */
            volatile int x = i * 2;
            volatile float y = i * 3.14f;
            asm volatile (
                "# Alternative path\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
            );
            total += stress_function(x);
        } else {
            /* Third path with different computation */
            int temp = stress_function(i * 2);
            total += temp * 2;
        }
        
        /* Unpredictable control flow within loop */
        if (i % 7 == 0) {
            static void* loop_labels[] = { &&loop_label1, &&loop_label2 };
            goto *loop_labels[i % 2];
        }
        
    loop_label1:
        continue;
        
    loop_label2:
        total += 1;
    }
    
    return total;
}

int main() {
    int result = 0;
    
    /* Multiple calls with different parameters */
    result += wrapper_function(10);
    result += wrapper_function(5);
    result += wrapper_function(8);
    
    /* Use volatile to prevent dead code elimination */
    volatile int output = result;
    
    printf("Result: %d\n", output);
    
    return 0;
}

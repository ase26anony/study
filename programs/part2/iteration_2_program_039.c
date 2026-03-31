/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Also try: gcc -O2 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test32 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with unknown target */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_func;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    short v6 = seed * 6;
    char v7 = seed * 7;
    int v8 = seed * 8;
    long long v9 = seed * 9LL;
    float v10 = seed * 10.0f;
    double v11 = seed * 11.0;
    int v12 = seed * 12;
    int v13 = seed * 13;
    int v14 = seed * 14;
    int v15 = seed * 15;
    int v16 = seed * 16;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex computation with mixed types */
    key_result = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7;
    key_result += v8 + (int)v9 + (int)v10 + (int)v11;
    
    /* Volatile variable to prevent optimization */
    volatile int control = seed % 7;
    
    /* Create label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[control % 5];
    
    /* Deeply nested conditional blocks */
    if (control > 0) {
        if (control > 2) {
            switch (control) {
                case 3: {
                    /* Use key_result in inline assembly with many clobbers */
                    int temp;
                    asm volatile (
                        "mov %[val], %[res]\n\t"
                        "add $1, %[val]\n\t"
                        : [val] "=r" (temp)
                        : [res] "r" (key_result)
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory"
                    );
                    key_result = temp;
                    break;
                }
                case 4: {
                    /* Different mode usage - long long */
                    long long ll_temp = key_result;
                    asm volatile (
                        "movq %[val], %%rax\n\t"
                        "addq $2, %%rax\n\t"
                        : "=a" (ll_temp)
                        : [val] "r" (ll_temp)
                        : "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
                          "r11", "r12", "r13", "r14", "r15", "memory"
                    );
                    key_result = (int)ll_temp;
                    break;
                }
                default: {
                    /* Float mode usage */
                    float f_temp = key_result;
                    asm volatile (
                        "movss %[val], %%xmm0\n\t"
                        "addss %[inc], %%xmm0\n\t"
                        : "=x" (f_temp)
                        : [val] "x" (f_temp), [inc] "x" (3.14f)
                        : "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13",
                          "xmm14", "xmm15", "memory"
                    );
                    key_result = (int)f_temp;
                    break;
                }
            }
            
            /* Call non-inlineable function with key_result */
            key_result = volatile_func_ptr(key_result);
            
            /* Unpredictable goto using label address */
            if (control % 3 == 0) {
                goto *volatile_label_ptr;
            }
        } else {
            /* Another conditional path */
            for (int i = 0; i < 3; i++) {
                key_result += v12 + v13;
                if (i == 1) {
                    /* Use key_result in double mode */
                    double d_temp = key_result;
                    asm volatile (
                        "movsd %[val], %%xmm0\n\t"
                        : : [val] "x" (d_temp)
                        : "xmm0", "xmm1", "memory"
                    );
                }
            }
        }
    } else {
        /* Alternative path with loop */
        int counter = 0;
        while (counter < 5) {
            key_result -= v14 + v15;
            counter++;
            
            /* Conditional goto inside loop */
            if (counter == 3 && control == 0) {
                goto *labels[2];
            }
        }
    }

label1:
    /* Use key_result after conditional block */
    key_result += v16;
    
    /* More complex control flow */
    if (control % 2 == 0) {
        goto label3;
    }

label2:
    /* Another use of key_result with different computation */
    key_result *= 2;
    
    /* Force spill/reload behavior */
    for (int i = 0; i < 100; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        key_result += i;
    }

label3:
    /* Final computation using all variables */
    int final_result = key_result;
    final_result += v1 + v2 + (int)v3 + (int)v4 + (int)v5;
    final_result += v6 + v7 + v8 + (int)v9 + (int)v10 + (int)v11;
    final_result += v12 + v13 + v14 + v15 + v16;
    
    /* One more conditional jump */
    if (final_result % 11 == 0) {
        goto label4;
    }

label4:
    /* Use key_result in pointer arithmetic */
    int* ptr = &final_result;
    *ptr += key_result;
    
    /* Mixed mode operation */
    long long big_temp = (long long)key_result * (long long)final_result;
    float float_temp = (float)key_result * 1.5f;
    double double_temp = (double)key_result * 2.5;
    
    /* Final aggregation to prevent elimination */
    return final_result + (int)big_temp + (int)float_temp + (int)double_temp;

label5:
    /* Unreachable path but needed for label array */
    return seed * -1;
}

int main() {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 1000; i++) {
        total += stress_function(i);
        
        /* Change function pointer occasionally */
        if (i % 37 == 0) {
            volatile_func_ptr = &external_func;
        }
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}

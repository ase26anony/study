/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test32 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) void use_registers(void) {
    asm volatile ("" : : : "memory");
}

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_func_ptr)(void) = use_registers;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline)) int stress_function(int seed) {
    /* Declare many variables of different types to create register pressure */
    volatile int v1 = seed;
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long long ll1, ll2, ll3, ll4;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    void *ptr1, *ptr2, *ptr3;
    
    /* Complex computation creating many intermediate values */
    i1 = v1 * 2;
    i2 = i1 + s1;
    i3 = i2 * 3;
    i4 = i3 - s2;
    i5 = i4 / 2;
    
    /* Mixed type computations forcing mode conversions */
    f1 = (float)i1 * 1.5f;
    f2 = (float)i2 * 2.5f;
    d1 = (double)i3 * 3.14159;
    d2 = (double)i4 * 2.71828;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = i5 * 7;
    
    /* Create label addresses for complex control flow */
    void *labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void *volatile_label_ptr = labels[seed % 5];
    
    /* Deeply nested conditional structure */
    if (v1 > 100) {
        if (c1 == 'a') {
            switch (s1) {
                case 100:
                    /* Use key_result in inline asm with many clobbers */
                    asm volatile (
                        "mov %0, %%eax\n\t"
                        "add $1, %%eax\n\t"
                        : 
                        : "r" (key_result)
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory", "cc"
                    );
                    
                    /* Call non-inlineable function */
                    vol_func_ptr();
                    
                    /* Conditional goto using label pointer */
                    if (v1 & 1) {
                        goto *volatile_label_ptr;
                    }
                    break;
                    
                case 200:
                    /* Different use of key_result */
                    asm volatile (
                        "imul $2, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "cc"
                    );
                    break;
                    
                default:
                    /* More register pressure */
                    ll1 = (long long)key_result * 100LL;
                    ll2 = ll1 + 500LL;
                    break;
            }
            
            /* Nested if-else chain */
            if (f1 > 10.0f) {
                i6 = key_result + 100;
                if (d1 > 20.0) {
                    i7 = i6 * 2;
                    /* Another conditional asm use */
                    asm volatile (
                        "add %%ebx, %%eax\n\t"
                        : 
                        : "a" (key_result), "b" (i7)
                        : "cc"
                    );
                } else {
                    i8 = key_result - 50;
                }
            }
        } else if (c2 == 'b') {
            /* Alternative path with different computations */
            f3 = f2 * 2.0f;
            d3 = d2 / 2.0;
            i9 = (int)f3 + (int)d3;
        }
    } else {
        /* Else branch with its own complex flow */
        for (int j = 0; j < 10; j++) {
            i10 = key_result + j;
            if (j & 1) {
                /* Use in loop with volatile access */
                volatile int *vol_ptr = &i10;
                key_result = *vol_ptr * 2;
            }
        }
    }
    
label1:
    /* Use key_result after conditional block */
    ll3 = (long long)key_result * 3LL;
    
label2:
    /* More mixed-type computations */
    f4 = (float)ll3 / 100.0f;
    i1 = (int)f4 + key_result;
    
label3:
    /* Pointer arithmetic creating different modes */
    ptr1 = malloc(100);
    if (ptr1) {
        int *int_ptr = (int *)ptr1;
        *int_ptr = key_result;
        free(ptr1);
    }
    
label4:
    /* Final computation using all variables to prevent elimination */
    int final_result = i1 + i2 + i3 + i4 + i5 + 
                      (int)f1 + (int)f2 + (int)f4 +
                      (int)d1 + (int)d2 +
                      c1 + c2 + c3 +
                      s1 + s2 + s3 +
                      (int)ll3;
    
label5:
    return final_result + key_result;
}

/* Another layer of indirection */
__attribute__((noinline)) int wrapper_function(int iterations) {
    int total = 0;
    for (int i = 0; i < iterations; i++) {
        total += stress_function(i);
        
        /* Volatile store to prevent loop optimization */
        volatile int *vol_total = &total;
        *vol_total = total;
    }
    return total;
}

int main(void) {
    int result = wrapper_function(1000);
    printf("Result: %d\n", result);
    return 0;
}

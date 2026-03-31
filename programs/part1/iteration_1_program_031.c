#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes and calling conventions */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
many_args_func(int a, int b, int c, int d, int e, int f, int g, int h, 
                int i, int j, float k, double l, void* m) {
    volatile int result = a + b + c + d;
    result += e + f + g + h;
    result += i + j;
    result += (int)k + (int)l;
    result += (int)(intptr_t)m;
    return result;
}

/* Function that clobbers registers via inline assembly */
static void __attribute__((noinline))
clobber_registers(void) {
    /* Force clobbering of call-clobbered registers */
    __asm__ volatile (
        "mov $0x12345678, %%eax\n\t"
        "mov $0x87654321, %%ecx\n\t"
        "mov $0x11111111, %%edx\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ecx", "edx", "memory"
    );
}

/* Function with pointer arithmetic to create register pressure */
static float __attribute__((noinline))
float_ops(float a, float b, float c, float d, float e, float f) {
    volatile float result = a * b + c / d - e * f;
    result = result * 2.0f - 1.0f;
    return result;
}

/* Function that takes mixed types and returns pointer */
static int* __attribute__((noinline))
pointer_func(int a, float b, double c, int* d) {
    static int storage[4];
    storage[0] = a + (int)b + (int)c;
    storage[1] = *d;
    storage[2] = storage[0] * storage[1];
    storage[3] = storage[2] / (a ? a : 1);
    return storage;
}

/* Function using alloca to affect frame pointer */
static int __attribute__((noinline))
use_alloca(int size) {
    volatile int* ptr = alloca(size * sizeof(int));
    int sum = 0;
    for (int i = 0; i < size && i < 8; i++) {
        ptr[i] = i * i;
        sum += ptr[i];
    }
    return sum;
}

/* Main computation with high register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 5.5, d2 = 6.6, d3 = 7.7;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile int* p3 = &v3;
    int result = 0;
    float float_result = 0.0f;
    
    /* Take addresses to inhibit optimization and affect frame pointer */
    int* addr1 = &v1;
    int* addr2 = &v2;
    float* addr3 = &f1;
    
    /* Loop to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Complex computation between calls to keep values in registers */
        v1 = v1 * 2 + iteration;
        v2 = v2 + v1 - iteration;
        v3 = v3 * 3 - v2;
        v4 = v4 + v3 / 2;
        v5 = v5 ^ v4;
        
        f1 = f1 * 1.5f + (float)iteration;
        f2 = f2 - f1 * 0.5f;
        f3 = f3 + f2 * 2.0f;
        f4 = f4 / (f3 + 1.0f);
        
        d1 = d1 + (double)v1;
        d2 = d2 * (double)f2;
        d3 = d3 - d1 + d2;
        
        /* Inline assembly that clobbers specific registers */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+m" (v1)
            : "r" (v2)
            : "eax", "memory"
        );
        
        /* Conditional to create basic block boundaries */
        if (iteration % 2 == 0) {
            /* Call function with many arguments - will exceed register passing */
            int call_result = many_args_func(v1, v2, v3, v4, v5, 
                                            v1+v2, v2+v3, v3+v4,
                                            v4+v5, v5+v1,
                                            f1, d1, (void*)p1);
            
            /* More computation between calls */
            v1 = call_result % 100;
            v2 = (call_result / 100) % 100;
            
            /* Force register clobbering */
            clobber_registers();
            
            /* Call floating point intensive function */
            float_result += float_ops(f1, f2, f3, f4, f1+f2, f3+f4);
            
            /* Use alloca to affect frame pointer */
            int alloca_result = use_alloca(iteration + 2);
            v3 += alloca_result;
        } else {
            /* Different call pattern in else branch */
            int* ptr_result = pointer_func(v1, f1, d1, &v2);
            v4 += *ptr_result;
            
            /* More inline assembly with different clobbers */
            __asm__ volatile (
                "mov $0xAAAAAAAA, %%r10d\n\t"
                "xor %%r10d, %0\n\t"
                : "+r" (v5)
                :
                : "r10", "cc"
            );
            
            /* Nested condition to create more basic blocks */
            if (v5 > 1000) {
                float_result -= float_ops(f2, f3, f4, f1, f2+f3, f4+f1);
            }
        }
        
        /* Additional computation to extend live ranges */
        p1 = &v1;
        p2 = &v2;
        p3 = &v3;
        
        *p1 = *p1 + *p2;
        *p2 = *p2 ^ *p3;
        *p3 = *p3 * 2 - *p1;
        
        /* Another function call with mixed arguments */
        int call2 = many_args_func(*p1, *p2, *p3, v4, v5,
                                   iteration, iteration*2, iteration*3,
                                   iteration*4, iteration*5,
                                   float_result, d2, (void*)p2);
        
        result += call2;
        
        /* Final clobber to ensure registers need saving */
        __asm__ volatile (
            "movl $0xDEADBEEF, %%ebx\n\t"
            "movl $0xCAFEBABE, %%esi\n\t"
            "movl $0xFEEDFACE, %%edi\n\t"
            :
            :
            : "ebx", "esi", "edi", "memory"
        );
    }
    
    /* Compute final checksum */
    int checksum = v1 + v2 + v3 + v4 + v5;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += result;
    checksum += (int)float_result;
    
    printf("Result: %d\n", checksum);
    printf("v1=%d, v2=%d, v3=%d, v4=%d, v5=%d\n", v1, v2, v3, v4, v5);
    printf("f1=%.2f, f2=%.2f, f3=%.2f, f4=%.2f\n", f1, f2, f3, f4);
    
    return checksum != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes and calling conventions */

/* Function with many arguments to force stack passing */
__attribute__((noinline))
static int many_args(int a, int b, int c, int d, int e, int f, int g, int h,
                     int i, int j, int k, int l) {
    volatile int result = a + b - c + d - e + f - g + h - i + j - k + l;
    return result;
}

/* Function returning float to use FP registers */
__attribute__((noinline))
static float float_ops(float a, float b, float c, float d) {
    volatile float temp = a * b + c / d;
    return temp * 2.0f;
}

/* Function with pointer arguments */
__attribute__((noinline))
static void pointer_ops(int* a, int* b, int* c) {
    *a = *b + *c;
    *b = *a - *c;
    *c = *b * *a;
}

/* Function that uses alloca to affect frame pointer */
__attribute__((noinline))
static int use_alloca(int size) {
    volatile int* ptr = alloca(size * sizeof(int));
    int sum = 0;
    for (int i = 0; i < size; i++) {
        ptr[i] = i;
        sum += ptr[i];
    }
    return sum;
}

/* External function to prevent inlining */
__attribute__((noinline))
extern int external_func(int x, int y);

/* External function implementation */
int external_func(int x, int y) {
    volatile int result = x * y + (x ^ y) - (x & y);
    return result;
}

/* Function with mixed types */
__attribute__((noinline))
static double mixed_types(int a, float b, double c, int* d) {
    volatile double result = (double)a + (double)b + c + (double)(*d);
    
    /* Inline assembly to clobber specific registers */
    __asm__ volatile (
        "# Clobber caller-saved registers\n"
        "mov $0x12345678, %%eax\n"
        "mov $0x87654321, %%edx\n"
        "add %%edx, %%eax\n"
        :
        : 
        : "eax", "edx", "memory"
    );
    
    return result * 2.0;
}

/* Another function with different register clobbering */
__attribute__((noinline))
static void clobber_registers(void) {
    /* Clobber more registers */
    __asm__ volatile (
        "# Clobber additional registers\n"
        "mov $0x11111111, %%r10\n"
        "mov $0x22222222, %%r11\n"
        "xor %%r10, %%r11\n"
        :
        :
        : "r10", "r11", "memory"
    );
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    
    int result = 0;
    
    /* Complex control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 + v2 * v3;
        v4 = v4 - v5 / (v6 + 1);
        f1 = f1 * f2 + f3;
        
        /* Function call that might be inlined or not */
        int temp = many_args(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, 
                            v1 + v2, v3 + v4);
        
        /* More computations keeping values live */
        v2 = v2 ^ temp;
        v3 = v3 | (v4 & v5);
        
        /* Inline assembly between computations and calls */
        __asm__ volatile (
            "# Force register clobbering between live values\n"
            "mov %0, %%eax\n"
            "add $0x100, %%eax\n"
            "mov %%eax, %0\n"
            : "+r" (v1)
            :
            : "eax", "memory"
        );
        
        /* Conditional to create basic block boundaries */
        if (iteration % 2 == 0) {
            /* Call function with float return */
            float ftemp = float_ops(f1, f2, f3, f4);
            v5 = v5 + (int)ftemp;
            
            /* Pointer operations */
            pointer_ops((int*)p1, (int*)p2, &v6);
            
            /* More register pressure */
            d1 = mixed_types(v7, f4, d2, &v8);
            
            /* Use alloca to affect frame pointer */
            int alloca_result = use_alloca(v9 % 5 + 2);
            v10 = v10 + alloca_result;
        } else {
            /* Different path with other function calls */
            v7 = external_func(v7, v8);
            
            /* Clobber registers function */
            clobber_registers();
            
            /* More computations */
            v8 = v8 * v9 + v10;
            f2 = f2 / f3 * f4;
        }
        
        /* Loop with computations between calls */
        for (int j = 0; j < 2; j++) {
            v9 = v9 + v10 - v1;
            f3 = f3 + f4 - f1;
            
            /* Function call inside nested loop */
            if (j == 0) {
                d2 = mixed_types(v2, f2, d1, &v3);
            }
            
            v10 = v10 ^ v9;
        }
        
        /* Final computations in the loop */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        result += (int)d1 + (int)d2;
    }
    
    /* Take address of locals to affect frame pointer optimization */
    int* addr_array[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Final function call sequence */
    int final_result = many_args(
        result, *addr_array[0], *addr_array[1], *addr_array[2],
        *addr_array[3], *addr_array[4], *addr_array[5], *addr_array[6],
        *addr_array[7], *addr_array[8], *addr_array[9], result % 100
    );
    
    /* One more clobber for good measure */
    __asm__ volatile (
        "# Final register clobber\n"
        "mov $0xFFFFFFFF, %%eax\n"
        "xor %%eax, %%eax\n"
        :
        :
        : "eax", "memory"
    );
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}

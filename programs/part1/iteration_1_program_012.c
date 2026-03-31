/* caller-save-test.c - ISO C99 program to exercise GCC's caller-save register allocation */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force noinline to prevent unwanted optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with different signatures and calling conventions */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result;
}

NOINLINE static float helper2(float a, float b, float c, float d, 
                               float e, float f, float g, float h) {
    volatile float result = a * b + c * d - e * f + g / h;
    return result;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, void* p4,
                              void* p5, void* p6, void* p7, void* p8) {
    volatile uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3 + 
                            (uintptr_t)p4 + (uintptr_t)p5 + (uintptr_t)p6 + 
                            (uintptr_t)p7 + (uintptr_t)p8;
    return (void*)(sum % 256);
}

NOINLINE int external_func(int x, float y, void* z) {
    volatile int result = x + (int)y + (int)(intptr_t)z;
    return result;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int frame_pointer_test(int n) {
    volatile int* array = alloca(n * sizeof(int));
    int sum = 0;
    for (int i = 0; i < n; i++) {
        array[i] = i * 2;
        sum += array[i];
    }
    return sum;
}

/* Complex function with mixed operations */
NOINLINE static int complex_calculation(int a, int b, int c, int d, int e,
                                        float f, float g, float h, float i) {
    volatile int int_part = a * b + c - d * e;
    volatile float float_part = f * g - h / i;
    
    /* Inline assembly to clobber specific registers */
    __asm__ volatile (
        "# Clobber caller-saved registers\n"
        "mov $0x12345678, %%eax\n"
        "mov $0x87654321, %%ecx\n"
        "mov $0x11111111, %%edx\n"
        :
        : 
        : "eax", "ecx", "edx", "memory"
    );
    
    return int_part + (int)float_part;
}

/* Main test function */
int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile void* p1 = (void*)0x1000, *p2 = (void*)0x2000;
    volatile int result1, result2, result3;
    volatile float fresult1, fresult2;
    volatile void* presult;
    
    /* Take addresses to affect frame pointer decisions */
    int* addr1 = &v1;
    int* addr2 = &v2;
    float* addr3 = &f1;
    
    /* Initial computations keeping values in registers */
    v1 = v1 * 2 + v2;
    v2 = v2 - v3 * v4;
    v3 = v3 + v5 / 2;
    f1 = f1 * f2 + f3;
    f2 = f2 - f4 / f5;
    
    /* First function call with many arguments */
    result1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
    
    /* More computations between calls */
    v4 = v4 + result1;
    v5 = v5 * 2 - v4;
    f3 = f3 * 2.0f + f1;
    
    /* Inline assembly to clobber registers between computations */
    __asm__ volatile (
        "# Force register clobbering\n"
        "mov $0xDEADBEEF, %%r10d\n"
        "mov $0xCAFEBABE, %%r11d\n"
        :
        : 
        : "r10", "r11", "memory"
    );
    
    /* Second function call with float arguments */
    fresult1 = helper2(f1, f2, f3, f4, f5, f1, f2, f3);
    
    /* Control flow to create basic block boundaries */
    if (result1 > 100) {
        /* More computations in this basic block */
        v6 = v6 + v7 * v8;
        v7 = v7 - v9 / v10;
        
        /* Third function call with pointer arguments */
        presult = helper3(p1, p2, addr1, addr2, addr3, 
                         (void*)&v1, (void*)&f1, (void*)&result1);
        
        /* Another inline assembly clobber */
        __asm__ volatile (
            "# More register clobbering\n"
            "xor %%rax, %%rax\n"
            "xor %%rcx, %%rcx\n"
            :
            :
            : "rax", "rcx", "memory"
        );
        
        v8 = v8 + (int)(intptr_t)presult;
    } else {
        /* Alternative path with different computations */
        v9 = v9 * 3 - v10;
        v10 = v10 + v1 * 2;
        
        /* Function call that uses alloca */
        result2 = frame_pointer_test(16);
        
        v9 = v9 + result2;
    }
    
    /* Loop to create more instruction density */
    for (int i = 0; i < 5; i++) {
        /* Mixed operations in loop */
        v1 = v1 + i;
        v2 = v2 - i * 2;
        f1 = f1 + (float)i * 0.5f;
        
        /* Function call inside loop */
        result3 = complex_calculation(v1, v2, v3, v4, v5, 
                                     f1, f2, f3, f4);
        
        /* More computations after call */
        v3 = v3 + result3;
        
        /* Conditional inside loop for more basic blocks */
        if (i % 2 == 0) {
            /* External function call */
            fresult2 = (float)external_func(v1, f1, (void*)(intptr_t)v2);
            v4 = v4 + (int)fresult2;
        }
    }
    
    /* Final computation and output */
    int final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    final_result += (int)f1 + (int)f2 + (int)f3;
    final_result += (int)(intptr_t)presult;
    
    printf("Final checksum: %d\n", final_result);
    
    /* Verify execution */
    if (final_result != 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and takes many arguments (exceeding register limits) */
static int __attribute__((noinline)) 
many_args_func(int a, int b, int c, int d, int e, int f, int g, int h, 
                int i, int j, int k, int l, float m, double n, void* p) {
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l;
    result += (int)m + (int)n + (int)(intptr_t)p;
    return result;
}

/* Function with pointer arguments that may be optimized differently */
float __attribute__((noinline)) 
float_heavy_func(float a, float b, float c, float* d, float* e) {
    volatile float temp = a * b + c;
    *d = temp * 2.0f;
    *e = temp / 2.0f;
    return temp;
}

/* Function that uses alloca to affect frame pointer */
void* __attribute__((noinline, noipa))
use_alloca_func(size_t size) {
    void* ptr = alloca(size);
    /* Use the allocated memory to prevent optimization */
    volatile char* vptr = (volatile char*)ptr;
    for (size_t i = 0; i < size && i < 16; i++) {
        vptr[i] = (char)i;
    }
    return ptr;
}

/* Function with mixed types */
double __attribute__((noinline))
mixed_types_func(int a, float b, double c, int* d, float* e, double* f) {
    volatile double result = (double)a + (double)b + c;
    *d = (int)result;
    *e = (float)result;
    *f = result * 2.0;
    return result;
}

/* Static function that might be inlined in some compilation modes */
static int static_helper(int x, int y) {
    volatile int temp = x * y;
    /* Inline assembly that clobbers registers */
    __asm__ volatile (
        "# Force clobber\n"
        : 
        : "r"(x), "r"(y)
        : "rax", "r10", "r11", "memory"
    );
    return temp + 1;
}

/* Another static function with different signature */
static float static_float_helper(float a, float b, float* out) {
    volatile float prod = a * b;
    *out = prod;
    
    /* More inline assembly with clobbers */
    __asm__ volatile (
        "# More forced clobbers\n"
        : 
        : "r"(a), "r"(b)
        : "xmm0", "xmm1", "xmm2", "memory"
    );
    
    return prod + 1.0f;
}

int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    volatile float f3 = 3.5f;
    volatile double d1 = 10.5;
    volatile double d2 = 20.5;
    int* p1 = (int*)&v1;
    float* p2 = &f1;
    double* p3 = &d1;
    
    /* Take addresses to affect frame pointer decisions */
    int* addr1 = &v1;
    int* addr2 = &v2;
    float* addr3 = &f1;
    
    /* Result accumulator */
    volatile int checksum = 0;
    
    /* Control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic Block 1: Multiple computations between calls */
        v1 = v2 + v3;
        v4 = v5 * v6;
        f1 = f2 + f3;
        d1 = d2 * 2.0;
        
        /* Inline assembly that clobbers call-clobbered registers */
        __asm__ volatile (
            "# Clobber important registers\n"
            : 
            : "r"(v1), "r"(v2), "r"(v3)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Function call with many arguments - will exceed register passing */
        int result1 = many_args_func(v1, v2, v3, v4, v5, v6, v7, v8, 
                                     v9, v10, v1+v2, v3+v4, f1, d1, p1);
        
        /* Basic Block 2: More computations */
        if (result1 > 100) {
            v7 = v8 - v9;
            v10 = v1 * v2;
            f2 = f1 * 3.0f;
            
            /* Another inline assembly with different clobbers */
            __asm__ volatile (
                "# Different register clobber set\n"
                : 
                : "r"(v7), "r"(v8), "r"(f2)
                : "rbx", "rbp", "r12", "r13", "r14", "r15",
                  "xmm6", "xmm7", "xmm8", "xmm9", "memory"
            );
            
            /* Call function that returns float */
            float result2 = float_heavy_func(f1, f2, f3, p2, &f3);
            
            /* Use alloca to affect frame pointer */
            void* dynamic = use_alloca_func(64);
            volatile char* vdyn = (volatile char*)dynamic;
            vdyn[0] = (char)result1;
            
            v5 = v6 + (int)result2;
        } else {
            v7 = v9 + v10;
            f3 = f1 / f2;
            
            /* Mixed types function call */
            int out1;
            float out2;
            double out3;
            double result3 = mixed_types_func(v1, f1, d1, &out1, &out2, &out3);
            
            v8 = out1 + (int)result3;
            d2 = out3;
        }
        
        /* Basic Block 3: Call static helpers */
        int static_result1 = static_helper(v1, v2);
        float static_out;
        float static_result2 = static_float_helper(f1, f2, &static_out);
        
        /* More computations using results */
        v9 = v10 + static_result1 + (int)static_result2;
        f1 = static_out + f3;
        
        /* Another call with many args in the loop */
        int result4 = many_args_func(v9, v8, v7, v6, v5, v4, v3, v2,
                                     v1, static_result1, v10, iteration,
                                     f1, d2, p1);
        
        /* Update checksum */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2;
        checksum += result1 + result4 + static_result1 + (int)static_result2;
        
        /* Loop creates multiple basic blocks with calls inside */
        if (iteration == 1) {
            /* Nested call sequence */
            int temp = many_args_func(1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                      checksum, iteration, 1.0f, 1.0, p1);
            checksum += temp;
        }
    }
    
    /* Final computation and output */
    printf("Final checksum: %d\n", checksum);
    
    /* Use addresses to ensure they're not optimized away */
    printf("Address samples: %p %p %p\n", (void*)addr1, (void*)addr2, (void*)addr3);
    
    return checksum != 0 ? 0 : 1;
}

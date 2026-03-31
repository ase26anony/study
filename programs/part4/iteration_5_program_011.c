/* reload_stress.c - Program to stress GCC's reload mechanism */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force register saves/restores across calls */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int barrier = a + b + (int)c + (int)d + e + (int)f;
    return barrier & 0xFF;
}

/* Another noinline function to clobber registers */
__attribute__((noinline, optimize("O0")))
void clobber_regs(void) {
    /* Empty function that compiler can't optimize away */
    asm volatile("" : : : "memory");
}

int main(void) {
    /* Phase 1: Declare MANY variables to create register pressure */
    
    /* Integer variables - many will need to stay live */
    register int r0 asm("r15") = rand();  /* Try to bind to specific reg */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    int v3 = rand() % 100;
    int v4 = rand() % 100;
    int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    
    /* Floating point variables - different register class */
    volatile float f1 = (float)rand() / RAND_MAX;
    volatile float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    float f6 = (float)rand() / RAND_MAX;
    
    /* Double precision - another register class on some archs */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    volatile double d4 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    volatile int* ptr1 = &array[0];
    volatile int* ptr2 = &array[128];
    int* ptr3 = array + 64;
    
    /* Phase 2: Complex loop with register pressure */
    volatile int loop_counter = 50;  /* Prevent loop unrolling */
    volatile int accumulator = 0;
    
    while (loop_counter-- > 0) {
        /* Step 1: Complex integer expressions with many live values */
        int t1 = v1 + v2 * v3 - v4 / (v5 + 1);
        int t2 = (v6 & v7) | (v8 << 2) ^ (v9 >> 1);
        int t3 = v10 * v11 + v12 - v13 * v14;
        
        /* Force integer-to-float conversions (requires moving between reg files) */
        f1 = (float)(t1 + t2) * 0.5f;
        f2 = (float)v15 / (float)(t3 + 1);
        
        /* Mixed-type expression */
        d1 = (double)f3 * (double)t1 + (double)f4 * (double)t2;
        
        /* Step 2: Function call clobbers registers */
        int result = dummy_function(t1, t2, f1, d1, v1, f2);
        
        /* Step 3: Inline assembly with explicit clobbers */
        asm volatile(
            "# Complex inline assembly\n"
            "add %[a], %[b]\n"
            : [a] "+r" (v3), [b] "+r" (v4)
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", 
              "f0", "f1", "f2", "f3", "f4", "f5",
              "memory", "cc"
        );
        
        /* Step 4: Complex addressing modes */
        /* Non-offsettable address: array[base + large_constant] */
        int idx = v5 + 1000;  /* Large constant may not be offsettable */
        int val1 = array[idx & 0xFF];  /* Complex addressing */
        
        /* Pointer arithmetic that may need reloading */
        int* complex_ptr = ptr3 + (v6 * 2 - v7) / 4;
        int val2 = *complex_ptr;
        
        /* Different sized accesses in same expression */
        short s1 = (short)val1;
        char c1 = (char)val2;
        int val3 = (int)s1 * (int)c1 + (v8 & 0xFF);
        
        /* Step 5: More type conversions and mixed operations */
        f3 = f1 * f2 + (float)val3;
        d2 = d1 * 0.5 + (double)f3;
        
        /* Convert float to int (another register file move) */
        v11 = (int)f3 + (int)d2;
        
        /* Bitwise float manipulation via integer */
        union { float f; uint32_t i; } u;
        u.f = f4;
        u.i = (u.i & 0x7F800000) | ((u.i & 0x007FFFFF) ^ v9);
        f4 = u.f;
        
        /* Step 6: Another function call */
        clobber_regs();
        
        /* Step 7: Update accumulator with complex expression */
        accumulator += t1 + t2 + (int)f1 + (int)d1 + val1 + val2 + v11;
        
        /* Step 8: Rotate values to extend live ranges */
        int temp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = temp;
        
        float ftemp = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6; f6 = ftemp;
        
        double dtemp = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = dtemp;
    }
    
    /* Phase 3: Final complex expression using all variables */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        accumulator;
    
    printf("Result: %d (Run with different optimization flags)\n", final_result);
    
    /* Use all variables one more time to prevent optimization */
    volatile int anti_opt = 
        r0 + *ptr1 + *ptr2 + array[final_result & 0xFF];
    
    return final_result & 0x7F;  /* Return non-zero to indicate execution */
}

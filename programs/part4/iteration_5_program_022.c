#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
float float_ops(float a, float b, double c, double d) {
    volatile float res = a * b + (float)(c * d);
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    volatile int v4 = rand();
    
    /* MANY LIVE SCALAR VARIABLES (20+) */
    /* Integer variables */
    int a1 = rand() % 100;
    int a2 = rand() % 100;
    int a3 = rand() % 100;
    int a4 = rand() % 100;
    int a5 = rand() % 100;
    int a6 = rand() % 100;
    int a7 = rand() % 100;
    int a8 = rand() % 100;
    int a9 = rand() % 100;
    int a10 = rand() % 100;
    
    /* More integers */
    int b1 = rand() % 100;
    int b2 = rand() % 100;
    int b3 = rand() % 100;
    int b4 = rand() % 100;
    int b5 = rand() % 100;
    int b6 = rand() % 100;
    int b7 = rand() % 100;
    int b8 = rand() % 100;
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int r1 asm("r12") = rand() % 100;  /* Try to bind to specific reg */
    register int r2 asm("r13") = rand() % 100;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer variables for address calculations */
    int *ptr1 = &array[0];
    int *ptr2 = &array[128];
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Integer operations mixing many variables */
        int t1 = a1 + a2 * a3 - a4 / (a5 + 1);
        int t2 = b1 & b2 | b3 ^ b4 << (b5 % 8);
        int t3 = (a6 * a7) + (a8 & a9) - (a10 | b6);
        
        /* Floating point operations */
        float ft1 = f1 * f2 + f3 / (f4 + 0.001f);
        float ft2 = (float)t1 * f5 - (float)t2 * 0.5f;
        
        /* Double precision operations */
        double dt1 = d1 * d2 + d3 / (d4 + 0.000001);
        double dt2 = (double)ft1 * 2.0 + (double)t3 * 0.25;
        
        /* TYPE CONVERSIONS - force moves between register files */
        f1 = (float)a1 + (float)b1 * 0.5f;
        d1 = (double)f1 * (double)a2;
        a1 = (int)f2 + (int)d2;
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = (a3 + b2) % 128;
        int idx2 = (a4 + b3 + 64) % 128;
        
        /* Non-simple address: array[base + index*scale + constant] */
        int val1 = array[idx1 * 2 + 16];  /* May need address reload */
        int val2 = array[idx2 * 3 + 32];  /* Complex addressing */
        
        /* More complex: pointer arithmetic with non-constant offset */
        int *addr1 = ptr1 + (a5 % 64) + (b4 % 32);
        int *addr2 = ptr2 + (a6 % 32) * 2 - (b5 % 16);
        
        int val3 = *addr1;
        int val4 = *addr2;
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_func(a1, a2, f1, d1, b1, b2);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS - increases register pressure */
        /* This tells GCC these registers are unusable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "memory"
        );
        
        /* MORE CALCULATIONS after clobber */
        /* Use explicit register variables in conflicting contexts */
        /* r1 and r2 are declared as integer registers, use in FP context */
        float fr1 = (float)r1 * 0.5f;  /* May need reload */
        double dr2 = (double)r2 * 0.25; /* May need reload */
        
        /* Mixed operations with different sizes */
        short s1 = (short)(a1 & 0xFFFF);
        char c1 = (char)(b1 & 0xFF);
        int mixed1 = (int)s1 * (int)c1 + a2;
        
        /* 64-bit operations on 32-bit arch would need reloads */
        int64_t big1 = (int64_t)a1 * (int64_t)b1;
        int64_t big2 = (int64_t)a2 * (int64_t)b2;
        
        /* BITWISE and ARITHMETIC combinations */
        int complex1 = (a1 & b1) + (a2 | b2) * (a3 ^ b3);
        int complex2 = (a4 << (b4 % 8)) + (a5 >> (b5 % 8)) - (a6 & b6);
        
        /* Another function call with different types */
        float fresult = float_ops(f1, f2, d1, d2);
        
        /* Update many variables to keep them live */
        a1 = t1 + val1;
        a2 = t2 - val2;
        a3 = t3 + val3;
        a4 = complex1 - val4;
        b1 = call_result % 100;
        b2 = mixed1 % 100;
        f1 = ft1 * 0.9f;
        f2 = ft2 + fresult;
        d1 = dt1 * 0.99;
        d2 = dt2 - dr2;
        
        /* Use explicit register variables */
        r1 = r1 * 2 + 1;
        r2 = r2 / 2 - 1;
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += a1 + a2 + a3 + a4 + b1 + b2 + 
                      (int)f1 + (int)f2 + (int)d1 + (int)d2 +
                      (int)fr1 + val1 + val2 + val3 + val4;
        
        /* Another inline asm with clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r12", "r13", "r14", "r15", 
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "memory"
        );
    }
    
    /* Final output to prevent optimization */
    printf("Result: %d\n", accumulator);
    
    /* Use all variables one more time to extend live ranges */
    int final_sum = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        r1 + r2;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

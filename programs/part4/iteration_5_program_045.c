#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, double c, int d, float e) {
    volatile double res = a * b / c + d + e;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    
    /* MANY LIVE SCALAR VARIABLES (high register pressure) */
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
    double d5 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    /* These may conflict with register allocator's choices */
    register int r1 asm("r12") = rand() % 50;  /* Try to bind to specific reg */
    register int r2 asm("r13") = rand() % 50;
    
    /* Array for complex addressing modes */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int iteration = 0; iteration < loop_limit; iteration++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS (extend live ranges) */
        /* Mix integer operations */
        a1 = a1 * a2 + a3 - a4;
        a2 = a2 ^ a5 | a6 & a7;
        a3 = (a3 << 2) | (a8 >> 3);
        a4 = a4 * a9 + a10;
        a5 = a5 - b1 * b2;
        
        /* Mixed integer/float operations (type conversions cause reloads) */
        f1 = (float)a1 + f2 * 3.14f;
        f2 = f3 - (float)a2 / 2.0f;
        
        /* Integer to float and float to int conversions */
        b1 = (int)f1 + a3;
        f3 = (float)b2 + d1;
        
        /* Double precision calculations */
        d1 = d1 * d2 + (double)a4;
        d2 = d3 / d4 - (double)f4;
        
        /* More type mixing */
        a6 = (int)d1 + (int)f2;
        f4 = (float)a7 + (float)((int)d2);
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int func_result = dummy_function(a1, a2, f1, d1, b3, f3);
        a7 = a7 + func_result;
        
        /* INLINE ASSEMBLY with MANY CLOBBERS */
        /* Forces compiler to save/restore registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* COMPLEX ADDRESSING MODES (non-offsettable) */
        /* Create addresses that might need reloading */
        int index1 = a1 % 50;
        int index2 = a2 % 50;
        
        /* Non-simple addressing: array[index + constant] */
        /* The combined offset may be too large for direct addressing */
        int val1 = array[index1 + 25];  /* May need address reload */
        int val2 = array[index2 + 30];  /* May need address reload */
        
        /* More complex pointer arithmetic */
        int* addr1 = &array[index1 + 15];
        int* addr2 = &array[index2 + 20];
        
        /* Use the pointers */
        a8 = *addr1 + *addr2;
        
        /* Different sized memory accesses */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        a9 = byte_ptr[index1] + short_ptr[index2] + array[index1 + 10];
        
        /* Another function call with mixed types */
        double dresult = complex_calc(d1, d2, d3, a10, f5);
        d4 = d4 + dresult;
        
        /* More type conversions and operations */
        f5 = (float)((int)d4 ^ (int)d5) + f1;
        d5 = (double)(a1 & a3) / (d2 + 1.0);
        
        /* Use explicit register variables in conflicting contexts */
        /* Try to use integer register variable in FP context */
        float temp_float = (float)r1 + f2;  /* May need reload */
        r2 = r2 + (int)temp_float;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += (double)a1 + (double)a2 + f1 + f2 + d1 + d2;
        
        /* More register pressure */
        b3 = b1 * b2 - b4 + b5;
        b4 = b3 ^ b5;
        b5 = b4 << 1 | b2 >> 1;
        
        /* Additional float/double mixing */
        f1 = f2 * f3 - f4 + f5;
        f2 = f3 / (f4 + 1.0f);
        
        d3 = d4 * d5 - d1;
        d4 = d2 / (d3 + 1.0);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %f\n", accumulator);
    printf("Final values: %d, %d, %f, %f\n", a1, b1, f1, d1);
    
    return 0;
}

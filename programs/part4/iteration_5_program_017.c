#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / (w + 1);
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
    
    /* MANY LIVE SCALAR VARIABLES - create register pressure */
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
    int b9 = rand() % 100;
    int b10 = rand() % 100;
    
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
    /* These may conflict with required register classes */
    register int r1 asm("r12") = rand() % 100;  /* Platform-specific reg */
    register int r2 asm("r13") = rand() % 100;  /* Platform-specific reg */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer that will be used with non-simple addresses */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer operations */
        a1 = a2 + a3 * a4 - a5;
        a2 = a6 & a7 | a8 ^ a9;
        a3 = (a10 << 2) + (b1 >> 3);
        a4 = b2 * b3 - b4 / (b5 + 1);
        a5 = b6 % (b7 + 1) + b8 * b9;
        
        /* Mixed integer/float operations - may require moves between reg files */
        f1 = (float)a1 + f2 * (float)a2;
        f2 = f3 - (float)(a3 * a4) / f4;
        
        /* Integer to float conversions */
        f3 = (float)b1 + (float)b2 * 0.5f;
        f4 = (float)(a5 & 0xFF) + f5;
        
        /* Float to double conversions */
        d1 = (double)f1 + d2 * (double)f2;
        d2 = d3 - (double)(f3 * f4) / d4;
        
        /* Double precision operations */
        d3 = d1 * d2 + d4 / d5;
        d4 = d5 - d3 * 0.5;
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads if used in FP contexts */
        int temp1 = r1 * r2;
        float temp2 = (float)r1 + f1;
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int call_result = dummy_func(a1, a2, f1, d1, b1, b2);
        
        /* INLINE ASSEMBLY with many clobbers - increases register pressure */
        /* This tells GCC these registers are unusable */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7"
        );
        
        /* MORE COMPLEX EXPRESSIONS after clobber */
        b1 = b3 + b4 * b5 - b6;
        b2 = b7 & b8 | b9 ^ b10;
        b3 = (a1 << 3) + (a2 >> 2);
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Array access with complex index calculation */
        int index = (a1 + a2 * 3 - a3) & 0xFF;
        
        /* This creates addressing like array[complex_expression + constant] */
        /* The combined offset may be too complex for direct addressing */
        int val1 = array[index + 16];  /* May need address reload */
        int val2 = array[index + 32];  /* May need address reload */
        int val3 = array[index + 64];  /* May need address reload */
        
        /* Pointer arithmetic with non-simple offsets */
        int* ptr1 = ptr + index + 8;
        int* ptr2 = ptr + index * 2 + 16;
        int* ptr3 = ptr + (index << 1) + 32;
        
        /* Different sized memory accesses */
        char* char_ptr = (char*)ptr;
        short* short_ptr = (short*)ptr;
        
        /* Mixed size accesses in same expression */
        int combined = *ptr1 + (int)(*char_ptr) + (int)(*short_ptr);
        
        /* Type conversions and mixed operations */
        double mixed = (double)val1 * f1 + (double)val2 * d1;
        float mixed2 = (float)val3 * 0.5f + f2;
        
        /* Another function call with mixed types */
        double dresult = complex_op(a1, f1, d1, b1);
        
        /* Update accumulator to prevent dead code elimination */
        accumulator += (double)call_result + mixed + dresult + 
                      (double)combined + (double)temp1 + temp2;
        
        /* More register pressure */
        a6 = a7 + a8 * a9 - a10;
        a7 = b1 & b2 | b3 ^ b4;
        f5 = (float)a6 + f1 * (float)a7;
        d5 = (double)f5 + d1 * (double)f2;
        
        /* Use all variables to keep them live */
        v1 += a1 + a2 + a3 + a4 + a5;
        v2 += f1 + f2 + f3 + f4 + f5;
        v3 += d1 + d2 + d3 + d4 + d5;
        v4 += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
    }
    
    /* Final output to prevent optimization */
    printf("Result: %f\n", accumulator);
    printf("Checksum: %d %f %f %d\n", v1, v2, v3, v4);
    
    return 0;
}

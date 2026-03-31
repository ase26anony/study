#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
float float_ops(float a, float b, float c, float d, float e) {
    volatile float sum = a + b + c + d + e;
    return sum * 0.5f;
}

int main(void) {
    /* Seed random for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    
    /* MANY LIVE SCALAR VARIABLES - high register pressure */
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
    
    /* More integer variables */
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
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    float f8 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int r1 asm("r12") = a1 + a2;  /* Use r12 if available */
    register int r2 asm("r13") = a3 + a4;  /* Use r13 if available */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        
        /* Integer expressions with bitwise and arithmetic ops */
        int t1 = (a1 & b1) + (a2 | b2) * (a3 ^ b3);
        int t2 = (a4 << 2) + (a5 >> 1) * (b4 & 0xFF);
        int t3 = (a6 * b5) + (a7 / (b6 + 1)) - (a8 % (b7 + 1));
        int t4 = (a9 ^ b8) | (a10 & b9) << (b10 & 3);
        
        /* Mixed integer/float conversions */
        f1 = (float)t1 + f2 * 2.0f;
        f3 = (float)t2 / (f4 + 1.0f);
        f5 = f6 + (float)(t3 * t4);
        
        /* Double precision operations */
        d1 = (double)t1 + d2 * 2.0;
        d3 = (double)t2 / (d4 + 1.0);
        
        /* FUNCTION CALL - clobbers registers */
        int func_result = dummy_function(t1, t2, f1, d1, t3, t4);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* This tells GCC many registers are unavailable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11",
              "f0", "f1", "f2", "f3", "f4", "f5"
        );
        
        /* MORE COMPLEX EXPRESSIONS after asm clobber */
        float f_result = float_ops(f1, f2, f3, f4, f5);
        
        /* NON-OFFSETTABLE MEMORY ADDRESSING */
        /* Complex address calculation that may need reloading */
        int index = (t1 + t2) & 0xFF;
        int offset = 17;  /* Non-simple offset */
        
        /* This address calculation: array[(index + offset) & 0xFF] */
        /* May require the sum to be in a register on some archs */
        int mem_val = array[(index + offset) & 0xFF];
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads if used in floating point context */
        float mixed1 = (float)r1 + f_result;
        double mixed2 = (double)r2 * d3;
        
        /* DIFFERENT SIZED MEMORY ACCESSES */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        int byte_sum = byte_ptr[index] + short_ptr[index] + array[index];
        
        /* Update many variables to keep them live */
        a1 = t1 + mem_val;
        a2 = t2 + byte_sum;
        a3 = t3 + func_result;
        a4 = t4 + (int)f_result;
        b1 = b1 ^ t1;
        b2 = b2 | t2;
        b3 = b3 & t3;
        
        /* Type conversions that may require move between reg files */
        f2 = (float)a1 + (float)b1 * 0.5f;
        f4 = (float)a2 / (float)(b2 + 1);
        d2 = (double)a3 + (double)b3 * 0.5;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += t1 + t2 + (int)f_result + mem_val + byte_sum;
        
        /* Use pointer arithmetic with non-simple offsets */
        ptr = array + ((index * 3 + 7) & 0xFF);
        int ptr_val = *ptr;
        
        /* More register pressure */
        a5 = a5 + ptr_val;
        a6 = a6 - ptr_val;
        a7 = a7 * (ptr_val + 1);
        a8 = a8 / (ptr_val + 1);
        
        /* Another function call */
        dummy_function(a1, a2, f2, d2, a3, a4);
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r12", "r13", "r14", "r15",
              "f6", "f7", "f8", "f9", "f10"
        );
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", accumulator);
    
    /* Use all variables one more time */
    int final_sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                   b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                   (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

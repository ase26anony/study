#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed random for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    volatile int v4 = rand();
    volatile int v5 = rand();
    
    /* MANY LIVE SCALAR VARIABLES - create register pressure */
    int a1 = rand(), a2 = rand(), a3 = rand(), a4 = rand(), a5 = rand();
    int b1 = rand(), b2 = rand(), b3 = rand(), b4 = rand(), b5 = rand();
    int c1 = rand(), c2 = rand(), c3 = rand(), c4 = rand(), c5 = rand();
    float f1 = (float)rand()/RAND_MAX, f2 = (float)rand()/RAND_MAX;
    float f3 = (float)rand()/RAND_MAX, f4 = (float)rand()/RAND_MAX;
    double d1 = (double)rand()/RAND_MAX, d2 = (double)rand()/RAND_MAX;
    double d3 = (double)rand()/RAND_MAX, d4 = (double)rand()/RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    register int r1 asm("r15") = rand();  /* May conflict with compiler's use */
    register int r2 asm("r14") = rand();
    
    /* Array for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    /* Pointer for non-offsettable addressing */
    int *ptr = array + 128;
    
    volatile int accumulator = 0;
    volatile int loop_limit = 100;  /* Prevent loop unrolling */
    
    /* LOOP with invariant spilling */
    for (int iteration = 0; iteration < loop_limit; iteration++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        int t1 = a1 * b1 + c1 * iteration;
        int t2 = a2 ^ b2 | c2 & iteration;
        int t3 = (a3 << 2) + (b3 >> 3) * c3;
        
        /* MIXED TYPE OPERATIONS - force register class changes */
        float ft1 = f1 * (float)t1 + f2;
        double dt1 = d1 * (double)ft1 + d2 * (double)t2;
        
        /* INTEGER TO FLOAT CONVERSIONS */
        f3 = (float)(t1 * t2) / 1024.0f;
        d3 = (double)(t2 ^ t3) / 65536.0;
        
        /* COMPLEX ADDRESSING with non-simple offset */
        /* array[index + constant] where offset may be too large */
        int idx = iteration * 8 + 16;
        int val1 = array[idx + 64];      /* Large offset */
        int val2 = array[idx - 32];      /* Negative offset */
        int val3 = ptr[iteration * 4];   /* Pointer with scaled index */
        
        /* BITWISE and ARITHMETIC COMBINATIONS */
        a4 = ((val1 & 0xFF) << 8) | ((val2 & 0xFF00) >> 8);
        b4 = (val3 * 37 + 12345) ^ a4;
        
        /* FUNCTION CALL - clobbers registers */
        int call_result = dummy_func(t1, t2, ft1, dt1, a4, b4);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS */
        /* Force compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11", "r12"
        );
        
        /* MORE COMPLEX CALCULATIONS after clobber */
        double dt2 = d2 * (double)call_result + d3 * (double)val1;
        float ft2 = f2 * (float)dt2 + f3 * (float)val2;
        
        /* TYPE CONVERSIONS and DIFFERENT SIZED ACCESSES */
        char char_val = (char)(val3 & 0xFF);
        short short_val = (short)(val2 & 0xFFFF);
        int int_val = val1;
        double double_val = (double)char_val * (double)short_val * dt2;
        
        /* Use explicit register variables in conflicting context */
        /* Using integer register in floating-point calculation */
        float mixed_calc = (float)r1 * ft2 + (float)r2 * 0.5f;
        
        /* More pointer arithmetic */
        int *ptr2 = array + (iteration & 0x3F) * 2;
        int val4 = ptr2[8];  /* Non-zero offset */
        int val5 = ptr2[-4]; /* Negative offset */
        
        /* Update many variables to keep them live */
        a1 = a2 + t1;
        a2 = a3 ^ t2;
        a3 = a4 | t3;
        b1 = b2 * val1;
        b2 = b3 + val2;
        b3 = b4 - val3;
        c1 = c2 & val4;
        c2 = c3 | val5;
        c3 = c4 ^ call_result;
        
        /* DIFFERENT TYPE SIZES in single expression */
        accumulator += (int)(ft1 * 100.0f) + 
                      (int)(dt1 * 10.0) + 
                      char_val + 
                      short_val +
                      (int)(mixed_calc * 2.0f);
        
        /* Prevent dead code elimination */
        if (accumulator > 1000000) {
            accumulator = accumulator % 1000000;
        }
    }
    
    /* Use all variables one more time to extend live ranges */
    int final_result = 
        a1 + a2 + a3 + a4 + a5 +
        b1 + b2 + b3 + b4 + b5 +
        c1 + c2 + c3 + c4 + c5 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 +
        r1 + r2;
    
    printf("Result: %d (accumulator: %d)\n", final_result, accumulator);
    return 0;
}

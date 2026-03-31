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
float float_ops(float a, float b, float c, float d, float e) {
    volatile float sum = a + b + c + d + e;
    return sum * 0.5f;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    
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
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    float f8 = (float)rand() / RAND_MAX;
    
    /* Double precision variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES - bind to specific registers */
    /* These create conflicts when used in different contexts */
    register int r_ax asm("rax") = rand() % 100;
    register int r_bx asm("rbx") = rand() % 100;
    register int r_cx asm("rcx") = rand() % 100;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand() % 1000;
    }
    
    /* Pointer that will be used with non-offsettable addresses */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer operations */
        int t1 = a1 + a2 * a3 - a4 / (a5 + 1);
        int t2 = b1 & b2 | b3 ^ b4 << 2;
        int t3 = (a6 * a7) + (a8 & a9) - (a10 | b5);
        
        /* Mix float and integer operations */
        f1 = (float)t1 * 0.5f + f2 - (float)t2 * 0.25f;
        f3 = f4 * f5 + (float)(b6 * b7) / 100.0f;
        
        /* Integer to float conversions */
        f6 = (float)(t3 & 0xFF) + f7 * 2.0f;
        f8 = (float)((a1 ^ a2) | (a3 & a4)) / 256.0f;
        
        /* Double precision operations */
        d1 = (double)t1 + d2 * 0.5;
        d3 = (double)f1 + d4 * (double)(t2 % 100);
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int func_result = dummy_func(t1, t2, f1, d1, a1, b1);
        
        /* INLINE ASSEMBLY WITH CLOBBERS - force register spills */
        /* This tells GCC these registers are unusable */
        asm volatile(
            "# Dummy assembly to clobber registers\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* MORE COMPLEX EXPRESSIONS after clobber */
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads due to register class conflicts */
        float float_from_int = (float)r_ax + f2;  /* Integer reg in float op */
        int int_from_float = (int)f3 + r_bx;      /* Float reg in integer op */
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* Create addresses that might not be directly usable */
        int index = (r_cx + loop) & 0xFF;
        
        /* Non-simple address: array[index + constant] where constant 
           might be too large for direct addressing */
        int offset = 64;  /* Large enough to cause issues on some archs */
        int val1 = array[index + offset];      /* May need address reload */
        int val2 = array[index + offset * 2];  /* More complex */
        int val3 = array[index + offset * 3];  /* Even more complex */
        
        /* Mixed size memory accesses */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Different sized accesses in same expression */
        int mixed_access = byte_ptr[index] + short_ptr[index] + array[index];
        
        /* More type conversions */
        double d_from_mixed = (double)val1 + (double)f1 + (double)int_from_float;
        float f_from_double = (float)d_from_mixed + float_from_int;
        
        /* Another function call with mixed types */
        float float_res = float_ops(f1, f2, f3, f4, f_from_double);
        
        /* Bitwise and arithmetic combinations */
        int complex_int = (val1 & val2) | (val3 ^ mixed_access);
        complex_int = complex_int + (t1 << 2) - (t2 >> 1);
        complex_int = complex_int * (func_result % 100) / (val1 + 1);
        
        /* Update all variables to keep them live */
        a1 = a2 + val1;
        a2 = a3 ^ val2;
        a3 = a4 | val3;
        a4 = a5 + complex_int;
        a5 = a6 - mixed_access;
        
        b1 = b2 & func_result;
        b2 = b3 | int_from_float;
        b3 = b4 ^ complex_int;
        b4 = b5 + val1;
        b5 = b6 - val2;
        
        f2 = f1 * 0.9f + float_res;
        f4 = f3 / 1.1f + (float)complex_int;
        f6 = f5 + f_from_double - float_from_int;
        
        d2 = d1 * 0.99 + (double)complex_int;
        d4 = d3 / 1.01 - d_from_mixed;
        
        /* Update explicit register variables */
        r_ax = r_ax + loop + val1;
        r_bx = r_bx ^ loop ^ val2;
        r_cx = r_cx * (loop + 1) + val3;
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += val1 + val2 + val3 + complex_int + (int)float_res;
        
        /* Use volatile variable in loop to prevent optimization */
        if (v1 > 1000000) {  /* Unlikely condition */
            loop_limit = 50;  /* Modifies loop limit */
        }
    }
    
    /* Final output to prevent entire program from being optimized away */
    printf("Result: %d\n", accumulator);
    printf("Final values: %d %f %lf\n", a1, f1, d1);
    printf("Register vars: %d %d %d\n", r_ax, r_bx, r_cx);
    
    return 0;
}

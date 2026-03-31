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
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / w;
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
    
    /* MANY LIVE SCALAR VARIABLES (20+ variables) */
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
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX;
    
    /* More variables to increase pressure */
    int b1 = rand() % 100;
    int b2 = rand() % 100;
    int b3 = rand() % 100;
    int b4 = rand() % 100;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    
    /* EXPLICIT REGISTER VARIABLES with potential conflicts */
    /* These may bind to specific registers causing conflicts */
    register int r1 asm("eax") = a1 + a2;
    register int r2 asm("ebx") = a3 + a4;
    register float r3 asm("xmm0") = f1 + f2;
    register double r4 asm("xmm1") = d1 + d2;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS */
        /* Mix integer and float operations */
        a1 = a2 * a3 + a4 - a5;
        f1 = (float)a1 / (f2 + 1.0f);
        d1 = (double)f1 * d2 - d3;
        
        /* Type conversions that require register moves */
        f3 = (float)(a6 + a7);
        a8 = (int)(f4 * 100.0f);
        d4 = (double)a9 + d5;
        
        /* Bitwise and arithmetic combinations */
        b1 = (a1 & a2) | (a3 << 2) + (a4 * 3);
        b2 = (b1 ^ a5) + (a6 >> 1) * (a7 & 0xFF);
        
        /* Mixed-size memory accesses */
        char char_val = (char)(array[a1 % 256] & 0xFF);
        short short_val = (short)(array[a2 % 256] & 0xFFFF);
        int int_val = array[a3 % 256];
        
        /* Non-offsettable memory address calculation */
        /* This often requires reloading the address */
        int index = a4 + a5 + loop;
        int offset = index * sizeof(int) + 16; /* Large offset */
        int complex_addr_val = *(int*)((char*)ptr + offset);
        
        /* FUNCTION CALL - clobbers registers */
        int func_result = dummy_func(a1, a2, f1, d1, b1, b2);
        
        /* INLINE ASSEMBLY with many clobbers */
        /* Forces compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            "nop\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory", "cc"
        );
        
        /* More complex calculations after assembly */
        f5 = f3 * f4 + (float)func_result;
        d5 = d3 / d4 + (double)(a8 * a9);
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads due to register class conflicts */
        float temp_float = r3 + (float)r1; /* Integer reg in float op */
        double temp_double = r4 * (double)r2; /* Integer reg in double op */
        
        /* Another function call with mixed types */
        double complex_result = complex_op(r1, r3, r4, r2);
        
        /* More pointer arithmetic with complex addressing */
        int* complex_ptr = ptr + (a1 * a2 + a3) / 4;
        int val1 = complex_ptr[0];
        int val2 = complex_ptr[16]; /* Non-simple offset */
        int val3 = complex_ptr[32]; /* Another non-simple offset */
        
        /* Update accumulator to prevent dead code elimination */
        accumulator += (double)a1 + f1 + d1 + (double)func_result + 
                      temp_double + complex_result + (double)(val1 + val2 + val3);
        
        /* More register-intensive operations */
        a10 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + b1 + b2 + b3 + b4;
        f6 = f1 + f2 + f3 + f4 + f5 + (float)(a10 % 100);
        d6 = d1 + d2 + d3 + d4 + d5 + (double)(a10 / 100);
        
        /* Another inline assembly to increase pressure */
        asm volatile (
            "# More register clobbering\n"
            "nop\n"
            "nop\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "memory"
        );
        
        /* Final mixed-type computation */
        accumulator += (double)((a1 & 0xF) << 4) + 
                      (double)((float)(a2 | 0xF0) * 0.5f) +
                      (double)((a3 ^ a4) + (a5 * a6)) +
                      f6 * 2.0f + d6 / 3.0;
    }
    
    /* Print result to prevent optimization */
    printf("Final accumulator: %f\n", accumulator);
    printf("Variables: %d %f %f %d\n", a1, f1, d1, b1);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double x, double y, int z, float w) {
    volatile double res = x * y + (double)z / (double)w;
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
    
    /* MANY LIVE SCALAR VARIABLES (20+) to create register pressure */
    /* Integer variables */
    register int r0 asm("r0") = rand() % 100;  /* Try to bind to specific reg */
    int i1 = rand() % 100;
    int i2 = rand() % 100;
    int i3 = rand() % 100;
    int i4 = rand() % 100;
    int i5 = rand() % 100;
    int i6 = rand() % 100;
    int i7 = rand() % 100;
    int i8 = rand() % 100;
    int i9 = rand() % 100;
    int i10 = rand() % 100;
    
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
    
    /* More variables for additional pressure */
    int i11 = rand() % 100;
    int i12 = rand() % 100;
    int i13 = rand() % 100;
    int i14 = rand() % 100;
    float f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX;
    double d6 = (double)rand() / RAND_MAX;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int j = 0; j < 256; j++) {
        array[j] = rand() % 1000;
    }
    
    /* Pointer variables */
    int *ptr1 = &array[0];
    int *ptr2 = &array[128];
    
    /* Volatile loop counter to prevent loop optimizations */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all variable types */
        /* This creates many intermediate values needing registers */
        
        /* Integer expressions with bitwise and arithmetic ops */
        i1 = ((i1 * i2) & (i3 | i4)) + ((i5 << 2) | (i6 >> 3));
        i2 = (i7 ^ i8) * (i9 + i10) - (i11 & i12);
        i3 = (i13 | i14) + (i1 * i2) - (i3 ^ i4);
        
        /* Mixed integer/float conversions */
        f1 = (float)i1 + f2 * (float)i3;
        f2 = (float)(i2 & i5) / f3 + f4;
        
        /* Float to double promotions */
        d1 = (double)f1 * d2 + (double)f3;
        d2 = d3 / (double)f2 - d4;
        
        /* Double precision calculations */
        d3 = d1 * d2 + d5 / d4;
        d4 = d6 * 3.14159 + d2;
        
        /* More integer work */
        i4 = (i1 + i2) * (i3 - i4) | (i5 & i6);
        i5 = (i7 << i8) + (i9 >> i10) ^ i11;
        
        /* FUNCTION CALL - forces register saves/restores */
        int func_result = dummy_func(i1, i2, f1, d1, i3, f3);
        
        /* INLINE ASSEMBLY with many clobbered registers */
        /* This tells GCC these registers are unavailable */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7"
        );
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = i1 % 128;
        int idx2 = i2 % 128;
        
        /* Non-simple addressing: array[index + constant] where offset might be too large */
        int val1 = array[idx1 + 64];  /* May need address reload */
        int val2 = array[idx2 + 96];  /* May need address reload */
        
        /* More complex: array[(index1 + index2) * 2] */
        int val3 = array[((idx1 + idx2) * 2) % 256];
        
        /* Pointer arithmetic with non-simple offsets */
        int *ptr3 = ptr1 + (i3 % 64);
        int *ptr4 = ptr2 + (i4 % 64);
        
        /* Dereference with potential reloads */
        i6 = *ptr3 + *ptr4;
        i7 = *(ptr3 + 8) + *(ptr4 - 8);
        
        /* MIXED TYPE OPERATIONS in single expressions */
        /* These can cause mode mismatches requiring reloads */
        d5 = d1 + (double)i1 + (double)((float)i2 * f1);
        f4 = f2 + (float)i3 + (float)(d2 * (double)i4);
        
        /* Type conversions and size changes */
        char c1 = (char)(i1 & 0xFF);
        short s1 = (short)(i2 & 0xFFFF);
        i8 = (int)c1 * (int)s1 + i5;
        
        /* Another function call with different types */
        double dresult = complex_calc(d1, d2, i1, f1);
        
        /* More inline asm to clobber registers between uses */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "rax", "rbx", "rcx", "rdx", "rdi", "rsi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Use the register-bound variable in conflicting context */
        /* r0 is bound as integer register, use in float context */
        float temp_float = (float)r0 * f1;
        r0 = (int)temp_float + i1;  /* Convert back */
        
        /* More complex expressions keeping variables live */
        i9 = (val1 * val2) + (i6 ^ i7) - (i8 & i9);
        i10 = (i11 << 2) | (i12 >> 3) + (i13 * i14);
        
        f5 = f3 * f4 + (float)(val3 / (i1 + 1));
        f6 = (float)i2 / f5 + f7;
        
        d6 = d3 * d4 + d5 / (double)(i3 + 1);
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += (double)i1 + (double)i2 + (double)f1 + d1 + 
                      (double)val1 + (double)val2 + dresult;
        
        /* Mix in volatile variables to prevent reordering */
        i11 = v1 + i1;
        f7 = v2 + f1;
        d1 = v3 + d6;
    }
    
    /* Final output to prevent entire program from being optimized away */
    printf("Result: %f\n", accumulator);
    printf("Final values: i1=%d, f1=%f, d1=%f\n", i1, f1, d1);
    
    return 0;
}

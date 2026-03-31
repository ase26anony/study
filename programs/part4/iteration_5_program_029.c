#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline))
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double complex_calc(double a, double b, int c, float d) {
    volatile double res = (a * b) / (c + d);
    return res;
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
    register int r0 asm("r0") = rand();  /* Try to bind to specific reg */
    int i1 = rand(), i2 = rand(), i3 = rand(), i4 = rand();
    int i5 = rand(), i6 = rand(), i7 = rand(), i8 = rand();
    int i9 = rand(), i10 = rand(), i11 = rand(), i12 = rand();
    int i13 = rand(), i14 = rand(), i15 = rand(), i16 = rand();
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX, f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX, f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX, f6 = (float)rand() / RAND_MAX;
    float f7 = (float)rand() / RAND_MAX, f8 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double d1 = (double)rand() / RAND_MAX, d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX, d4 = (double)rand() / RAND_MAX;
    double d5 = (double)rand() / RAND_MAX, d6 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[100];
    for (int j = 0; j < 100; j++) {
        array[j] = rand();
    }
    int *ptr1 = &array[0];
    int *ptr2 = &array[50];
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_limit = 100;
    volatile double accumulator = 0.0;
    
    /* MAIN LOOP - creates sustained register pressure */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        i1 = i2 + i3 * i4 - i5 / (i6 + 1);
        i2 = i7 ^ i8 | i9 & i10;
        i3 = (i11 << 3) | (i12 >> 2);
        i4 = i13 * i14 - i15 + i16;
        
        /* MIXED TYPE OPERATIONS - force register class changes */
        f1 = (float)i1 + f2 * 3.14f - (float)i2;
        f3 = f4 / f5 + (float)(i3 & 0xFF);
        d1 = (double)f6 + d2 * 2.71828 - (double)i4;
        d3 = d4 / d5 + (double)(i5 | 0x0F);
        
        /* INTEGER TO FLOAT CONVERSIONS */
        f7 = (float)(i6 * i7) / 256.0f;
        d6 = (double)(i8 ^ i9) / 65536.0;
        
        /* FLOAT TO INTEGER CONVERSIONS */
        i10 = (int)(f1 * 100.0f) + (int)d1;
        i11 = (int)(f2 * 1000.0f) ^ (int)(d2 * 10.0);
        
        /* COMPLEX ADDRESSING MODES - non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = i12 & 0x0F;
        int idx2 = i13 & 0x1F;
        int val1 = array[idx1 * 3 + 7];  /* Non-simple offset */
        int val2 = array[idx2 * 5 + 13]; /* Another non-simple offset */
        
        i14 = val1 * val2 + array[(idx1 + idx2) * 2];
        
        /* FUNCTION CALL - clobbers caller-saved registers */
        int func_result = dummy_function(i1, i2, f1, d1, i3, f3);
        i15 = i15 + func_result;
        
        /* INLINE ASSEMBLY WITH CLOBBERS - increase register pressure */
        /* Clobber multiple registers to force spills */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7"
        );
        
        /* MORE COMPLEX EXPRESSIONS AFTER ASM */
        f4 = f5 * f6 + (float)(i14 >> 4);
        d4 = d5 * d6 - (double)(i15 & 0xFF);
        
        /* BITWISE AND ARITHMETIC COMBINATIONS */
        i16 = (i1 + i2) & (i3 | i4) ^ (i5 * i6);
        
        /* TYPE SIZE MIXING - different sized accesses */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        i12 = byte_ptr[i7 & 0x3F] + short_ptr[(i8 & 0x1F) * 2];
        
        /* SECOND FUNCTION CALL with different types */
        double d_result = complex_calc(d1, d2, i9, f7);
        accumulator += d_result;
        
        /* POINTER ARITHMETIC with complex expressions */
        ptr1 = &array[(i10 + i11) & 0x3F];
        ptr2 = &array[(i12 * 2 + i13) & 0x3F];
        
        /* MEMORY ACCESS through computed pointers */
        i13 = *ptr1 + *ptr2;
        
        /* VOLATILE UPDATE - prevent dead code elimination */
        v1 = v1 + i1 + i2 + i3;
        v2 = v2 + f1 + f2;
        v3 = v3 + d1 + d2;
        
        /* Use the register variable in conflicting context */
        /* This may force reloads due to register class conflicts */
        float temp_float = (float)r0;  /* Integer reg used for float */
        r0 = (int)temp_float + 1;      /* And back */
        
        /* Another inline asm with fewer clobbers but between critical ops */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1"
        );
    }
    
    /* FINAL OUTPUT to prevent complete optimization */
    printf("Result: v1=%d, v2=%f, v3=%f, acc=%f\n", 
           v1, v2, v3, accumulator);
    printf("i1=%d, i16=%d, f8=%f, d6=%f\n", i1, i16, f8, d6);
    
    return 0;
}

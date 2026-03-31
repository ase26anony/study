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
float float_ops(float a, double b, int c, float d) {
    volatile float res = a * (float)b + (float)c - d;
    return res;
}

int main(void) {
    /* SEED RANDOM FOR VARIABILITY */
    srand(42);
    
    /* VOLATILE VARIABLES TO PREVENT OPTIMIZATION */
    volatile int vi1 = rand() % 100;
    volatile int vi2 = rand() % 100;
    volatile float vf1 = (float)(rand() % 100) / 10.0f;
    volatile double vd1 = (double)(rand() % 100) / 5.0;
    
    /* MANY LIVE SCALAR VARIABLES - HIGH REGISTER PRESSURE */
    /* Integer variables */
    int i1 = vi1 + 1;
    int i2 = vi2 * 2;
    int i3 = i1 + i2;
    int i4 = i2 - i1;
    int i5 = i3 * i4;
    int i6 = i5 / (i1 + 1);
    int i7 = i6 << 2;
    int i8 = i7 | 0xFF;
    int i9 = i8 & 0xF0;
    int i10 = i9 ^ i8;
    int i11 = i10 + i9;
    int i12 = i11 - i10;
    int i13 = i12 * 3;
    int i14 = i13 / 2;
    int i15 = i14 % 7;
    
    /* Floating point variables */
    float f1 = vf1 + 1.5f;
    float f2 = f1 * 2.0f;
    float f3 = f2 - f1;
    float f4 = f3 / 2.0f;
    float f5 = f4 + 3.14f;
    float f6 = f5 * f4;
    float f7 = f6 - 3.0f;
    float f8 = f7 / 1.5f;
    
    /* Double variables */
    double d1 = vd1 + 2.5;
    double d2 = d1 * 1.5;
    double d3 = d2 - d1;
    double d4 = d3 / 2.0;
    double d5 = d4 + 1.618;
    double d6 = d5 * d4;
    
    /* EXPLICIT REGISTER VARIABLES WITH POTENTIAL CONFLICTS */
    /* These may conflict with compiler's register allocation */
    register int r1 asm("eax") = i1 + i2;
    register int r2 asm("ebx") = i3 + i4;
    register float rf1 asm("xmm0") = f1 + f2;
    
    /* ARRAY FOR COMPLEX ADDRESSING MODES */
    int array[256];
    for (int idx = 0; idx < 256; idx++) {
        array[idx] = rand() % 1000;
    }
    
    /* VOLATILE LOOP COUNTER TO PREVENT LOOP OPTIMIZATIONS */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MAIN LOOP CREATING REGISTER PRESSURE */
    for (volatile int loop = 0; loop < loop_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - EXTEND LIVE RANGES */
        /* Mix integer operations */
        int t1 = i1 + i2 - i3 * i4;
        int t2 = (i5 & i6) | (i7 ^ i8);
        int t3 = (i9 << 3) + (i10 >> 2);
        int t4 = t1 * t2 - t3;
        
        /* Mix float operations with integer conversions */
        float ft1 = (float)t1 + f1;
        float ft2 = ft1 * (float)t2;
        float ft3 = (float)(t3 & 0xFF) + f2;
        
        /* Mix double operations */
        double dt1 = (double)t4 + d1;
        double dt2 = dt1 * d2 - (double)t1;
        
        /* TYPE CONVERSIONS FORCING REGISTER MOVES */
        i1 = (int)ft1 + t1;
        f1 = (float)i2 + ft2;
        d1 = (double)i3 + dt1;
        
        /* COMPLEX ADDRESSING MODES - NON-OFFSETTABLE */
        /* These addresses may require reloads */
        int idx1 = (i4 + i5) & 0xFF;
        int idx2 = (i6 * 2 + 7) & 0xFF;
        int idx3 = (i7 + 256) & 0xFF;  /* Large constant offset */
        
        /* Memory accesses with complex addressing */
        int mem1 = array[idx1] + array[idx2];
        int mem2 = array[idx3] - array[(idx1 + idx2) & 0xFF];
        
        /* Use explicit register variables in conflicting contexts */
        /* Force integer register to be used in FP context */
        float mixed1 = (float)r1 + rf1;
        int mixed2 = r2 + (int)rf1;
        
        /* FUNCTION CALLS TO CLOBBER REGISTERS */
        int call_result = dummy_func(i1, i2, f1, d1, i3, i4);
        float float_result = float_ops(f2, d2, i5, f3);
        
        /* INLINE ASSEMBLY WITH MANY CLOBBERED REGISTERS */
        /* Forces compiler to save/restore around asm */
        asm volatile (
            "# Dummy assembly\n"
            : 
            : "r"(i1), "r"(i2), "r"(i3)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* MORE COMPLEX EXPRESSIONS AFTER ASM */
        /* All variables still live */
        i2 = mem1 + call_result;
        i3 = mem2 - (int)float_result;
        f2 = mixed1 * 2.0f;
        d2 = (double)mixed2 / 3.0;
        
        /* BITWISE AND ARITHMETIC COMBINATIONS */
        i4 = (t1 & 0xF0F0) + (t2 | 0x0F0F) - (t3 ^ 0xAAAA);
        i5 = (i4 << 2) * (i4 >> 1) + (i4 & 0xFF);
        
        /* UPDATE VOLATILE ACCUMULATOR TO PREVENT DEAD CODE ELIMINATION */
        accumulator += i1 + i2 + i3 + (int)f1 + (int)d1;
        
        /* MIXED SIZE MEMORY ACCESSES */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        /* Different sized accesses in same expression */
        int byte_sum = byte_ptr[idx1] + byte_ptr[idx2];
        int short_sum = short_ptr[idx3] + short_ptr[idx1];
        i6 = byte_sum * 2 + short_sum / 2;
        
        /* ANOTHER FUNCTION CALL */
        call_result = dummy_func(i6, i7, f3, d3, i8, i9);
        
        /* MORE TYPE CONVERSIONS */
        f3 = (float)i10 + (float)call_result / 10.0f;
        d3 = (double)i11 + (double)call_result / 20.0;
        
        /* USE ALL REMAINING VARIABLES */
        i7 = i12 + i13 - i14;
        i8 = i15 * 2 + accumulator % 100;
        f4 = f5 + f6 - f7;
        d4 = d5 * d6 + 1.0;
        
        /* FINAL COMPLEX EXPRESSION USING EVERYTHING */
        accumulator += (i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
                       i11 + i12 + i13 + i14 + i15 +
                       (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                       (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                       (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                       (int)d5 + (int)d6);
    }
    
    printf("Final accumulator: %d\n", accumulator);
    printf("Values: i1=%d, f1=%.2f, d1=%.2f\n", i1, f1, d1);
    
    return 0;
}

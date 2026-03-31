#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / w;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int v_limit = 100;
    volatile int v_acc = 0;
    volatile float v_facc = 0.0f;
    volatile double v_dacc = 0.0;
    
    /* MANY LIVE SCALAR VARIABLES (25+ variables) */
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
    
    /* More integers with volatile qualifier */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    
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
    /* Try to bind to specific registers (compiler may ignore but tries) */
    register int r_ax asm("ax") = a1 + a2;  /* May conflict with float ops */
    register int r_bx asm("bx") = a3 * a4;  /* May conflict with address calc */
    register float r_f0 asm("xmm0") = f1 + f2;  /* Float reg variable */
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with high register pressure */
    for (volatile int loop = 0; loop < v_limit; loop++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS mixing all types */
        
        /* Integer expressions with bitwise and arithmetic ops */
        int t1 = (a1 & a2) | (a3 << 2) + (a4 * a5) - (a6 ^ a7);
        int t2 = ((a8 + a9) * (a10 - v1)) / (v2 | 1) + (v3 & 0xFF);
        
        /* Float/double conversions and operations */
        float ft1 = (float)t1 + f1 * f2 - (float)t2 / f3;
        double dt1 = (double)ft1 * d1 + d2 / (double)(t1 + 1);
        
        /* Mixed type conversions */
        float ft2 = (float)((int)d1 + (int)d2) * f4;
        double dt2 = d3 + (double)((int)f5 * a1);
        
        /* Use explicit register variables in conflicting contexts */
        int t3 = r_ax * 2 + r_bx / 3;  /* Using integer register vars */
        float ft3 = r_f0 * 2.5f + (float)r_ax;  /* Mixing reg var types - may need reload */
        
        /* NON-OFFSETTABLE MEMORY ADDRESSING */
        /* Complex address calculation that may not fit in displacement */
        int idx = (t1 + t2 + loop) & 0xFF;
        int val1 = array[idx + 64];  /* idx+64 may be non-offsettable */
        int val2 = *(ptr + idx - 32);  /* Another complex address */
        
        /* FUNCTION CALL to clobber registers */
        int call_result = dummy_func(t1, t2, ft1, dt1, val1, ft2);
        
        /* INLINE ASSEMBLY with multiple clobbers */
        /* Force compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory"
        );
        
        /* MORE COMPLEX EXPRESSIONS after asm clobber */
        /* All variables need to be reloaded after asm */
        double dt3 = complex_op(call_result, ft3, dt2, val2);
        
        /* Different sized memory accesses */
        char* byte_ptr = (char*)array;
        short* short_ptr = (short*)array;
        
        /* Mixed size accesses in same expression */
        int byte_sum = byte_ptr[idx] + byte_ptr[idx + 1] + 
                      short_ptr[idx / 2] + array[idx];
        
        /* Update volatile accumulators to prevent elimination */
        v_acc += t1 + t2 + call_result + byte_sum;
        v_facc += ft1 + ft2 + ft3 + (float)dt1;
        v_dacc += dt1 + dt2 + dt3;
        
        /* Modify some variables to extend live ranges */
        a1 = (a1 + t1) & 0xFFF;
        a2 = (a2 ^ t2) | 1;
        f1 = f1 * 0.99f + ft1 * 0.01f;
        d1 = d1 * 0.99 + dt1 * 0.01;
        
        /* Update explicit register variables */
        r_ax = r_ax + loop;
        r_bx = r_bx * 2 - loop;
        r_f0 = r_f0 * 1.1f;
        
        /* Another asm with different clobbers */
        asm volatile (
            "# More register clobbering\n"
            :
            :
            : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "memory"
        );
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d, %f, %f\n", v_acc, v_facc, v_dacc);
    
    return 0;
}

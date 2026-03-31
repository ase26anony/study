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
    volatile int v1 = rand();
    volatile float v2 = (float)rand() / RAND_MAX;
    volatile double v3 = (double)rand() / RAND_MAX;
    volatile int loop_limit = 100;  /* Controls loop iterations */
    
    /* MANY LIVE SCALAR VARIABLES (20+) to create register pressure */
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
    
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* Variables used in complex addressing */
    int idx1 = rand() % 50;
    int idx2 = rand() % 50;
    int idx3 = rand() % 50;
    
    /* Array for non-offsettable addressing */
    int array[100];
    double darray[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array[i] = rand() % 1000;
        darray[i] = (double)rand() / RAND_MAX;
    }
    
    /* Volatile accumulator to prevent dead code elimination */
    volatile double accumulator = 0.0;
    
    /* LOOP with invariant spilling */
    for (volatile int counter = 0; counter < loop_limit; counter++) {
        
        /* COMPLEX EXPRESSIONS mixing all variable types */
        /* Integer operations */
        int temp1 = a1 * a2 + a3 / (a4 + 1);
        int temp2 = (a5 & a6) | (a7 << 2) ^ a8;
        
        /* Floating point operations */
        float ftemp1 = f1 * f2 + f3 - f4 / f5;
        double dtemp1 = d1 * d2 + d3 - d4;
        
        /* Type conversions (require moves between register files) */
        float int_to_float1 = (float)a1 + (float)a2 * 0.5f;
        double int_to_double1 = (double)a3 + (double)a4 * 0.25;
        int float_to_int1 = (int)f1 + (int)(f2 * 100.0f);
        
        /* Mixed type operations */
        double mixed1 = (double)a1 * f1 + d1;
        float mixed2 = (float)a2 * (float)d1 + f2;
        
        /* COMPLEX ADDRESSING with non-offsettable addresses */
        /* These often require address reloads */
        int addr1 = array[idx1 + 17];  /* Non-simple offset */
        int addr2 = array[idx2 + 23];  /* Non-simple offset */
        double daddr1 = darray[idx3 + 19];  /* Non-simple offset */
        
        /* More complex addressing with arithmetic */
        int complex_addr = array[(a1 + a2) % 50 + 15];
        double dcomplex_addr = darray[(a3 + a4) % 50 + 12];
        
        /* FUNCTION CALL to clobber registers */
        int func_result = dummy_func(a1, a2, f1, d1, a3, f2);
        
        /* INLINE ASSEMBLY with register clobbers */
        /* This tells GCC many registers are unavailable */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* More calculations after assembly clobber */
        /* This forces reloads of previously computed values */
        a1 = temp1 + func_result;
        a2 = temp2 ^ addr1;
        f1 = ftemp1 * (float)addr2;
        d1 = dtemp1 + daddr1;
        
        /* Another function call with different arguments */
        double dresult = complex_op(a5, f3, d2, a6);
        
        /* More type mixing and conversions */
        double conv1 = (double)(a7 & 0xFF) * f4;
        float conv2 = (float)((a8 >> 4) + (int)d3);
        
        /* Update all variables to extend live ranges */
        a3 = a1 * 2 - a2;
        a4 = (a3 + a4) % 100;
        a5 = (a5 ^ a6) + addr1;
        a6 = complex_addr % 100;
        a7 = (a7 << 3) | (a8 >> 2);
        a8 = (a1 + a2 + a3 + a4) / 4;
        
        f2 = f1 * 0.9f + f3;
        f3 = f4 * 1.1f - f5;
        f4 = (float)a9 * 0.01f + f2;
        f5 = f3 * f4 - f1;
        
        d2 = d1 * 1.01 + d3;
        d3 = d4 * 0.99 - d2;
        d4 = (double)a10 * 0.001 + dcomplex_addr;
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += (double)a1 + (double)a2 + f1 + f2 + d1 + d2 
                     + (double)addr1 + daddr1 + (double)func_result + dresult;
        
        /* Modify indices for next iteration */
        idx1 = (idx1 + 1) % 50;
        idx2 = (idx2 + 2) % 50;
        idx3 = (idx3 + 3) % 50;
        
        /* Use volatile variable in loop to prevent optimization */
        if (v1 > 1000) {
            a9 = v1 % 100;
        }
    }
    
    /* Print result to prevent entire program elimination */
    printf("Final accumulator: %f\n", (double)accumulator);
    
    /* Use all variables one more time at the end */
    int final_sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
                  + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5
                  + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

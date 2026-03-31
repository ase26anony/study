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
    /* Volatile variables to prevent optimization */
    volatile int loop_limit = 100;
    volatile int accumulator = 0;
    
    /* MANY live scalar variables to create register pressure */
    /* Integer variables */
    register int v1 asm("eax") = rand();
    register int v2 asm("ebx") = rand();
    register int v3 asm("ecx") = rand();
    register int v4 asm("edx") = rand();
    int v5 = rand();
    int v6 = rand();
    int v7 = rand();
    int v8 = rand();
    int v9 = rand();
    int v10 = rand();
    int v11 = rand();
    int v12 = rand();
    int v13 = rand();
    int v14 = rand();
    int v15 = rand();
    
    /* Floating point variables */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* More variables for additional pressure */
    volatile int v16 = rand();
    volatile int v17 = rand();
    volatile float f5 = (float)rand() / RAND_MAX;
    volatile double d5 = (double)rand() / RAND_MAX;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    /* Pointer variables */
    int *ptr1 = array;
    int *ptr2 = array + 128;
    
    /* Loop with invariant spilling */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* Complex interdependent expressions mixing all variables */
        
        /* 1. Integer operations with register variables in conflicting contexts */
        v1 = (v1 * v2) + (v3 << 2) | (v4 & 0xFF);
        v2 = v5 + v6 - v7 * v8;
        v3 = (v9 ^ v10) | (v11 & v12);
        v4 = v13 * v14 - v15;
        
        /* 2. Type conversions requiring register file moves */
        f1 = (float)v1 + f2 * 2.5f;
        d1 = (double)v2 + d2 * 3.14159;
        v5 = (int)f3 + (int)d3;
        
        /* 3. Complex addressing modes with non-offsettable addresses */
        /* This creates addresses that may need reloading */
        int idx1 = v1 % 128;
        int idx2 = v2 % 128;
        
        /* Non-simple address: array[idx + constant] where idx is in register */
        int val1 = array[idx1 + 64];  /* May need address reload */
        int val2 = array[idx2 + 96];  /* May need address reload */
        
        /* 4. Mixed-size memory accesses */
        char *cptr = (char *)array;
        short *sptr = (short *)array;
        
        /* Different sized accesses in same expression */
        v6 = cptr[idx1] + sptr[idx2] + array[idx1];
        
        /* 5. Function call clobbering registers */
        v7 = dummy_func(v1, v2, f1, d1, v3, v4);
        
        /* 6. Inline assembly with multiple clobbers */
        /* This tells GCC these registers are unusable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* 7. More complex expressions after clobber */
        v8 = (val1 * val2) + (v6 << 3);
        f2 = f1 * 1.5f + (float)v8;
        d2 = d1 * 2.0 + (double)v7;
        
        /* 8. Pointer arithmetic creating complex addresses */
        ptr1 = array + (v1 % 64);
        ptr2 = array + (v2 % 64) + 32;  /* Non-simple offset */
        
        /* Use both pointers in computation */
        v9 = *ptr1 + *ptr2 + v8;
        
        /* 9. Bitwise and arithmetic combinations */
        v10 = (v9 & 0xFFFF) * (v8 | 0xFF) + (v7 << 2);
        
        /* 10. More type mixing */
        f3 = f2 + (float)(v10 % 256);
        d3 = d2 - (double)(v9 % 512);
        
        /* 11. Another function call */
        v11 = dummy_func(v5, v6, f3, d3, v7, v8);
        
        /* 12. Complex expression with all variable types */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11
                     + (int)f1 + (int)f2 + (int)f3 + (int)f4
                     + (int)d1 + (int)d2 + (int)d3 + (int)d4;
        
        /* Update volatile variables to prevent optimization */
        v16 = v1;
        v17 = v2;
        f5 = f1;
        d5 = d1;
        
        /* External call to clobber more registers */
        external_call();
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", accumulator);
    printf("Final values: %d %d %f %f\n", v16, v17, f5, d5);
    
    return 0;
}

/* External function definition to satisfy linker */
void external_call(void) {
    /* Do nothing but clobber registers */
    asm volatile ("" : : : "memory");
}

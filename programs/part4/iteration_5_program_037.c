#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e) {
    volatile int result = a + b + (int)c + (int)d + e;
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
    register int var1 asm("eax") = rand();
    register int var2 asm("ebx") = rand();
    register int var3 asm("ecx") = rand();
    int var4 = rand();
    int var5 = rand();
    int var6 = rand();
    int var7 = rand();
    int var8 = rand();
    int var9 = rand();
    int var10 = rand();
    int var11 = rand();
    int var12 = rand();
    int var13 = rand();
    int var14 = rand();
    int var15 = rand();
    
    /* Floating point variables */
    float fvar1 = (float)rand() / RAND_MAX;
    float fvar2 = (float)rand() / RAND_MAX;
    float fvar3 = (float)rand() / RAND_MAX;
    float fvar4 = (float)rand() / RAND_MAX;
    float fvar5 = (float)rand() / RAND_MAX;
    
    /* Double variables */
    double dvar1 = (double)rand() / RAND_MAX;
    double dvar2 = (double)rand() / RAND_MAX;
    double dvar3 = (double)rand() / RAND_MAX;
    double dvar4 = (double)rand() / RAND_MAX;
    
    /* Pointer variables for complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    int *ptr1 = &array[0];
    int *ptr2 = &array[128];
    volatile int *volatile_ptr = &array[64];
    
    /* Loop with invariant spilling */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* Complex interdependent expressions with mixed types */
        var1 = var2 * var3 + var4 - var5;
        var2 = (var6 & 0xFF) | (var7 << 8);
        var3 = var8 ^ var9 ^ var10;
        
        /* Type conversions forcing register moves */
        fvar1 = (float)var1 + fvar2 * 2.0f;
        dvar1 = (double)var2 + dvar2 * 3.14159;
        
        /* Complex pointer arithmetic with non-offsettable addresses */
        /* Large offset that may not fit in addressing mode */
        int idx = var3 % 128;
        int val1 = array[idx + 100];  /* idx + 100 may be too large for simple displacement */
        int val2 = array[idx * 2 + 50]; /* Multiplication in index */
        
        /* Mixed size memory accesses */
        char *byte_ptr = (char *)array;
        short *short_ptr = (short *)array;
        
        /* Different sized accesses in same expression */
        var4 = byte_ptr[idx] + short_ptr[idx] + array[idx];
        
        /* Function call clobbering registers */
        int call_result = dummy_func(var1, var2, fvar1, dvar1, var3);
        
        /* Inline assembly with many clobbered registers */
        /* Forces compiler to save/restore around asm */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory"
        );
        
        /* More complex calculations after asm clobber */
        var5 = (var11 * var12) / (var13 + 1);
        var6 = (var14 << 3) | (var15 >> 2);
        
        /* Floating point operations */
        fvar3 = fvar1 * fvar2 - fvar4 / fvar5;
        dvar3 = dvar1 + dvar2 * dvar4;
        
        /* Type punning through pointers */
        float *float_as_int = (float *)&var7;
        *float_as_int = fvar3;
        
        /* Complex expression with many operands */
        var8 = var1 + var2 - var3 * var4 / (var5 + 1) + (var6 & 0xFF) - (var7 >> 8);
        
        /* Another function call with different arguments */
        call_result += dummy_func(var4, var5, fvar3, dvar3, var8);
        
        /* More inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "memory"
        );
        
        /* Update volatile accumulator to prevent dead code elimination */
        accumulator += var1 + var2 + var3 + var4 + var5 + 
                      (int)fvar1 + (int)dvar1 + call_result;
        
        /* Modify variables to create data dependencies across iterations */
        var9 = var8 + iter;
        var10 = var9 * 2 - var7;
        var11 = (var10 & 0x5555) | (var9 & 0xAAAA);
        
        /* More type mixing */
        fvar4 = (float)var9 / 100.0f + fvar5;
        dvar4 = (double)var10 / 200.0 + dvar2;
        
        /* Complex addressing with pointer arithmetic */
        ptr1 += (var11 % 16) - 8;
        val1 = *ptr1 + *(ptr1 + 16);  /* Two memory accesses with pointer arithmetic */
        
        /* Access through volatile pointer */
        val2 = *volatile_ptr + iter;
        
        /* Final update to many variables */
        var12 = var11 ^ val1 ^ val2;
        var13 = (var12 << 1) + (var12 >> 31);  /* Rotate */
        var14 = var13 * 1103515245 + 12345;    /* PRNG step */
        var15 = var14 % 65536;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    /* Use all variables one more time to extend live ranges */
    int final_sum = var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10 +
                   var11 + var12 + var13 + var14 + var15 +
                   (int)fvar1 + (int)fvar2 + (int)fvar3 + (int)fvar4 + (int)fvar5 +
                   (int)dvar1 + (int)dvar2 + (int)dvar3 + (int)dvar4;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

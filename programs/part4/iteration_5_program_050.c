#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
float float_ops(float a, float b, float c, double d, double e) {
    volatile float res = a * b + c / (float)d - (float)e;
    return res;
}

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int loop_counter;
    volatile int accumulator = 0;
    volatile int loop_limit = 100;
    
    /* MANY INTEGER VARIABLES - create register pressure */
    register int r0 asm("r0") = rand();
    register int r1 asm("r1") = rand() % 100;
    register int r2 asm("r2") = rand() % 200;
    int v1 = rand();
    int v2 = rand();
    int v3 = rand();
    int v4 = rand();
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
    
    /* FLOATING POINT VARIABLES - different register class */
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    float f5 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    double d4 = (double)rand() / RAND_MAX;
    
    /* ARRAY for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = rand();
    }
    
    /* POINTERS for address calculations */
    int* ptr1 = &array[0];
    int* ptr2 = &array[128];
    volatile int* volatile_ptr = &array[64];
    
    /* LOOP with high register pressure */
    for (loop_counter = 0; loop_counter < loop_limit; loop_counter++) {
        /* COMPLEX EXPRESSIONS mixing all variables */
        /* Integer expressions with many intermediates */
        int expr1 = v1 + v2 * v3 - v4 / (v5 + 1);
        int expr2 = (v6 & v7) | (v8 << 2) ^ (v9 >> 1);
        int expr3 = expr1 * expr2 + v10 - v11;
        
        /* Floating point conversions and operations */
        float f_expr1 = f1 * f2 + (float)v1 / 100.0f;
        float f_expr2 = (float)(v2 & 0xFF) * f3 - f4;
        double d_expr1 = d1 * (double)f_expr1 + d2;
        double d_expr2 = (double)v3 / 256.0 + d3 * d4;
        
        /* TYPE CONVERSIONS forcing register moves */
        float float_from_int = (float)(v4 + v5);
        int int_from_float = (int)(f1 * 100.0f);
        double double_from_mixed = (double)(v6 * v7) + d1;
        
        /* COMPLEX ADDRESSING with non-offsettable addresses */
        /* These often require address reloads */
        int idx1 = (v8 + loop_counter) & 0xFF;
        int idx2 = (v9 * 3 + 7) & 0xFF;
        int idx3 = (v10 << 1) + 16;
        
        /* Non-simple addressing: array[index + constant] where 
           combined offset may be too large for direct addressing */
        int val1 = array[idx1 + 64];  /* May need address reload */
        int val2 = array[idx2 + 128]; /* Different large offset */
        int val3 = ptr1[idx3];        /* Pointer with index */
        
        /* More complex: *(ptr + (index << 2) + constant) */
        int* complex_addr = ptr2 + (idx1 << 2) + 16;
        int val4 = *complex_addr;
        
        /* FUNCTION CALL clobbers registers */
        int call_result = dummy_function(v1, v2, f1, d1, expr3, val1);
        
        /* INLINE ASSEMBLY with register clobbers */
        /* This forces compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            "nop\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "r10", "r11", "r12",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* SECOND FUNCTION CALL with different types */
        float float_result = float_ops(f1, f2, f3, d_expr1, d_expr2);
        
        /* MORE COMPLEX EXPRESSIONS after clobber */
        v1 = v2 + call_result;
        v2 = v3 ^ val2;
        v3 = v4 * (val3 + 1);
        v4 = v5 | (val4 & 0xFFFF);
        
        /* Floating point updates */
        f1 = f2 * float_result;
        f2 = f3 + (float)call_result / 1000.0f;
        f3 = f4 - f_expr1;
        f4 = f5 * 0.99f;
        
        /* Double precision updates */
        d1 = d2 + d_expr1;
        d2 = d3 * d_expr2;
        d3 = d4 - (double)float_result;
        
        /* Update register variables in ways that might conflict */
        /* Using integer register variables in floating context */
        r0 = r0 + (int)(f1 * 100.0f);
        r1 = r1 ^ (int)d1;
        
        /* VOLATILE accumulator update prevents dead code elimination */
        accumulator += expr3 + (int)f_expr1 + (int)d_expr1 + val1 + val2 + call_result;
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "# More clobbers\n"
            "nop\n"
            :
            :
            : "memory", "rax", "rbx", "rcx", "rdx", "rdi", "rsi",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
        );
        
        /* MIXED TYPE OPERATIONS in single expression */
        /* This can cause mode mismatches */
        double mixed = (double)v1 + f1 + d1 + (double)(v2 & v3);
        float mixed_float = (float)v4 + f2 + (float)d2;
        
        /* Update volatile pointer access */
        *volatile_ptr = *volatile_ptr + loop_counter;
        
        /* COMPLEX POINTER ARITHMETIC */
        /* Force address reloads with non-constant offsets */
        int dynamic_offset = (loop_counter * v1) & 0x3F;
        int* dynamic_ptr = &array[128 + dynamic_offset];
        int dynamic_val = *dynamic_ptr;
        
        /* Use in expression */
        v5 = v6 + dynamic_val * 2;
        v6 = v7 - (dynamic_val >> 1);
        
        /* Bitfield operations */
        v7 = (v8 << 3) | (v9 >> 5);
        v8 = (v10 & 0xF0F0F0F0) ^ (v11 & 0x0F0F0F0F);
        
        /* Another function call to break up live ranges */
        if (loop_counter % 3 == 0) {
            dummy_function(v12, v13, f3, d3, v14, v15);
        }
    }
    
    /* Final output to prevent optimization */
    printf("Result: %d\n", accumulator);
    
    /* Use all variables one more time to extend live ranges */
    int final_check = r0 + r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                     v9 + v10 + v11 + v12 + v13 + v14 + v15 +
                     (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                     (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    printf("Final check: %d\n", final_check % 1000);
    
    return 0;
}

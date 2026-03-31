#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_function(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double complex_op(int x, float y, double z, int w) {
    volatile double res = (double)x * y + z / (w + 1);
    return res;
}

int main(void) {
    /* Phase 1: Declare many variables to create register pressure */
    /* Integer variables - some with explicit register binding */
    register int r0_var asm("ax");
    register int r1_var asm("bx");
    register int r2_var asm("cx");
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Floating point variables */
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    
    /* Pointer variables for complex addressing */
    int array[256];
    volatile int* ptr1 = &array[0];
    volatile int* ptr2 = &array[128];
    
    /* Initialize with random values to create data dependencies */
    srand(42);
    r0_var = rand() % 100;
    r1_var = rand() % 100;
    r2_var = rand() % 100;
    
    for (int i = 0; i < 20; i++) {
        v0 = rand() % 100; v1 = rand() % 100; v2 = rand() % 100;
        v3 = rand() % 100; v4 = rand() % 100; v5 = rand() % 100;
        v6 = rand() % 100; v7 = rand() % 100; v8 = rand() % 100;
        v9 = rand() % 100; v10 = rand() % 100; v11 = rand() % 100;
        v12 = rand() % 100; v13 = rand() % 100; v14 = rand() % 100;
        v15 = rand() % 100; v16 = rand() % 100; v17 = rand() % 100;
        v18 = rand() % 100; v19 = rand() % 100;
        
        f0 = (float)rand() / RAND_MAX; f1 = (float)rand() / RAND_MAX;
        f2 = (float)rand() / RAND_MAX; f3 = (float)rand() / RAND_MAX;
        f4 = (float)rand() / RAND_MAX; f5 = (float)rand() / RAND_MAX;
        f6 = (float)rand() / RAND_MAX; f7 = (float)rand() / RAND_MAX;
        f8 = (float)rand() / RAND_MAX; f9 = (float)rand() / RAND_MAX;
        
        d0 = (double)rand() / RAND_MAX; d1 = (double)rand() / RAND_MAX;
        d2 = (double)rand() / RAND_MAX; d3 = (double)rand() / RAND_MAX;
        d4 = (double)rand() / RAND_MAX; d5 = (double)rand() / RAND_MAX;
        d6 = (double)rand() / RAND_MAX; d7 = (double)rand() / RAND_MAX;
        d8 = (double)rand() / RAND_MAX; d9 = (double)rand() / RAND_MAX;
    }
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Phase 2: Complex loop with high register pressure */
    volatile int loop_counter = 100;  /* Prevent loop unrolling */
    volatile int accumulator = 0;
    
    while (loop_counter-- > 0) {
        /* Complex expression 1: Mix integer and float operations */
        int temp1 = v0 + v1 * v2 - v3 / (v4 + 1);
        float temp2 = f0 * f1 + (float)temp1 / f2;
        double temp3 = d0 + d1 * (double)temp2 - d2 / (d3 + 1.0);
        
        /* Use explicit register variables in conflicting contexts */
        /* r0_var is bound to ax (integer reg), use in float context */
        float conflict1 = (float)r0_var * f3 + f4;
        /* Then use it in pointer arithmetic */
        int offset = r0_var * 4 + 16;  /* Non-simple offset */
        int mem_val = array[offset];   /* Complex addressing mode */
        
        /* Type conversions and mixed operations */
        double mixed1 = (double)v5 * d4 + (double)(v6 & 0xFF) * d5;
        float mixed2 = (float)(v7 | 0x0F) * f5 + (float)(v8 << 2) / f6;
        
        /* Function call to clobber registers */
        int func_result = dummy_function(v9, v10, f7, d6, v11, f8);
        
        /* Inline assembly with many clobbers */
        /* This forces the compiler to save/restore registers */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* More complex expressions after clobber */
        /* Use r1_var (bound to bx) in floating point operation */
        double conflict2 = (double)r1_var * d7 + d8;
        
        /* Complex pointer arithmetic with non-offsettable addresses */
        /* Create address that might need reloading */
        int idx = v12 + v13 * 2 - v14;
        int* complex_ptr = &array[idx + 64];  /* Large offset */
        int mem_val2 = *complex_ptr;
        
        /* Mixed size accesses */
        char char_val = (char)(v15 & 0xFF);
        short short_val = (short)(v16 * 2);
        int int_val = v17 + (int)char_val * short_val;
        
        /* Bitwise and arithmetic combinations */
        int complex_bits = (v18 << 3) | (v19 & 0x7);
        float float_from_bits = (float)complex_bits * f9;
        
        /* Another function call with different types */
        double func_result2 = complex_op(v0, f0, d9, v1);
        
        /* Update accumulator to prevent dead code elimination */
        accumulator += temp1 + (int)temp2 + (int)temp3 + (int)conflict1;
        accumulator += mem_val + (int)mixed1 + (int)mixed2 + func_result;
        accumulator += (int)conflict2 + mem_val2 + int_val;
        accumulator += (int)float_from_bits + (int)func_result2;
        
        /* Update some volatile variables to extend live ranges */
        v0 = accumulator % 100;
        f0 = (float)(accumulator % 100) / 10.0f;
        d0 = (double)(accumulator % 100) / 10.0;
        
        /* Use explicit register variable in another context */
        r2_var = accumulator & 0xFF;
    }
    
    /* Phase 3: Final complex expression using all variables */
    double final_result = 0.0;
    
    /* Large expression that uses most variables */
    final_result += (double)v0 * 1.1 + (double)v1 * 2.2;
    final_result += (double)v2 * 3.3 - (double)v3 * 4.4;
    final_result += (double)v4 / 5.5 + (double)v5 * 6.6;
    final_result += (double)v6 * 7.7 - (double)v7 / 8.8;
    final_result += (double)v8 * 9.9 + (double)v9 * 10.1;
    final_result += (double)v10 / 11.1 - (double)v11 * 12.2;
    final_result += (double)v12 * 13.3 + (double)v13 / 14.4;
    final_result += (double)v14 * 15.5 - (double)v15 / 16.6;
    final_result += (double)v16 * 17.7 + (double)v17 * 18.8;
    final_result += (double)v18 / 19.9 - (double)v19 * 20.1;
    
    final_result += (double)f0 * 21.1 + (double)f1 * 22.2;
    final_result += (double)f2 * 23.3 - (double)f3 / 24.4;
    final_result += (double)f4 * 25.5 + (double)f5 * 26.6;
    final_result += (double)f6 / 27.7 - (double)f7 * 28.8;
    final_result += (double)f8 * 29.9 + (double)f9 * 30.1;
    
    final_result += d0 * 31.1 + d1 * 32.2;
    final_result += d2 * 33.3 - d3 / 34.4;
    final_result += d4 * 35.5 + d5 * 36.6;
    final_result += d6 / 37.7 - d7 * 38.8;
    final_result += d8 * 39.9 + d9 * 40.1;
    
    /* Use explicit register variables one last time */
    final_result += (double)r0_var * 41.1;
    final_result += (double)r1_var * 42.2;
    final_result += (double)r2_var * 43.3;
    
    printf("Final accumulator: %d\n", accumulator);
    printf("Final result: %f\n", final_result);
    
    return 0;
}

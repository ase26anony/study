/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* External function to create opaque values */
extern int rand(void);

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Use seed to initialize computation */
    int base = seed * 37 + 12345;
    
    /* Complex arithmetic expression creating many temporaries */
    for (i = 0; i < n; i++) {
        /* Volatile reads force loads */
        int v1 = vol_a;
        int v2 = vol_b;
        int v3 = vol_c;
        int v4 = vol_d;
        
        /* Long dependency chain with mixed operations */
        int temp1 = v1 * v2 + v3 / (v4 + 1);
        int temp2 = v2 % (v3 + 1) - v4 * v1;
        int temp3 = (v1 + v2) * (v3 - v4) % 256;
        int temp4 = (v2 << 2) | (v3 >> 1) & v4;
        
        /* Multi-use temporary value - candidate for rematerialization */
        int multi_use = temp1 * temp2 - temp3 + temp4;
        
        /* Use multi_use in different contexts */
        if (i % 3 == 0) {
            result += multi_use * 2;
        } else if (i % 3 == 1) {
            result += multi_use / 2;
        } else {
            result += multi_use % 256;
        }
        
        /* Inline assembly to clobber registers */
        /* This reduces available physical registers */
#ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
#endif
        
        /* Floating point operations for different register types */
        float f1 = vol_f1;
        float f2 = vol_f2;
        float f3 = vol_f3;
        
        float ftemp1 = f1 * f2 + f3 / (f1 + 0.5f);
        float ftemp2 = f2 - f3 * f1 / (f2 + 1.0f);
        float ftemp3 = (f1 + f2) * (f3 - f1) / 2.0f;
        
        /* Use results to affect integer computation */
        result += (int)(ftemp1 + ftemp2 + ftemp3);
        
        /* Address computation with multiple offsets */
        /* This can trigger base register rematerialization */
        int array[16];
        for (j = 0; j < 8; j++) {
            /* Compute base address */
            int *base_ptr = &array[j];
            
            /* Use with different offsets - may trigger register recreation */
            int val1 = *(base_ptr + 0);
            int val2 = *(base_ptr + 1);
            int val3 = *(base_ptr + 2);
            int val4 = *(base_ptr + 3);
            
            result += val1 + val2 * 2 - val3 + val4 / 2;
        }
        
        /* Opaque function call prevents value analysis */
        int rand_val = rand() % 100;
        
        /* Another complex expression with function result */
        int complex_expr = (multi_use * rand_val) / (temp2 + 1) 
                         + (temp3 % (rand_val + 1)) 
                         - (temp4 & (rand_val * 3));
        
        result += complex_expr % 65536;
        
        /* Loop-carried dependency with volatile */
        vol_a = (vol_a + 1) % 100;
    }
    
    return result;
}

/* Second stress function with different pattern */
int stress_memory_access(int seed, int n) {
    int result = 0;
    int data[256];
    int i;
    
    /* Initialize array with pseudo-random values */
    for (i = 0; i < 256; i++) {
        data[i] = (seed + i * 37) % 7919;
    }
    
    /* Complex memory access pattern */
    for (i = 0; i < n; i++) {
        /* Multiple base computations */
        int idx1 = (i * 17) % 256;
        int idx2 = (i * 23) % 256;
        int idx3 = (i * 31) % 256;
        
        /* Compute multiple address expressions */
        int *ptr1 = &data[idx1];
        int *ptr2 = &data[idx2];
        int *ptr3 = &data[idx3];
        
        /* Complex expression using multiple memory values */
        int val1 = *ptr1;
        int val2 = *ptr2;
        int val3 = *ptr3;
        
        /* Expression with many temporaries */
        int expr1 = val1 * val2 + val3;
        int expr2 = val1 % (val2 + 1) - val3;
        int expr3 = (val1 << 3) | (val2 >> 2) & val3;
        
        /* Multi-use value */
        int combined = expr1 + expr2 * 2 - expr3;
        
        /* Use in different basic blocks */
        switch (i % 4) {
            case 0:
                result += combined * 3;
                break;
            case 1:
                result += combined / 3;
                break;
            case 2:
                result += combined % 511;
                break;
            case 3:
                result += (combined << 1) | 1;
                break;
        }
        
        /* More register pressure */
        float fcalc = (float)val1 * 1.5f + (float)val2 * 2.5f - (float)val3 * 0.5f;
        result += (int)fcalc;
    }
    
    return result;
}

/* Main test harness */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    int total_result = 0;
    int i;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    srand(seed);
    
    /* Call stress functions multiple times */
    for (i = 0; i < 5; i++) {
        int result1 = stress_computation(seed + i, iterations);
        int result2 = stress_memory_access(seed + i * 7, iterations / 2);
        
        total_result += result1 + result2;
        
        /* Modify volatile variables between calls */
        vol_a = (vol_a * 3 + 7) % 256;
        vol_b = (vol_b * 5 + 11) % 256;
        vol_f1 = vol_f1 * 1.1f + 0.5f;
    }
    
    /* Prevent dead code elimination */
    printf("Final result: %d\n", total_result % 1000000);
    
    return total_result != 0 ? 0 : 1;
}

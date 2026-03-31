/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Complex arithmetic with volatile variables - creates many temporaries */
    int base1 = vol_a * vol_b + vol_c / vol_d - seed % 7;
    int base2 = vol_b * vol_c - vol_d / vol_a + seed % 11;
    
    /* Floating point chain - creates FP virtual registers */
    float fp_base = vol_f1 * vol_f2 + vol_f3 / vol_f1 - (float)seed / 13.0f;
    
    /* Multi-use temporary value used in different contexts */
    int multi_use_temp = base1 * base2 + (int)(fp_base * 100.0f);
    
    /* Inline assembly that clobbers registers - increases pressure */
    asm volatile (
        "# Clobber multiple registers\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        :
        : "r" (base1), "r" (base2)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* Loop with volatile bounds - prevents optimization */
    for (i = 0; i < vol_a + n; i++) {
        int loop_temp = 0;
        
        /* Complex expression inside loop */
        loop_temp = multi_use_temp * i 
                  + (vol_b * vol_c) / (vol_d + 1)
                  - (seed % (i + 1))
                  + (int)(vol_f1 * vol_f2 * i);
        
        /* Switch with different uses of temporaries */
        switch (i % 4) {
            case 0:
                result += loop_temp + base1;
                break;
            case 1:
                result += loop_temp * base2 - multi_use_temp;
                break;
            case 2:
                result += (loop_temp / (base1 + 1)) * multi_use_temp;
                break;
            case 3:
                result += loop_temp % (base2 + 1) + multi_use_temp / 2;
                break;
        }
        
        /* Nested loop with opaque function call */
        for (j = 0; j < vol_b; j++) {
            /* Opaque value forces register holding */
            int opaque_val = get_external_value();
            
            /* Address computation with multiple offsets */
            int* addr_base = &result;
            int offset1 = *(addr_base + 0) + opaque_val;
            int offset2 = *(addr_base - 0) * opaque_val;  /* Same base, different use */
            int offset3 = *(int*)((char*)addr_base + 4) + opaque_val * 2;
            
            /* More complex arithmetic with the offsets */
            result += offset1 * offset2 - offset3 / (opaque_val + 1);
            
            /* Another inline asm with clobbers */
            asm volatile (
                "# More register clobbering\n"
                "add r4, %0, %1\n"
                :
                : "r" (offset1), "r" (offset2)
                : "r4", "r5", "r6", "cc"
            );
        }
        
        /* Update volatile to prevent loop unrolling */
        vol_a = (vol_a + 1) % 5;
    }
    
    /* Final complex expression using all temporaries */
    result = result 
           + multi_use_temp * 3 
           - base1 * 2 
           + base2 
           + (int)(fp_base * 1000.0f);
    
    return result;
}

/* Second stress function with different patterns */
int stress_memory_access(int* array, int size) {
    int sum = 0;
    int i;
    
    /* Compute base address once, use with multiple offsets */
    int* mid_ptr = array + size / 2;
    
    for (i = 0; i < size; i++) {
        /* Multiple uses of the same base pointer with different offsets */
        int val1 = *(mid_ptr + i - size/2);
        int val2 = *(mid_ptr + i - size/2 + 1);
        int val3 = *(mid_ptr + i - size/2 - 1);
        
        /* Complex arithmetic chain */
        int temp = val1 * val2 + val3 / (val1 + 1) - i % (val2 + 1);
        
        /* Use opaque function result */
        int rand_val = rand() % 256;
        temp = temp * rand_val - (val1 ^ val2) + (val3 & rand_val);
        
        sum += temp;
        
        /* Volatile access in the middle */
        vol_c = sum % 1000;
    }
    
    return sum;
}

/* Main test harness */
int main(int argc, char** argv) {
    int i, total_result = 0;
    int test_array[100];
    
    /* Initialize with some data */
    srand(time(NULL));
    for (i = 0; i < 100; i++) {
        test_array[i] = rand() % 1000;
    }
    
    /* Call stress functions multiple times */
    for (i = 0; i < 10; i++) {
        int seed = rand() % 100;
        
        /* First stress function */
        int result1 = stress_computation(seed, 20 + i);
        
        /* Second stress function */
        int result2 = stress_memory_access(test_array, 50 + i % 50);
        
        /* Combine results */
        total_result += result1 - result2;
        
        /* Update volatile to affect next iteration */
        vol_d = total_result % 100;
    }
    
    /* Prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}

/* Dummy implementation of external function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 13 + 7) % 97;
}

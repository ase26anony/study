#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper functions to prevent inlining */
__attribute__((noinline)) void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    asm volatile ("" ::: "memory");
}

__attribute__((noinline)) void complex_barrier(int *arr, int idx) {
    volatile int temp = arr[idx];
    arr[idx] = temp * 3 - (temp % 7);
    asm volatile ("" ::: "memory");
}

/* Main computation with scheduling stress */
int main() {
    srand(time(NULL));
    
    /* Large arrays with different data types */
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    volatile int checksum = 0;
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)(rand() % 1000) / 3.14159;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double fp_acc = fp_array[0];
        int int_acc = int_array[0];
        
        /* Long chain of dependent FP operations */
        fp_acc = fp_acc + fp_array[1] * 2.5;
        fp_acc = sqrt(fabs(fp_acc)) + sin(fp_acc * 0.01);
        fp_acc = fp_acc / (fp_array[2] + 1.0);
        fp_acc = fp_acc * fp_array[3] - fp_array[4];
        fp_acc = fp_acc + cos(fp_acc * 0.1) * 3.14159;
        
        /* Interleaved integer operations */
        int_acc = int_acc + int_array[1] * 3;
        int_acc = int_acc - (int_array[2] % 17);
        int_acc = int_acc * (int_array[3] / 5 + 1);
        int_acc = int_acc + (int_array[4] << 2);
        int_acc = int_acc ^ (int_array[5] | 0xFF);
        
        /* Memory operations with varying addressing */
        for (int i = 5; i < 50; i++) {
            int_array[i] = int_array[i-1] + int_array[i-2] * int_array[i-3];
            fp_array[i] = fp_array[i-1] * 1.1 + fp_array[i-2] / 1.3;
        }
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = (rand() % 50) + 10;  /* Data-dependent */
        for (int j = 0; j < inner_bound; j++) {
            /* Mixed operations within inner loop */
            double temp_fp = fp_array[j] * 2.0;
            temp_fp = temp_fp + sin(temp_fp * 0.05);
            
            int temp_int = int_array[j] + j * 7;
            temp_int = temp_int - (temp_int % 13);
            
            /* Memory store with complex addressing */
            int idx = (j * 17 + outer) % ARRAY_SIZE;
            int_array[idx] = temp_int + (int)(temp_fp * 100);
            fp_array[idx] = temp_fp * 0.99;
            
            /* Function call as scheduling barrier */
            complex_barrier(int_array, idx);
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42);  /* Rare condition */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_fp = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_fp += sqrt(fp_array[k % ARRAY_SIZE] + 1.0);
                cold_fp = cold_fp * 1.01 - 0.5;
                asm volatile ("" ::: "memory");
            }
            checksum += (int)cold_fp;
        } else {
            /* Hot path - simpler operations */
            checksum += int_array[outer % ARRAY_SIZE];
        }
        
        /* Pattern 4: VLA usage between patterns */
        int vla_size = (rand() % 100) + 10;
        use_vla(vla_size);
        
        /* More complex dependency chains */
        double chain_result = fp_array[0];
        for (int m = 1; m < 20; m++) {
            chain_result = chain_result * fp_array[m] + fp_array[m+1];
            chain_result = chain_result / (m + 1.0);
            chain_result = sqrt(fabs(chain_result));
            
            /* Insert barriers at strategic points */
            if (m % 5 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Final mixed operations */
        int final_int = int_array[ARRAY_SIZE-1];
        final_int = final_int * 3 + (final_int % 256);
        final_int = final_int ^ (final_int >> 4);
        final_int = final_int - (final_int / 11);
        
        checksum += final_int + (int)chain_result;
        
        /* Additional scheduling region with alloca */
        if (outer % 10 == 0) {
            int* dynamic = (int*)alloca(sizeof(int) * 50);
            for (int n = 0; n < 50; n++) {
                dynamic[n] = int_array[n] * n;
                dynamic[n] = dynamic[n] + (dynamic[n] % 19);
            }
            checksum += dynamic[25];
        }
    }
    
    /* Final result computation to prevent elimination */
    double final_fp = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_fp += fp_array[i] * 0.01;
        final_fp = final_fp - floor(final_fp);
    }
    
    checksum += (int)(final_fp * 1000000);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

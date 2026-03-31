/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_scheduling_1(int* arr_a, int* arr_b, int* arr_c, int size, volatile int threshold) {
    int sum = 0;
    int temp1, temp2, temp3;
    
    /* Nested loops with data-dependent branches */
    for (int j = 0; j < 4; j++) {  /* Outer loop */
        int base = (j * 17) & 0xFF;  /* Outer loop carried state */
        
        for (int i = 0; i < size; i++) {
            /* Mixed arithmetic with flow dependencies */
            temp1 = arr_a[i] * base;
            temp2 = arr_b[i] + temp1;
            
            /* Data-dependent conditional branch */
            if (temp2 & 1) {  /* Unpredictable branch */
                temp3 = temp2 * 3;
                arr_c[i] = temp3 - base;
                
                /* Inline assembly barrier to prevent optimization */
                asm volatile("" ::: "memory");
            } else {
                temp3 = temp2 / 2;
                arr_c[i] = temp3 + base;
            }
            
            /* Anti-dependency: reusing temp1 */
            temp1 = arr_c[i] ^ 0x55;
            
            /* Output dependency: multiple writes to sum */
            sum = sum + temp1;
            
            /* Complex condition with threshold */
            if (sum > threshold) {
                arr_a[i] = sum;
                sum = sum / 2;  /* Reset partially */
                
                /* Another barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int idx = i + 1;
                temp1 = arr_a[idx] * (base + 1);
                temp2 = arr_b[idx] - temp1;
                
                if (temp2 & 2) {
                    arr_c[idx] = temp2 * 5;
                } else {
                    arr_c[idx] = temp2 / 3;
                }
                
                sum = sum + arr_c[idx];
                i++;  /* Skip next iteration */
            }
        }
    }
}

/* Function with volatile counters and mixed data types */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_scheduling_2(float* farr, double* darr, int size) {
    volatile int v_counter = size;  /* Volatile to prevent optimization */
    double acc_d = 0.0;
    float acc_f = 0.0f;
    
    /* Mixed float/double operations */
    for (volatile int i = 0; i < v_counter; i++) {
        /* Type conversions create complex RTL */
        double d_val = (double)farr[i];
        float f_val = (float)darr[i % 16];  /* Modulo for variability */
        
        /* Floating-point operations with dependencies */
        acc_d = acc_d + d_val * 1.5;
        acc_f = acc_f + f_val * 2.0f;
        
        /* Conditional with floating comparison */
        if (acc_d > 1000.0 || acc_f > 500.0f) {
            darr[i % 16] = acc_d;
            acc_d = acc_d * 0.5;
            acc_f = acc_f * 0.5f;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Nested conditional with integer cast */
        int int_val = (int)acc_d;
        if (int_val & (1 << (i % 8))) {
            farr[i] = (float)int_val / 255.0f;
        }
        
        /* Unrolled section */
        if (i + 3 < size) {
            for (int unroll = 1; unroll <= 3; unroll++) {
                int idx = i + unroll;
                double tmp = darr[idx % 16] * unroll;
                farr[idx] = (float)(tmp + acc_d);
                acc_d += tmp * 0.1;
            }
            i += 3;
        }
    }
}

/* Outer loop carried state pattern */
void test_outer_loop_carried(int* data, int rows, int cols) {
    int state = 0;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop modifies state */
        state = (state * 13 + r) & 0xFFF;
        int factor = (state >> 4) + 1;
        
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Inner loop uses outer loop state */
            int val = data[idx];
            val = (val * factor) + (state & 0xF);
            
            /* Data-dependent operation */
            if (val > 1000) {
                val = val / factor;
                asm volatile("" ::: "memory");
            } else if (val < 100) {
                val = val * factor;
            }
            
            /* Multiple dependency chains */
            int tmp1 = val + c;
            int tmp2 = tmp1 - state;
            data[idx] = tmp2 ^ (factor * 7);
            
            /* Anti-dependency */
            tmp1 = data[idx] * 3;
            state = (state + tmp1) & 0xFFF;
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with pseudo-random data */
    int array_a[SIZE];
    int array_b[SIZE];
    int array_c[SIZE];
    float farray[SIZE];
    double darray[16];
    int matrix[ROWS * COLS];
    
    printf("Initializing arrays...\n");
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = 0;
        farray[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 16; i++) {
        darray[i] = (double)(lcg_rand() % 1000) / 5.0;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = lcg_rand() % 500;
    }
    
    /* Volatile flag for runtime variability */
    volatile int flag = 0;
    int checksum = 0;
    
    /* Call test functions multiple times with volatile conditions */
    for (int iteration = 0; iteration < 5; iteration++) {
        flag = lcg_rand() & 1;
        
        if (flag) {
            /* Force selective scheduler activation */
            test_selective_scheduling_1(array_a, array_b, array_c, SIZE, 5000);
            
            /* Multiple calls to increase scheduling complexity */
            for (int rep = 0; rep < 3; rep++) {
                test_selective_scheduling_2(farray, darray, SIZE);
            }
        } else {
            test_outer_loop_carried(matrix, ROWS, COLS);
            
            /* Mix with first test */
            test_selective_scheduling_1(array_b, array_c, array_a, SIZE / 2, 2500);
        }
        
        /* Update checksum to prevent dead code elimination */
        checksum += array_a[iteration % SIZE];
        checksum += array_c[iteration % SIZE];
        checksum += (int)farray[iteration % SIZE];
    }
    
    /* Final computation using all arrays */
    int final_checksum = 0;
    for (int i = 0; i < SIZE; i += 64) {
        final_checksum += array_a[i] + array_b[i] + array_c[i];
        final_checksum += (int)farray[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i += 128) {
        final_checksum += matrix[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Test completed - selective scheduler should have been triggered.\n");
    
    return 0;
}

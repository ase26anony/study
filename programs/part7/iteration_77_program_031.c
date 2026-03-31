/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in GCC's targhooks.cc, specifically lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_start = 0;
volatile int g_end = 100;
int g_array[1000];
long g_matrix[10][10];
volatile int* g_ptr = NULL;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end) {
    __transaction_atomic {
        /* Simple array processing loop - candidate for load/store transformation */
        for (int i = start; i < end; i++) {
            g_array[i] = g_array[i] * 2 + 1;
        }
        
        /* Additional store to volatile pointer */
        if (g_ptr) {
            *g_ptr = end;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int rows, int cols) {
    /* Nested loops with multi-dimensional array access */
    __transaction_relaxed {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                g_matrix[i][j] = g_matrix[i][j] + (i * cols + j);
            }
        }
    }
    
    /* Transaction with retry logic */
    int attempts = 3;
    while (attempts-- > 0) {
        __transaction_atomic {
            for (int i = 0; i < rows; i++) {
                g_matrix[i][i] *= 2;
            }
            
            /* Conditional transaction cancel */
            if (attempts == 1) {
                __transaction_cancel;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int limit) {
    volatile int local_counter = limit;
    
    /* Complex control flow with TM in branches */
    if (local_counter > 50) {
        __transaction_atomic {
            /* While loop with volatile condition */
            while (local_counter > 0) {
                int idx = limit - local_counter;
                if (idx < 1000) {
                    g_array[idx] += local_counter;
                }
                local_counter--;
            }
        }
    } else {
        __transaction_relaxed {
            /* Different loop structure */
            for (int i = 0; i < limit; i += 2) {
                g_array[i] = g_array[i + 1] + i;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(void) {
    /* Pointer-based loop within transaction */
    int* ptr = g_array;
    int counter = g_end;
    
    __transaction_atomic {
        while (counter-- > g_start) {
            *ptr = *ptr ^ 0xAAAA;  /* Non-linear transformation */
            ptr++;
        }
    }
    
    /* Nested transaction region */
    __transaction_relaxed {
        __transaction_atomic {
            for (int i = 0; i < 10; i++) {
                g_matrix[0][i] = g_array[i * 10];
            }
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1000; i++) {
        g_array[i] = i % 256;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            g_matrix[i][j] = i * 10 + j;
        }
    }
    
    g_ptr = &g_array[500];
}

/* Compute checksum to verify execution and prevent elimination */
long compute_checksum(void) {
    long sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += g_array[i];
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += g_matrix[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Execute various TM loop patterns */
    tm_loop_transform1(g_start, g_end);
    tm_loop_transform2(10, 10);
    tm_loop_transform3(75);
    tm_loop_transform4();
    
    /* Additional calls with different parameters */
    tm_loop_transform1(50, 150);
    tm_loop_transform2(5, 5);
    tm_loop_transform3(25);
    
    /* Compute and print checksum to ensure execution */
    long checksum = compute_checksum();
    printf("TM Loop Transformation Test\n");
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}

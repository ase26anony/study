/* tm_loop_coverage.c
 * Designed to trigger default_builtin_tm_load_store_loop_transform
 * in targhooks.cc for coverage of lines 981-990
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
    /* Transaction with simple array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load and store operations */
            int val = g_array[i];          /* Load */
            g_array[i] = val * 2 + i;      /* Store */
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int n) {
    /* Nested loops with relaxed transaction */
    __transaction_relaxed {
        int i = 0;
        while (i < n) {
            int j = 0;
            while (j < n) {
                /* Load and store on 2D matrix */
                long current = g_matrix[i][j];  /* Load */
                g_matrix[i][j] = current + i * j + 1;  /* Store */
                j++;
            }
            i++;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int* data, int size) {
    /* Complex control flow with transaction */
    if (size > 0) {
        __transaction_atomic {
            /* Loop with pointer arithmetic */
            for (int i = 0; i < size; i++) {
                volatile int* elem = &data[i];
                int old = *elem;           /* Load */
                *elem = old ^ 0x55AA55AA;  /* Store */
                
                /* Nested transaction attempt */
                if (old < 0) {
                    __transaction_cancel;
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform4(void) {
    /* Multiple transactional regions */
    __transaction_relaxed {
        for (int i = g_start; i < g_end; i += 2) {
            g_array[i] = g_array[i] * 3;
        }
    }
    
    __transaction_atomic {
        int i = g_start;
        do {
            g_array[i] += g_array[i + 1];
            i++;
        } while (i < g_end - 1);
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform5(int rows, int cols) {
    /* Complex nested loops with conditional transactions */
    for (int r = 0; r < rows; r++) {
        /* Transaction inside outer loop */
        __transaction_atomic {
            for (int c = 0; c < cols; c++) {
                /* Multiple loads and stores */
                long a = g_matrix[r][c];
                long b = (c > 0) ? g_matrix[r][c-1] : 0;
                long sum = a + b;
                g_matrix[r][c] = sum;
                
                /* Volatile access to force memory ops */
                volatile long* vptr = &g_matrix[r][c];
                *vptr = *vptr ^ 0x12345678;
            }
        }
    }
}

/* Initialize test data */
void init_data(void) {
    for (int i = 0; i < 1000; i++) {
        g_array[i] = i % 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            g_matrix[i][j] = i * 10 + j;
        }
    }
    
    g_ptr = (int*)malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++) {
        g_ptr[i] = i * 2;
    }
}

/* Calculate checksum to prevent optimization */
long calculate_checksum(void) {
    long sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += g_array[i];
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += g_matrix[i][j];
        }
    }
    if (g_ptr) {
        for (int i = 0; i < 100; i++) {
            sum += g_ptr[i];
        }
    }
    return sum;
}

int main(void) {
    /* Initialize shared data */
    init_data();
    
    /* Call test functions with non-constant bounds */
    int start = g_start + 10;
    int end = g_end - 10;
    
    tm_loop_transform1(start, end);
    tm_loop_transform2(8);  /* Non-power-of-two to prevent optimization */
    tm_loop_transform3(g_ptr, 50);
    tm_loop_transform4();
    tm_loop_transform5(5, 5);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 3; i++) {
        tm_loop_transform1(i * 20, i * 20 + 30);
        __transaction_atomic {
            g_start += 1;
            g_end -= 1;
        }
    }
    
    /* Calculate and print checksum */
    long checksum = calculate_checksum();
    printf("TM Loop Transformation Test\n");
    printf("Final checksum: %ld\n", checksum);
    
    /* Cleanup */
    if (g_ptr) free(g_ptr);
    
    return 0;
}

/* tm_loop_test.c - Test program for GCC transactional memory loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_counter = 0;
int g_data_array[1024];
long g_large_buffer[2048];
volatile int *g_volatile_ptr = NULL;

/* Prevent optimization and inlining */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end, int *array) {
    /* Transaction with array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern that should trigger transformation */
            int temp = array[i];
            temp = temp * 2 + g_shared_counter;
            array[i] = temp;
            g_shared_counter++;
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int rows, int cols, int *matrix) {
    /* Nested loops with transactional memory */
    __transaction_relaxed {
        for (int i = 0; i < rows; i++) {
            int row_start = i * cols;
            int row_end = row_start + cols;
            
            /* Inner loop with pointer arithmetic */
            int *row_ptr = &matrix[row_start];
            for (int j = 0; j < cols; j++) {
                /* Complex load-store pattern */
                int val = *row_ptr;
                val = (val << 3) | (val >> 5);  /* Rotate bits */
                *row_ptr = val ^ g_shared_counter;
                row_ptr++;
                
                /* Conditional transaction cancel */
                if (val == 0xFFFFFFFF) {
                    __transaction_cancel;
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(int limit, long *buffer) {
    /* Mixed transaction types with while loop */
    volatile int local_volatile = limit;
    
    if (local_volatile > 0) {
        __transaction_atomic {
            int idx = 0;
            while (idx < local_volatile) {
                /* Multiple memory operations */
                buffer[idx] = buffer[idx] * 3;
                buffer[idx] += idx;
                
                /* Access through volatile pointer */
                if (g_volatile_ptr) {
                    buffer[idx] ^= *g_volatile_ptr;
                }
                
                idx += (local_volatile % 7) + 1;  /* Non-uniform stride */
            }
        }
    }
    
    /* Another transaction in same function */
    __transaction_relaxed {
        for (int i = 0; i < limit && i < 100; i++) {
            g_data_array[i % 1024] = buffer[i % 2048];
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_nested_transactions(int depth, int iterations) {
    /* Complex control flow with nested TM regions */
    for (int d = 0; d < depth; d++) {
        __transaction_atomic {
            int base = d * 100;
            
            for (int i = 0; i < iterations; i++) {
                int idx = (base + i) % 1024;
                
                /* Load-modify-store with multiple arrays */
                int old_val = g_data_array[idx];
                g_data_array[idx] = old_val + g_large_buffer[i % 2048];
                g_large_buffer[i % 2048] = old_val;
                
                /* Inner transaction for specific cases */
                if ((i % 17) == 0) {
                    __transaction_relaxed {
                        g_shared_counter += g_data_array[idx];
                    }
                }
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_pointer_chasing(int steps, int **pointer_array) {
    /* Loop with pointer chasing pattern */
    __transaction_atomic {
        int *current = &g_shared_counter;
        
        for (int i = 0; i < steps; i++) {
            /* Load through pointer, modify, store */
            int value = *current;
            value = (value * 1103515245 + 12345) & 0x7fffffff;
            *current = value;
            
            /* Chase to next pointer */
            if (i < steps - 1 && pointer_array[i]) {
                current = pointer_array[i];
            }
        }
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Initialize shared data */
    for (int i = 0; i < 1024; i++) {
        g_data_array[i] = i * 3;
    }
    
    for (int i = 0; i < 2048; i++) {
        g_large_buffer[i] = i * 5;
    }
    
    volatile int local_vol = 42;
    g_volatile_ptr = &local_vol;
    
    /* Create pointer array for chasing */
    int *ptr_array[50];
    for (int i = 0; i < 50; i++) {
        ptr_array[i] = &g_data_array[i * 20 % 1024];
    }
    
    /* Execute TM functions with varying parameters */
    printf("Starting TM loop transformation tests...\n");
    
    /* Test 1: Simple array transformation */
    tm_loop_transform1(10, 500, g_data_array);
    
    /* Test 2: Matrix transformation */
    int matrix[100];
    for (int i = 0; i < 100; i++) matrix[i] = i;
    tm_loop_transform2(10, 10, matrix);
    
    /* Test 3: Buffer processing with volatile */
    tm_loop_transform3(300, g_large_buffer);
    
    /* Test 4: Nested transactions */
    tm_nested_transactions(5, 200);
    
    /* Test 5: Pointer chasing */
    tm_pointer_chasing(40, ptr_array);
    
    /* Compute checksum to verify execution */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += g_data_array[i];
    }
    for (int i = 0; i < 2048; i++) {
        checksum += g_large_buffer[i];
    }
    checksum += g_shared_counter;
    
    printf("Checksum: %ld\n", checksum);
    printf("Shared counter: %d\n", g_shared_counter);
    
    return 0;
}

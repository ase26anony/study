/* tm_loop_test.c - Test program for GCC TM load/store loop transformation */
#include <stdio.h>
#include <stdlib.h>

/* Shared global variables for TM operations */
volatile int g_shared_array[1024];
volatile long g_shared_long[512];
int* g_shared_ptr = NULL;
volatile int g_loop_bound = 100;

/* Prevent optimization of critical functions */
__attribute__((noinline, noipa, used))
void tm_loop_transform1(int start, int end) {
    /* Transaction with simple array processing loop */
    __transaction_atomic {
        for (int i = start; i < end; i++) {
            /* Load-store pattern that should trigger transformation */
            int val = g_shared_array[i];          /* Load */
            val = val * 2 + i;                    /* Transform */
            g_shared_array[i] = val;              /* Store */
            
            /* Additional store to different array */
            if (i < 512) {
                g_shared_long[i] = val * 3L;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform2(int* data, int size) {
    volatile int local_bound = size;
    int attempts = 0;
    
    /* Transaction with retry logic and pointer-based loop */
    while (attempts < 3) {
        __transaction_atomic {
            int bound = local_bound;
            int* ptr = data;
            
            /* While loop with pointer arithmetic */
            while (ptr < data + bound) {
                int old_val = *ptr;               /* Load */
                *ptr = old_val ^ 0x55AA55AA;      /* Store with transform */
                ptr++;
            }
            
            /* Nested loop for 2D access pattern */
            for (int i = 0; i < bound/4; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < bound) {
                        g_shared_array[idx] += data[i];
                    }
                }
            }
            
            /* Conditional transaction cancel */
            if (attempts > 0 && (data[0] & 1)) {
                __transaction_cancel;
            }
        }
        attempts++;
    }
}

__attribute__((noinline, noipa, used))
void tm_loop_transform3(volatile int* arr, int n) {
    /* Mixed relaxed and atomic transactions */
    if (n > 10) {
        __transaction_relaxed {
            /* Loop with volatile accesses */
            for (int i = 0; i < n; i += 2) {
                int a = arr[i];                   /* Volatile load */
                int b = arr[i + 1];               /* Volatile load */
                arr[i] = b;                       /* Volatile store */
                arr[i + 1] = a;                   /* Volatile store */
            }
        }
    }
    
    /* Follow with atomic transaction */
    __transaction_atomic {
        /* Complex loop with conditionals */
        int i = 0;
        while (i < n) {
            if (arr[i] > 0) {
                for (int j = 0; j < 3 && (i + j) < n; j++) {
                    arr[i + j] -= 1;              /* Store */
                }
                i += 3;
            } else {
                arr[i] *= -1;                     /* Load-store */
                i++;
            }
        }
    }
}

__attribute__((noinline, noipa, used))
void tm_nested_transactions(int depth, int size) {
    /* Function with potentially nested TM regions */
    if (depth > 0) {
        __transaction_atomic {
            /* Loop that might trigger transformation */
            for (int i = 0; i < size; i++) {
                g_shared_array[i] += depth;
            }
            
            /* Recursive call creates nested TM possibility */
            if (depth > 1) {
                tm_nested_transactions(depth - 1, size / 2);
            }
            
            /* Another loop after recursive call */
            for (int i = size - 1; i >= 0; i--) {
                g_shared_long[i % 512] += g_shared_array[i];
            }
        }
    }
}

/* Initialize shared data */
__attribute__((noinline, used))
void init_shared_data(void) {
    for (int i = 0; i < 1024; i++) {
        g_shared_array[i] = i;
    }
    for (int i = 0; i < 512; i++) {
        g_shared_long[i] = i * 2L;
    }
    
    /* Allocate and initialize pointer-based data */
    g_shared_ptr = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        g_shared_ptr[i] = i * 3;
    }
}

/* Compute checksum to verify execution */
__attribute__((noinline, used))
long compute_checksum(void) {
    long checksum = 0;
    
    __transaction_atomic {
        for (int i = 0; i < 1024; i++) {
            checksum += g_shared_array[i];
        }
        for (int i = 0; i < 512; i++) {
            checksum += g_shared_long[i];
        }
        if (g_shared_ptr) {
            for (int i = 0; i < 256; i++) {
                checksum += g_shared_ptr[i];
            }
        }
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    /* Use arguments to create non-constant loop bounds */
    int base_bound = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_bound <= 0) base_bound = 100;
    
    /* Initialize data */
    init_shared_data();
    
    /* Execute various TM patterns with different loop structures */
    
    /* Test 1: Simple array loop */
    tm_loop_transform1(0, base_bound);
    
    /* Test 2: Pointer-based loops with retry */
    tm_loop_transform2(g_shared_ptr, 256);
    
    /* Test 3: Volatile access patterns */
    tm_loop_transform3(g_shared_array, base_bound * 2);
    
    /* Test 4: Nested transaction patterns */
    tm_nested_transactions(3, base_bound);
    
    /* Additional tests with varying bounds */
    for (int i = 1; i <= 3; i++) {
        int bound = base_bound / i;
        if (bound > 10) {
            __transaction_atomic {
                /* Mixed loop types in single transaction */
                int j = 0;
                while (j < bound) {
                    g_shared_array[j] += i;
                    j++;
                }
                
                for (int k = bound - 1; k >= 0; k--) {
                    g_shared_long[k % 512] -= g_shared_array[k];
                }
            }
        }
    }
    
    /* Compute and print checksum to prevent optimization */
    long final_checksum = compute_checksum();
    printf("TM Loop Test Checksum: %ld\n", final_checksum);
    
    /* Cleanup */
    if (g_shared_ptr) {
        free(g_shared_ptr);
    }
    
    return (final_checksum != 0) ? 0 : 1;
}

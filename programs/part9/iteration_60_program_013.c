/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256];

/* Function 1: Complex loop with conditional branches and memory operations */
void test_complex_loop(int n, int *arr) {
    int i, j;
    volatile int sum = 0;
    
    /* Outer loop with data dependency */
    for (i = 0; i < n; i++) {
        /* Inner loop with conditional */
        for (j = 0; j < 8; j++) {
            /* Create complex data dependencies */
            int idx = (i * 8 + j) % 256;
            
            /* Conditional store with dependency chain */
            if (arr[idx] > 0) {
                sum += arr[idx] * 2;
                global_array[idx] = sum;
            } else {
                sum -= arr[idx];
                global_array[idx] = sum / 2;
            }
            
            /* Inline assembly to create specific RTL patterns */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Function call to create control flow boundary */
        if (i % 16 == 0) {
            global_counter++;
        }
    }
    
    /* Final store with barrier */
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Function 2: Nested loops with different iteration patterns */
void test_nested_loops(int rows, int cols, int matrix[][64]) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Triple nested loop for outer loop pipelining */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            /* Complex addressing with multiple dependencies */
            int base = matrix[i][j];
            
            for (k = 0; k < 4; k++) {
                /* Create anti-dependencies */
                int temp = base + k;
                
                /* Conditional with both paths used */
                if (temp & 1) {
                    acc = acc * 3 + temp;
                } else {
                    acc = acc / 2 - temp;
                }
                
                /* Memory store with dependency */
                global_array[(i * cols + j + k) % 256] = acc;
            }
            
            /* Create loop-carried dependency */
            matrix[i][j] = acc;
        }
        
        /* Periodic function-like operation */
        if (i % 8 == 0) {
            /* Use inline asm to prevent optimization */
            asm volatile ("# complex operation %0" : : "r"(acc));
        }
    }
}

/* Function 3: Switch statement with computed goto-like pattern */
void test_switch_complex(int mode, int iterations) {
    volatile int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Complex switch with multiple cases */
        switch ((mode + i) % 7) {
            case 0:
                state = state * 2 + 1;
                global_array[i % 256] = state;
                break;
            case 1:
                state = state / 3 - i;
                global_array[(i + 1) % 256] = state;
                break;
            case 2:
                state = state + global_counter;
                /* Fall through */
            case 3:
                state = state ^ 0x55AA;
                global_array[(i + 2) % 256] = state;
                break;
            case 4:
                /* Nested conditional */
                if (state > 1000) {
                    state = 0;
                } else {
                    state = state + 500;
                }
                break;
            case 5:
                state = state << 2;
                break;
            case 6:
                state = state >> 1;
                /* Memory barrier */
                asm volatile ("" : : "r"(state) : "memory");
                break;
        }
        
        /* Additional operation to create scheduling opportunities */
        if (i % 32 == 0) {
            global_counter = state;
        }
    }
}

/* Function 4: Mixed control flow with pointer chasing */
void test_pointer_chasing(int size, int *data) {
    volatile int result = 0;
    int *ptr = data;
    int count = 0;
    
    while (count < size && ptr != NULL) {
        /* Dereference with bounds check */
        int value = *ptr;
        
        /* Complex conditional chain */
        if (value < 0) {
            result = result - value;
            ptr = data + (value & 0xFF);
        } else if (value > 1000) {
            result = result + (value >> 2);
            ptr = data + ((value >> 8) & 0xFF);
        } else {
            result = result * 3 + value;
            ptr++;
        }
        
        /* Array access with complex index */
        global_array[result % 256] = value;
        
        /* Inline asm for specific RTL generation */
        asm volatile ("# pointer chase %0, %1" : : "r"(result), "r"(value));
        
        count++;
        
        /* Loop exit condition with side effect */
        if (count % 64 == 0) {
            global_counter = result;
        }
    }
}

/* Function 5: SIMD-like operations for vectorization opportunities */
void test_vector_like(int n, int *a, int *b, int *c) {
    int i;
    volatile int checksum = 0;
    
    /* Loop with multiple independent memory operations */
    for (i = 0; i < n; i += 4) {
        /* Multiple parallel computations */
        int t0 = a[i] * b[i];
        int t1 = a[i + 1] + b[i + 1];
        int t2 = a[i + 2] - b[i + 2];
        int t3 = a[i + 3] ^ b[i + 3];
        
        /* Conditional stores with dependencies */
        c[i] = t0;
        if (t0 > t1) {
            c[i + 1] = t1 * 2;
        } else {
            c[i + 1] = t1 / 2;
        }
        
        c[i + 2] = t2 + checksum;
        c[i + 3] = t3 | 0xFF;
        
        /* Update checksum with complex expression */
        checksum = (checksum + t0 - t1) * t2 + t3;
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : "r"(t0), "r"(t1), "r"(t2), "r"(t3) : "memory");
    }
    
    /* Final reduction */
    for (i = 0; i < 4; i++) {
        checksum = checksum ^ c[i];
    }
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    int test_data[256];
    int matrix[16][64];
    int i, j;
    
    /* Initialize data */
    for (i = 0; i < 256; i++) {
        test_data[i] = (i * 37) & 0xFF;
        global_array[i] = 0;
    }
    
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 64; j++) {
            matrix[i][j] = (i * 64 + j) * 3;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    test_complex_loop(32, test_data);
    test_nested_loops(8, 32, matrix);
    test_switch_complex(argc > 1 ? atoi(argv[1]) : 3, 128);
    test_pointer_chasing(100, test_data);
    
    int a[128], b[128], c[128];
    for (i = 0; i < 128; i++) {
        a[i] = i;
        b[i] = 128 - i;
    }
    test_vector_like(128, a, b, c);
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (i = 0; i < 256; i++) {
        final_result ^= global_array[i];
    }
    final_result += global_counter;
    
    /* Return something based on computation */
    return final_result & 0xFF;
}

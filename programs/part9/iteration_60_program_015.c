/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int *arr) {
    int i, j;
    volatile int sum = 0;
    
    /* Create data dependencies to force scheduling decisions */
    for (i = 0; i < n; i++) {
        /* Complex addressing to create memory RTL patterns */
        int idx = (i * 7) & 255;
        
        /* Conditional store with dependency chain */
        if (arr[i] > 0) {
            global_array[idx] = arr[i] + global_counter;
            sum += global_array[idx];
        } else {
            /* Different path with arithmetic */
            sum -= i * 2;
        }
        
        /* Inline asm to create unschedulable dependencies */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    global_counter += sum;
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int m, int n) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Outer loop with variable bound */
    for (i = 0; i < m; i++) {
        /* Middle loop */
        for (j = 0; j < n; j++) {
            /* Inner loop with small iteration count */
            for (k = 0; k < 3; k++) {
                /* Complex expression with multiple operations */
                int val = (i * j + k) * 3 - (i >> 2) + (j << 1);
                
                /* Conditional update */
                if (val & 1) {
                    acc += val * 2;
                } else {
                    acc -= val / 2;
                }
                
                /* Memory barrier to force scheduling boundaries */
                asm volatile ("" : : : "memory");
            }
            
            /* Function call to create control flow */
            if (j % 7 == 0) {
                /* Recursive-like pattern but not actually recursive */
                acc += global_array[j & 15];
            }
        }
        
        /* Store result periodically */
        if (i % 5 == 0) {
            global_array[i & 255] = acc;
        }
    }
    
    global_counter += acc;
}

/* Function 3: Switch statement with computed goto-like pattern */
void test_switch_complex(int x) {
    volatile int result = 0;
    int i;
    
    /* Loop with switch inside */
    for (i = 0; i < 100; i++) {
        /* Complex condition for switch */
        int case_val = (x + i * 3) % 7;
        
        switch (case_val) {
            case 0:
                result += i * 2;
                /* Memory operation */
                global_array[i & 255] = result;
                break;
            case 1:
                result -= i * 3;
                /* Inline asm with specific register constraints */
                asm volatile ("# dummy asm" : "+r"(result) : : "cc");
                break;
            case 2:
                result = result * 2 - i;
                break;
            case 3:
                result = result / (i + 1) + 5;
                break;
            case 4:
                /* Nested condition */
                if (result > 1000) {
                    result = 0;
                } else {
                    result += 100;
                }
                break;
            case 5:
                /* Complex expression */
                result = (result << 2) | (i & 3);
                break;
            case 6:
                /* Multiple operations */
                result = result ^ i;
                result = result * 3 + 7;
                break;
            default:
                result = -1;
        }
        
        /* Force dependency chain */
        asm volatile ("" : "+r"(result) : : );
    }
    
    global_counter ^= result;
}

/* Function 4: Mixed control flow with pointer arithmetic */
void test_mixed_control_flow(int *data, int size) {
    volatile int total = 0;
    int *ptr = data;
    int *end = data + size;
    
    /* While loop with pointer arithmetic */
    while (ptr < end) {
        int val = *ptr;
        
        /* Multiple if-else chains */
        if (val < 0) {
            total -= (-val) * 2;
            /* Store with offset */
            *(ptr + 1) = total;
        } else if (val < 100) {
            total += val + 5;
            /* Conditional store */
            if (total & 1) {
                global_array[val & 255] = total;
            }
        } else if (val < 200) {
            total += val / 2;
            /* Inline asm with memory clobber */
            asm volatile ("# complex pattern" : : "r"(val), "r"(total) : "memory");
        } else {
            total = total ^ val;
        }
        
        /* Update pointer with stride */
        ptr += (val % 3) + 1;
        
        /* Prevent tail merging */
        asm volatile ("" : : "r"(ptr), "r"(total) : );
    }
    
    global_counter += total;
}

/* Function 5: Array processing with reduction */
int test_reduction_pattern(int *arr, int n) {
    volatile int max_val = arr[0];
    volatile int min_val = arr[0];
    volatile int sum = 0;
    int i;
    
    /* Reduction loop with multiple accumulators */
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Multiple dependent operations */
        if (val > max_val) {
            max_val = val;
            /* Side effect */
            global_array[i & 255] = val;
        }
        
        if (val < min_val) {
            min_val = val;
        }
        
        sum += val * val - val;
        
        /* Create artificial dependency */
        asm volatile ("# reduction step" : "+r"(sum) : "r"(val) : );
    }
    
    /* Final computation with all values */
    int result = (max_val - min_val) * sum;
    
    /* Complex return expression */
    return (result & 0xFF) + global_counter;
}

/* Main driver function */
int main(int argc, char **argv) {
    int test_array[100];
    int i;
    
    /* Initialize test data */
    for (i = 0; i < 100; i++) {
        test_array[i] = (i * 37) % 123 - 50;
    }
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(50, test_array);
    test_nested_loops(10, 15);
    test_switch_complex(argc);
    test_mixed_control_flow(test_array, 100);
    int result = test_reduction_pattern(test_array, 100);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (counter: %d)\n", result, global_counter);
    
    return 0;
}

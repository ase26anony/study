/* Test program to trigger DDG edge initialization in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    static volatile int counter = 100;
    return counter;
}

static void __attribute__((noinline, noipa)) sink(int value) {
    static volatile int sink_var;
    sink_var = value;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on arr1[i] from previous iteration */
        int temp = arr1[i] + prev;           /* Uses prev from i-1 or earlier */
        
        /* WAR dependency: arr2[i] is read, then written */
        int read_val = arr2[i];
        arr2[i] = temp + read_val;           /* Anti-dependency on arr2[i] */
        
        /* WAW dependency on acc */
        acc = acc + temp;                    /* Output dependency chain on acc */
        
        /* Loop-carried dependency with distance 1 */
        prev = temp;                         /* True dependency to next iteration */
        
        /* Control dependency */
        if (acc > 1000) {
            arr1[i] = acc % 256;             /* Control-dependent store */
        }
    }
    
    sink(acc + prev);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* matrix) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* Complex memory access pattern */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Multiple dependencies forming potential cycles */
            int val1 = matrix[idx] + matrix[prev_idx];   /* RAW on matrix */
            int val2 = matrix[left_idx] * 2;             /* RAW on matrix */
            
            /* Cross-iteration dependency in inner loop */
            row_acc = row_acc + val1 + val2;             /* Loop-carried on row_acc */
            
            /* Store with potential WAW */
            matrix[idx] = row_acc % 100;
            
            /* Recurrence chain within iteration (potential micro-SCC) */
            int x = val1 + 1;
            int y = x * 2;
            x = y - val2;                                /* Cycle: x -> y -> x */
            row_acc += x;
        }
        
        sum += row_acc;
    }
    
    sink(sum);
}

/* Test 3: Pointer arithmetic and aliasing */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int* data, int* alias) {
    int* p1 = data;
    int* p2 = alias;
    
    /* Force potential aliasing */
    if (n > 10) {
        p2 = data + 5;  /* Create overlap */
    }
    
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n - 5; ++i) {
        /* Indirect accesses with potential aliasing */
        int val1 = p1[i];
        int val2 = p2[i];
        
        /* Complex dependency chain */
        acc1 = acc1 + val1 * 2;
        acc2 = acc2 + val2 * 3;
        
        /* Store with possible aliasing */
        p1[i + 1] = acc1 - acc2;
        
        /* Pointer-chasing dependency */
        if (i % 3 == 0) {
            p2[i] = acc1;                    /* May alias with p1[i+1] */
        }
        
        /* Cross-iteration through pointers */
        int* tmp = p1;
        p1 = p2;
        p2 = tmp;                            /* Swap pointers */
    }
    
    sink(acc1 + acc2);
}

/* Test 4: Mixed data types and control flow */
static void __attribute__((noinline, noipa))
test4_mixed_control_flow(int n, short* shorts, int* ints, char* chars) {
    int int_acc = 0;
    short short_acc = 0;
    
    volatile int threshold = 500;  /* Prevent constant propagation */
    
    for (int i = 1; i < n; ++i) {
        /* Mixed data type accesses */
        short s_val = shorts[i];
        int i_val = ints[i - 1];            /* RAW dependency */
        char c_val = chars[i % 256];
        
        /* Data type conversions creating dependencies */
        int mixed = (int)s_val + i_val + (int)c_val;
        
        /* Loop-carried on different variable types */
        int_acc = int_acc + mixed;
        short_acc = short_acc + (short)(mixed % 32768);
        
        /* Control dependency with loop-variant condition */
        if (int_acc > threshold) {
            shorts[i] = (short)int_acc;     /* Control-dependent write */
            threshold = int_acc / 2;         /* Modify control variable */
        }
        
        /* Output dependency on array */
        ints[i] = int_acc;                  /* WAW dependency chain */
        
        /* Anti-dependency through array */
        chars[i % 256] = (char)(short_acc % 256);
    }
    
    sink(int_acc + short_acc);
}

/* Test 5: Complex recurrence with multiple distances */
static void __attribute__((noinline, noipa))
test5_multiple_distances(int n, int* arr) {
    /* Initialize recurrence chain */
    int x = arr[0], y = arr[1], z = arr[2];
    int sum = 0;
    
    for (int i = 3; i < n; ++i) {
        /* Dependency chain with distance 1 */
        int new_x = y + z;
        
        /* Dependency with distance 2 */
        int new_y = x + arr[i - 2];
        
        /* Dependency with distance 3 */
        int new_z = arr[i] + arr[i - 3];
        
        /* Update recurrence chain */
        x = new_x;      /* Depends on y,z from previous iteration */
        y = new_y;      /* Depends on x from 2 iterations ago */
        z = new_z;      /* Depends on arr[i-3] from 3 iterations ago */
        
        /* Complex accumulation with multiple dependencies */
        sum = sum + x - y + z;
        
        /* Store creating memory dependencies at different distances */
        arr[i] = sum % 1000;
        
        /* Conditional with data-dependent condition */
        if ((x + y + z) % 7 == 0) {
            arr[i - 1] = sum;      /* May create additional dependencies */
        }
    }
    
    sink(sum + x + y + z);
}

/* Main driver */
int main(int argc, char** argv) {
    /* Use volatile to prevent compile-time computation */
    volatile int base_size = 1000;
    int n = base_size;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
    }
    
    /* Allocate test arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    short* shorts = (short*)malloc(n * sizeof(short));
    char* chars = (char*)malloc(256 * sizeof(char));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = (i * 13 + 7) % 100;
        arr2[i] = (i * 17 + 11) % 100;
        if (i < n) shorts[i] = (short)((i * 19 + 13) % 32768);
    }
    for (int i = 0; i < 256; ++i) {
        chars[i] = (char)((i * 23 + 17) % 256);
    }
    
    /* Matrix for nested loop test */
    int rows = 50, cols = 50;
    int* matrix = (int*)malloc(rows * cols * sizeof(int));
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = (i * 29 + 19) % 100;
    }
    
    /* Run all test cases */
    test1_register_memory_deps(n, arr1, arr2);
    test2_nested_loops_scc(rows, cols, matrix);
    test3_pointer_aliasing(n, arr1, arr2);
    test4_mixed_control_flow(n, shorts, arr1, chars);
    test5_multiple_distances(n, arr2);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < n && i < 100; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(shorts);
    free(chars);
    free(matrix);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static int __attribute__((noinline, noipa)) get_value(int idx) {
    volatile int v = idx * 3 + 7;
    return v;
}

static void __attribute__((noinline, noipa)) use_value(volatile int *sink, int val) {
    *sink = val;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int *arr, volatile int *sink) {
    int acc = 0;
    int prev = arr[0];
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* RAW (true) dependency: arr[i] depends on prev from previous iteration */
        int temp = prev + arr[i];           /* Memory + register dependency */
        
        /* WAR (anti) dependency: prev is read before being overwritten */
        int scaled = temp * 2;
        
        /* Output dependency: prev is written, creating WAW with next iteration */
        prev = scaled - i;                  /* Loop-carried, distance 1 */
        
        /* Register accumulation with loop-carried dependency */
        acc = acc + temp;                   /* Register RAW, distance 1 */
        
        /* Memory output dependency: store result */
        arr[i] = prev + acc;                /* Memory WAW */
        
        /* Control dependency based on loop-variant value */
        if (acc > 1000) {                   /* May create control dependency edges */
            prev = prev / 2;
        }
    }
    
    /* Prevent dead code elimination */
    use_value(sink, acc + prev);
}

/* Test 2: Nested loops for SCC formation */
static void __attribute__((noinline, noipa))
test2_nested_matrix(int n, int *mat, volatile int *sink) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < n; ++j) {
            /* Complex memory dependencies with multiple distances */
            int idx = i * n + j;
            int prev_idx = (i-1) * n + j;
            int diag_idx = (i-1) * n + (j-1);
            
            /* True dependencies from previous row and column */
            int val = mat[prev_idx] + mat[idx - 1] - mat[diag_idx];
            
            /* Anti-dependency: read before write */
            int old_val = mat[idx];
            
            /* Output dependency + recurrence chain within iteration */
            int x = val + old_val;
            int y = x * 3;
            x = y - 7;                      /* Creates cycle: x -> y -> x */
            
            mat[idx] = x + row_acc;
            
            /* Loop-carried in inner loop */
            row_acc = row_acc + mat[idx];   /* Distance 1 in inner loop */
        }
        
        /* Loop-carried in outer loop */
        sum = sum + row_acc;                /* Distance 1 in outer loop */
    }
    
    use_value(sink, sum);
}

/* Test 3: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test3_pointer_aliasing(int n, int *data, volatile int *sink) {
    int *ptr1 = data;
    int *ptr2 = data + n/2;
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Indirect accesses that may alias */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Cross-iteration dependencies through pointers */
        *ptr1 = val1 + acc1;
        *ptr2 = val2 + acc2;
        
        /* Swap accumulators between iterations */
        int tmp = acc1;
        acc1 = acc2 + val1;
        acc2 = tmp + val2;                  /* Distance 2 dependency chain */
        
        /* Pointer arithmetic creates complex address dependencies */
        ptr1++;
        ptr2--;
        
        /* Conditional with loop-variant condition */
        if (val1 > val2) {
            acc1 = acc1 - val2;             /* Control dependency */
        }
    }
    
    use_value(sink, acc1 + acc2);
}

/* Test 4: Mixed dependencies with function calls */
static void __attribute__((noinline, noipa))
test4_mixed_complex(int n, int *a, int *b, volatile int *sink) {
    int state = 0;
    int counter = 0;
    
    /* Loop with varying dependency distances */
    for (int i = 2; i < n; ++i) {
        /* Distance 1 memory dependency */
        int t1 = a[i-1] + b[i];
        
        /* Distance 2 memory dependency */
        int t2 = a[i-2] * t1;
        
        /* Register recurrence chain */
        int x = state + t2;
        int y = x * counter;
        state = y - i;                      /* Loop-carried, distance 1 */
        
        /* Output dependencies to memory */
        a[i] = state + t1;
        b[i] = t2 - state;
        
        /* Anti-dependency through counter */
        int old_counter = counter;
        counter = old_counter + (a[i] > b[i] ? 1 : -1);
        
        /* Complex condition with side effects */
        if (state % 3 == 0) {
            a[i] = a[i] * 2;                /* Control + memory dependency */
            state = state / 2;
        }
    }
    
    use_value(sink, state + counter);
}

/* Test 5: Reduction with multiple accumulators */
static void __attribute__((noinline, noipa))
test5_multiple_reductions(int n, int *arr, volatile int *sink) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple independent reduction chains */
        sum1 = sum1 + arr[i];
        sum2 = sum2 + arr[i] * i;
        sum3 = sum3 + (arr[i] > 0 ? arr[i] : -arr[i]);
        
        /* Product reductions with anti-dependencies */
        int old_prod1 = prod1;
        prod1 = old_prod1 * (arr[i] + 1);
        
        int old_prod2 = prod2;
        prod2 = old_prod2 * (arr[i] - 1);
        
        /* Cross-dependency between reduction chains */
        if (i % 4 == 0) {
            arr[i] = sum1 + prod1;          /* Mixes different chains */
        } else if (i % 4 == 1) {
            arr[i] = sum2 - prod2;
        } else if (i % 4 == 2) {
            arr[i] = sum3 * old_prod1;
        } else {
            arr[i] = old_prod2 / (sum1 + 1);
        }
        
        /* Loop-carried dependency with distance 2 */
        if (i >= 2) {
            arr[i] = arr[i] + arr[i-2];     /* Distance 2 memory dependency */
        }
    }
    
    use_value(sink, sum1 + sum2 + sum3 + prod1 + prod2);
}

int main(void) {
    volatile int sink = 0;
    int iterations = get_iterations();
    
    /* Allocate arrays with volatile initialization */
    int size = iterations + 10;
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *mat = (int*)malloc(size * size * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = get_value(i * 2);
    }
    
    for (int i = 0; i < size * size; ++i) {
        mat[i] = get_value(i);
    }
    
    /* Execute all test cases to trigger various DDG edge creations */
    test1_loop_carried_deps(iterations, arr1, &sink);
    test2_nested_matrix(iterations, mat, &sink);
    test3_pointer_aliasing(iterations, arr1, &sink);
    test4_mixed_complex(iterations, arr1, arr2, &sink);
    test5_multiple_reductions(iterations, arr2, &sink);
    
    /* Compute checksum */
    int checksum = sink;
    for (int i = 0; i < size; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(mat);
    
    return 0;
}

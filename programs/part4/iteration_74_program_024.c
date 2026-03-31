/* Test program to trigger DDG edge initialization in ddg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

static void __attribute__((noinline, noipa)) use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* arr1, int* arr2) {
    int acc = 0;
    int prev = arr1[0];
    
    for (int i = 1; i < n; ++i) {
        /* RAW dependency on arr1[i] from previous iteration */
        int temp = arr1[i] + prev;  
        
        /* WAR dependency: reading arr2[i] before writing to it */
        int read_before_write = arr2[i] * 2;
        
        /* WAW dependency: multiple writes to same variable */
        acc = acc + temp;            /* Write to acc */
        acc = acc + read_before_write; /* Another write to acc */
        
        /* Loop-carried memory dependency with distance 1 */
        arr2[i] = temp + acc;
        
        /* Anti-dependency (WAR) on prev */
        prev = arr1[i];
    }
    
    use(acc);
    use(arr2[n-1]);
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_loops_scc(int n, int m, int* mat) {
    int sum = 0;
    
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency */
        for (int j = 1; j < m; ++j) {
            /* RAW: read mat[(i-1)*m + j] before writing */
            int top = mat[(i-1)*m + j];
            
            /* RAW: read mat[i*m + (j-1)] before writing */
            int left = mat[i*m + (j-1)];
            
            /* Cycle within iteration: x depends on y, y depends on x */
            int x = top + left + row_acc;
            int y = x * 2 - left;
            row_acc = x + y;
            
            /* Write creates WAW with next iteration */
            mat[i*m + j] = row_acc;
        }
        
        sum += row_acc;
    }
    
    use(sum);
}

/* Test 3: Loop with control dependencies */
static void __attribute__((noinline, noipa))
test3_control_deps(int n, int* data, int* output) {
    int threshold = 50;
    int count = 0;
    int running_sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Loop-carried dependency on running_sum */
        running_sum += data[i];
        
        /* Control dependency: branch depends on loop-variant value */
        if (running_sum > threshold) {
            /* True dependency chain inside conditional */
            count = count + 1;
            output[i] = data[i] * count;
            
            /* Anti-dependency: read output[i-1] before potential write */
            if (i > 0) {
                output[i] += output[i-1];
            }
        } else {
            /* Different dependency chain in else path */
            output[i] = data[i] - running_sum;
        }
        
        /* Output dependency (WAW) on output[i] across iterations
           when control flow merges */
        output[i] = output[i] * 2;
    }
    
    use(count);
    use(running_sum);
}

/* Test 4: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int result = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing between ptr1 and ptr2 accesses */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Complex dependency chain */
        *ptr1 = val1 + val2 + result;
        result = *ptr1 - val2;
        
        /* Pointer increment creates WAW on memory locations */
        *ptr2 = result * val1;
        
        ptr1++;
        ptr2--;
        
        /* Loop-carried dependency through result */
        result = result + i;
    }
    
    use(result);
}

/* Test 5: Mixed data types and distances */
static void __attribute__((noinline, noipa))
test5_mixed_distances(int n, float* farr, int* iarr) {
    float f_acc = 0.0f;
    int i_acc = 0;
    
    /* Loop with multiple dependency distances */
    for (int i = 2; i < n; ++i) {
        /* Distance 2 memory dependency */
        float f1 = farr[i] + farr[i-2];
        
        /* Distance 1 register dependency */
        f_acc = f_acc * 0.9f + f1;
        
        /* Interleaved int operations with different distance */
        int i1 = iarr[i] + iarr[i-1];  /* Distance 1 */
        i_acc = i_acc ^ i1;            /* Loop-carried on i_acc */
        
        /* Cross-type dependency: float result affects int */
        iarr[i] = i_acc + (int)f_acc;
        
        /* Write with potential WAW on farr */
        farr[i-1] = f_acc;
    }
    
    volatile float fsink = f_acc;
    volatile int isink = i_acc;
    (void)fsink;
    (void)isink;
}

/* Test 6: Complex recurrence chain within iteration */
static void __attribute__((noinline, noipa))
test6_recurrence_chain(int n, int* arr) {
    for (int i = 1; i < n; ++i) {
        /* Cycle of dependencies within single iteration:
           a -> b -> c -> a (forms SCC in DDG) */
        int a = arr[i] + arr[i-1];
        int b = a * 2 - i;
        int c = b / 3 + arr[i];
        arr[i] = c + a;  /* Closes the cycle: depends on a and writes to arr[i] */
        
        /* Additional loop-carried dependency */
        arr[i] = arr[i] + arr[i-1];
    }
    
    use(arr[n-1]);
}

int main(void) {
    /* Use volatile to prevent compile-time known values */
    volatile int size = get_iterations();
    int n = size;
    
    /* Allocate arrays with dynamic sizes */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    int* data = (int*)malloc(n * sizeof(int));
    int* output = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; ++i) {
        arr1[i] = (i * 13) % 97;
        arr2[i] = (i * 17) % 101;
        data[i] = (i * 23) % 107;
        output[i] = 0;
        farr[i] = (float)(i % 29) * 0.5f;
    }
    
    /* Execute all test cases */
    test1_register_memory_deps(n, arr1, arr2);
    test2_nested_loops_scc(10, 10, arr1);  /* Smaller matrix */
    test3_control_deps(n, data, output);
    test4_pointer_aliasing(n, arr1);
    test5_mixed_distances(n, farr, arr2);
    test6_recurrence_chain(n, arr1);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + data[i] + output[i] + (int)farr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(data);
    free(output);
    free(farr);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static int __attribute__((noinline, noipa)) get_value(int x) {
    volatile int v = x;
    return v;
}

static void __attribute__((noinline, noipa)) sink(int x) {
    volatile int v = x;
    (void)v;
}

/* Test 1: Simple loop with register and memory dependencies */
static int __attribute__((noinline, noipa)) 
test1_loop_carried_deps(int n, int* arr1, int* arr2) {
    int acc = get_value(0);
    int prev = get_value(1);
    
    /* Complex loop-carried dependencies */
    for (int i = 0; i < n; ++i) {
        /* RAW: Read arr1[i], then write arr2[i] */
        int temp = arr1[i] + prev;  /* Uses prev from previous iteration */
        
        /* WAR: Write to arr2[i] after reading arr1[i] */
        arr2[i] = temp * 2;
        
        /* WAW: Multiple writes to 'prev' */
        prev = arr2[i] - arr1[i];   /* Output dependency with previous iteration */
        
        /* Register dependency chain */
        acc = acc + temp;           /* Loop-carried register dependency */
        
        /* Anti-dependency: Read arr1[i] before potentially aliased write */
        if (i > 0) {
            arr1[i-1] = acc % 100;  /* WAR with next iteration's arr1[i] read */
        }
    }
    
    return acc + prev;
}

/* Test 2: Nested loops for SCC formation */
static int __attribute__((noinline, noipa))
test2_nested_matrix(int n, int m, int* mat) {
    int sum = 0;
    
    /* Outer loop with dependency */
    for (int i = 1; i < n; ++i) {
        int row_acc = 0;
        
        /* Inner loop with loop-carried dependency - forms SCC */
        for (int j = 1; j < m; ++j) {
            /* True dependency within inner loop */
            int val = mat[(i-1)*m + j] + mat[i*m + (j-1)];
            
            /* Anti-dependency */
            mat[i*m + j] = val * 3;
            
            /* Output dependency */
            row_acc = row_acc + val;
            
            /* Control dependency */
            if (row_acc > 1000) {
                mat[i*m + j] /= 2;  /* Creates control flow edges */
            }
        }
        
        /* Loop-carried dependency between outer iterations */
        sum = sum + row_acc;
        
        /* Cross-iteration memory dependency */
        mat[i*m] = sum % 256;
    }
    
    return sum;
}

/* Test 3: Complex recurrence chain within single iteration */
static int __attribute__((noinline, noipa))
test3_recurrence_chain(int n, int* data) {
    int x = get_value(1);
    int y = get_value(2);
    int z = get_value(3);
    
    for (int i = 0; i < n; ++i) {
        /* Cycle of dependencies within one iteration */
        int t1 = x + data[i];      /* RAW: depends on x from prev iteration */
        int t2 = y * t1;           /* RAW: depends on y and t1 */
        int t3 = z - t2;           /* RAW: depends on z and t2 */
        
        /* Update all three - creates output dependencies */
        x = t3 + i;                /* WAW with next iteration's x read */
        y = t1 * t2;               /* WAW with next iteration's y read */
        z = t2 / (t3 + 1);         /* WAW with next iteration's z read */
        
        /* Memory anti-dependency */
        data[i] = x + y + z;       /* WAR with next iteration's data[i] read? */
        
        /* Control dependency based on loop-variant value */
        if (x > y) {
            z = z * 2;             /* Control flow edge */
        } else {
            y = y + z;             /* Alternative control flow edge */
        }
    }
    
    return x + y + z;
}

/* Test 4: Pointer arithmetic with aliasing */
static int __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* base) {
    int* ptr1 = base;
    int* ptr2 = base + n/2;
    int result = 0;
    
    for (int i = 0; i < n/2; ++i) {
        /* Potential aliasing creates complex memory dependencies */
        *ptr1 = *ptr2 + result;    /* RAW on *ptr2, WAR on result */
        
        result = result + *ptr1;   /* RAW on *ptr1, WAW on result */
        
        /* Pointer movement - creates varying access patterns */
        ptr1++;
        ptr2--;
        
        /* Conditional store based on computed value */
        if (result % 2 == 0) {
            *(ptr1 - 1) = result;  /* Control + memory dependency */
        }
    }
    
    return result;
}

/* Test 5: Mixed dependency distances */
static int __attribute__((noinline, noipa))
test5_variable_distance(int n, int* arr) {
    int sum = 0;
    
    /* Loop with distance > 1 dependencies */
    for (int i = 4; i < n; ++i) {
        /* Distance 4 true dependency */
        arr[i] = arr[i-4] * 2 + arr[i-2];  /* Depends on iterations i-4 and i-2 */
        
        /* Distance 2 anti-dependency */
        int temp = arr[i-2];       /* Read arr[i-2] */
        arr[i-2] = sum;            /* Write arr[i-2] - WAR with iteration i */
        
        /* Register dependency chain with distance 1 */
        sum = sum + temp + arr[i];
        
        /* Output dependency with distance 3 */
        if (i % 3 == 0) {
            arr[i-3] = sum % 100;  /* WAW with iteration i-3's future read? */
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    /* Allocate arrays with volatile elements to prevent optimization */
    int size = N > 100 ? N : 100;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* mat = (int*)malloc(size * size * sizeof(int));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < size; ++i) {
        arr1[i] = get_value(i);
        arr2[i] = get_value(i * 2);
    }
    
    for (int i = 0; i < size * size; ++i) {
        mat[i] = get_value(i % 100);
    }
    
    int result = 0;
    
    /* Execute all test cases to trigger various DDG edge creations */
    result += test1_loop_carried_deps(N, arr1, arr2);
    result += test2_nested_matrix(50, 50, mat);
    result += test3_recurrence_chain(N, arr1);
    result += test4_pointer_aliasing(N, arr2);
    result += test5_variable_distance(N, arr1);
    
    /* Sink the result to prevent dead code elimination */
    sink(result);
    
    printf("Result checksum: %d\n", result);
    
    free(arr1);
    free(arr2);
    free(mat);
    
    return 0;
}

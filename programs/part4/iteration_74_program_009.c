/* Test program to trigger Data Dependency Graph edge initialization */
#include <stdio.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
static int __attribute__((noinline, noipa)) get_iterations(void) {
    volatile int n = 100;
    return n;
}

/* Dummy function to create memory barriers */
static void __attribute__((noinline, noipa)) memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test 1: Simple loop with register and memory dependencies */
static void __attribute__((noinline, noipa)) 
test1_register_memory_deps(int n, int* a, int* b, int* c) {
    int i;
    int acc = 0;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* True dependency (RAW) on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Anti dependency (WAR) on acc */
        int temp = acc;
        acc = temp + c[i];
        
        /* Output dependency (WAW) on b[i] */
        b[i] = acc * 2;
        
        /* Loop-carried dependency with distance 2 */
        if (i >= 2) {
            c[i] = c[i-2] + 1;
        }
    }
    
    /* Volatile sink to prevent elimination */
    volatile int sink = acc + a[n-1] + b[n-1];
    (void)sink;
}

/* Test 2: Nested loops forming SCCs */
static void __attribute__((noinline, noipa))
test2_nested_scc(int n, int m, int mat[][100]) {
    int i, j;
    
    for (i = 1; i < n; i++) {
        for (j = 1; j < m; j++) {
            /* Complex dependencies forming potential SCC */
            int diag = mat[i-1][j-1];
            int left = mat[i][j-1];
            int up = mat[i-1][j];
            
            /* Recurrence chain within iteration */
            int x = diag + left;
            int y = x * 2;
            mat[i][j] = y + up;
            
            /* Cross-iteration dependency */
            mat[i][j] += mat[i][j-1] % 7;
        }
    }
    
    volatile int sink = mat[n-1][m-1];
    (void)sink;
}

/* Test 3: Loop with control dependencies */
static void __attribute__((noinline, noipa))
test3_control_deps(int n, int* data, int* output) {
    int i;
    int threshold = 50;
    int count = 0;
    
    for (i = 0; i < n; i++) {
        /* Control dependency based on loop-variant value */
        if (data[i] > threshold) {
            output[i] = data[i] * 2;
            count++;
        } else {
            output[i] = data[i] / 2;
        }
        
        /* Loop-carried dependency through count */
        data[i] += count;
        
        /* Anti dependency through temporary */
        int old_val = output[i];
        output[i] = old_val + i;
    }
    
    volatile int sink = count + output[n-1];
    (void)sink;
}

/* Test 4: Pointer arithmetic with aliasing */
static void __attribute__((noinline, noipa))
test4_pointer_aliasing(int n, int* arr) {
    int* p = arr;
    int* q = arr + n/2;
    int i;
    
    for (i = 0; i < n/2; i++) {
        /* Potential aliasing through pointers */
        *p = *q + i;
        *q = *p - i;
        
        /* Loop-carried through pointer movement */
        p++;
        q--;
        
        /* Memory barrier to prevent reordering */
        memory_barrier();
    }
    
    volatile int sink = arr[0] + arr[n-1];
    (void)sink;
}

/* Test 5: Complex recurrence with multiple distances */
static void __attribute__((noinline, noipa))
test5_multi_distance(int n, int* x, int* y) {
    int i;
    
    /* Initialize first few elements */
    if (n > 0) x[0] = 1;
    if (n > 1) x[1] = 2;
    if (n > 2) x[2] = 3;
    
    for (i = 3; i < n; i++) {
        /* Dependencies with different distances */
        y[i] = x[i-1] + x[i-2];      /* distances 1 and 2 */
        x[i] = y[i-1] + x[i-3];      /* distances 1 and 3 */
        
        /* Output dependency on y */
        y[i] = y[i] * 2;
        
        /* Anti dependency through temporary */
        int tmp = x[i];
        x[i] = tmp + i % 5;
    }
    
    volatile int sink = x[n-1] + y[n-1];
    (void)sink;
}

/* Test 6: Reduction with if-converted dependencies */
static void __attribute__((noinline, noipa))
test6_reduction_control(int n, int* data) {
    int i;
    int sum_even = 0;
    int sum_odd = 0;
    int prod = 1;
    
    for (i = 0; i < n; i++) {
        /* Control flow converted to data dependencies */
        int is_even = (data[i] % 2 == 0);
        sum_even += is_even ? data[i] : 0;
        sum_odd += is_even ? 0 : data[i];
        
        /* Loop-carried output dependency */
        prod *= (data[i] % 10) + 1;
        
        /* Cross-iteration anti dependency */
        data[i] = sum_even + sum_odd;
    }
    
    volatile int sink = sum_even + sum_odd + prod;
    (void)sink;
}

int main(void) {
    const int N = 100;
    const int M = 100;
    
    /* Dynamically allocate to avoid constant propagation */
    int* array1 = (int*)malloc(N * sizeof(int));
    int* array2 = (int*)malloc(N * sizeof(int));
    int* array3 = (int*)malloc(N * sizeof(int));
    int* output = (int*)malloc(N * sizeof(int));
    
    int (*matrix)[100] = (int(*)[100])malloc(N * 100 * sizeof(int));
    
    /* Initialize with volatile-like pattern */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3) % 97;
        array2[i] = (i * 5) % 89;
        array3[i] = (i * 7) % 83;
        output[i] = 0;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (i * j) % 79;
        }
    }
    
    /* Get non-constant iteration count */
    int iterations = get_iterations();
    
    /* Execute all test cases */
    test1_register_memory_deps(iterations, array1, array2, array3);
    test2_nested_scc(iterations, M, matrix);
    test3_control_deps(iterations, array1, output);
    test4_pointer_aliasing(iterations, array2);
    test5_multi_distance(iterations, array3, output);
    test6_reduction_control(iterations, array1);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i] + array3[i] + output[i];
        if (i < M) checksum += matrix[i][i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(output);
    free(matrix);
    
    return 0;
}

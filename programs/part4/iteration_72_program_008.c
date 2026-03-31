/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile int sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile double sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: Small count (count <= 2) with int array */
static void __attribute__((noinline))
test_small_count_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Mixed lvalue/rvalue contexts */
    for (int i = 0; i < 10; i++) {
        /* Store context (lvalue) */
        for (int j = lo; j <= hi; j++) {
            arr[i][j] = i * 100 + j;
        }
        
        /* Load context (rvalue) */
        for (int j = lo; j <= hi; j++) {
            int val = arr[i][j];
            use_int(val);
        }
    }
    
    /* Force volatility for index calculations */
    volatile int vlo = 7;
    volatile int vhi = 8;  /* count = 2 */
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Another pattern with volatile-derived bounds */
    for (int i = 0; i < 5; i++) {
        arr[i][actual_lo] = i * 10;
        arr[i][actual_hi] = i * 20;
        
        use_int(arr[i][actual_lo] + arr[i][actual_hi]);
    }
}

/* Test 2: Larger count with double array */
static void __attribute__((noinline))
test_large_count_double(void) {
    double matrix[15][25] = {0};
    
    /* Constant bounds for larger slice */
    const int lo = 3;
    const int hi = 12;  /* count = 10 > 2 */
    
    /* Store to slice */
    for (int i = 0; i < 15; i++) {
        for (int j = lo; j <= hi; j++) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Load from slice with volatile consumption */
    volatile double sum = 0;
    for (int i = 2; i < 8; i++) {
        for (int j = lo; j <= hi; j++) {
            sum += matrix[i][j];
        }
    }
    
    /* Mixed access pattern */
    for (int j = lo; j <= hi; j++) {
        /* Both store and load in same expression */
        matrix[10][j] = matrix[0][j] * 2.0;
        use_double(matrix[10][j]);
    }
}

/* Test 3: Single element count (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    char buffer[50][100];
    
    const int lo = 42;
    const int hi = 42;  /* count = 1 */
    
    /* Initialize slice */
    for (int i = 0; i < 50; i++) {
        buffer[i][lo] = (char)(i + 'A');
    }
    
    /* Copy slice to another location */
    char dest[50];
    for (int i = 0; i < 50; i++) {
        dest[i] = buffer[i][lo];
        use_int(dest[i]);
    }
    
    /* Volatile index forcing middle-end analysis */
    volatile int vidx = 30;
    int idx = vidx;
    
    /* Single element access with volatile index */
    buffer[25][idx] = 'Z';
    use_int(buffer[25][idx]);
}

/* Test 4: VLA with constant size */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Constant bounds within VLA */
    const int lo = 10;
    const int hi = 19;  /* count = 10 > 2 */
    
    /* Initialize VLA slice */
    for (int i = 0; i < n; i++) {
        for (int j = lo; j <= hi; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Process slice with volatile sink */
    volatile int checksum = 0;
    for (int i = 5; i < 15; i++) {
        for (int j = lo; j <= hi; j++) {
            checksum += vla[i][j];
        }
    }
    
    /* Mixed lvalue/rvalue in same loop */
    for (int j = lo; j <= hi; j++) {
        vla[20][j] = vla[0][j] * 2;
        use_int(vla[20][j]);
    }
}

/* Test 5: Different element sizes to vary TYPE_SIZE calculation */
static void __attribute__((noinline))
test_mixed_element_sizes(void) {
    /* Test with long long for larger element size */
    long long big_arr[20][15];
    
    const int lo = 2;
    const int hi = 5;  /* count = 4 > 2 */
    
    /* Store context */
    for (int i = 0; i < 20; i++) {
        for (int j = lo; j <= hi; j++) {
            big_arr[i][j] = (long long)i << 32 | j;
        }
    }
    
    /* Load context with pointer arithmetic */
    for (int i = 0; i < 20; i++) {
        long long *slice_start = &big_arr[i][lo];
        for (int k = 0; k <= (hi - lo); k++) {
            use_ptr(slice_start + k);  /* Force MEM_P consideration */
        }
    }
    
    /* Test with short for smaller element size */
    short small_arr[100][50];
    const int slo = 20;
    const int shi = 40;  /* count = 21 > 2 */
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = slo; j <= shi; j++) {
            small_arr[i][j] = (short)(i + j);
        }
    }
    
    /* Process */
    volatile long total = 0;
    for (int i = 0; i < 100; i++) {
        for (int j = slo; j <= shi; j++) {
            total += small_arr[i][j];
        }
    }
}

/* Test 6: Complex index expressions that simplify to constants */
static void __attribute__((noinline))
test_complex_const_bounds(void) {
    int arr3d[5][10][15] = {0};
    
    /* Complex but compile-time constant expressions */
    const int dim1 = 3;
    const int dim2_low = 2 * 2;      /* = 4 */
    const int dim2_high = 10 - 3;    /* = 7, count = 4 > 2 */
    
    /* Access 2D slice of 3D array */
    for (int i = 0; i < 5; i++) {
        /* Store to slice */
        for (int j = dim2_low; j <= dim2_high; j++) {
            arr3d[i][dim1][j] = i * 100 + dim1 * 10 + j;
        }
        
        /* Load from slice with computation */
        int row_sum = 0;
        for (int j = dim2_low; j <= dim2_high; j++) {
            row_sum += arr3d[i][dim1][j];
        }
        use_int(row_sum);
    }
    
    /* Volatile forcing of constant propagation */
    volatile int vbase = 1;
    int base = vbase;
    const int offset_low = base + 0;  /* Should be analyzable as constant */
    const int offset_high = base + 2; /* count = 3 > 2 */
    
    /* This should still trigger constant bounds analysis */
    for (int k = 0; k < 15; k++) {
        arr3d[2][offset_low][k] = k * 10;
        arr3d[2][offset_high][k] = k * 20;
        
        use_int(arr3d[2][offset_low][k] + arr3d[2][offset_high][k]);
    }
}

int main(void) {
    printf("Starting array bounds tests...\n");
    
    /* Run all tests */
    test_small_count_int();
    test_large_count_double();
    test_single_element();
    test_vla_constant_bounds();
    test_mixed_element_sizes();
    test_complex_const_bounds();
    
    printf("Tests completed.\n");
    
    /* Final volatile sink to prevent optimization */
    volatile int final_sink = 0;
    for (int i = 0; i < 100; i++) {
        final_sink += i;
    }
    
    printf("Checksum: %d\n", final_sink);
    return 0;
}

/* test_expr_coverage.c */
#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile static int sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile static double sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile static void *sink;
    sink = ptr;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline, noipa))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - write to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - read from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] + 1;
    use_int(arr[3][lo]);
}

/* Test 2: Multi-dimensional int array with medium slice (count > 2) */
static void __attribute__((noinline, noipa))
test_medium_slice_int(void) {
    int arr[15][25] = {0};
    volatile int start = 3;
    volatile int end = 12;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[7][j] = j * 100;
    }
    
    /* Read from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[7][j];
    }
    use_int(sum);
    
    /* Partial slice copy within same row */
    for (int j = lo + 2; j <= hi - 2; ++j) {
        arr[7][j] = arr[7][j - 1] + arr[7][j + 1];
    }
    use_int(arr[7][lo + 5]);
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline, noipa))
test_double_slice(void) {
    double matrix[8][16] = {0};
    volatile int start = 4;
    volatile int end = 13;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[2][j] = j * 1.5;
    }
    
    /* Compute using slice */
    double prod = 1.0;
    for (int j = lo; j <= hi; ++j) {
        prod *= matrix[2][j];
    }
    use_double(prod);
    
    /* Copy between slices */
    for (int j = lo; j <= hi; ++j) {
        matrix[3][j] = matrix[2][j];
    }
    use_double(matrix[3][lo + 3]);
}

/* Test 4: Char array with single element slice (count = 1) */
static void __attribute__((noinline, noipa))
test_char_slice(void) {
    char grid[32][64] = {0};
    volatile int start = 31;
    volatile int end = 31;  /* count = 1 */
    int lo = start;
    int hi = end;
    
    grid[10][lo] = 'A';
    char c = grid[10][lo];
    use_int((int)c);
    
    /* Adjacent pair (count = 2) */
    grid[10][lo] = grid[10][lo + 1];
    grid[10][lo + 1] = c;
    use_int((int)grid[10][lo] + (int)grid[10][lo + 1]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline, noipa))
test_vla_slice(void) {
    const int n = 30;
    int vla[n][n];
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize diagonal slice */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j] = j * j;
    }
    
    /* Copy to another slice */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j + 5] = vla[j][j];
    }
    
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[j][j + 5];
    }
    use_int(sum);
}

/* Test 6: Mixed operations with compile-time constant bounds */
static void __attribute__((noinline, noipa))
test_const_bounds(void) {
    int table[50][40] = {0};
    
    /* Direct constant bounds - should be recognized */
    for (int j = 5; j <= 14; ++j) {  /* count = 10 */
        table[20][j] = j * 3;
    }
    
    /* Another with count = 2 */
    table[20][5] = table[20][6];
    table[20][6] = table[20][5] + 1;
    
    /* Use volatile to force middle-end analysis */
    volatile int idx = 5;
    int lo_idx = idx;
    int hi_idx = idx + 9;  /* count = 10 */
    
    for (int j = lo_idx; j <= hi_idx; ++j) {
        table[21][j] = table[20][j] * 2;
    }
    
    int total = 0;
    for (int j = 5; j <= 14; ++j) {
        total += table[21][j];
    }
    use_int(total);
}

/* Test 7: Struct array to test different TYPE_SIZE */
struct point {
    int x;
    int y;
    double z;
};

static void __attribute__((noinline, noipa))
test_struct_slice(void) {
    struct point points[20][15] = {0};
    volatile int start = 3;
    volatile int end = 7;  /* count = 5 */
    int lo = start;
    int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        points[5][j].x = j;
        points[5][j].y = j * 2;
        points[5][j].z = j * 0.5;
    }
    
    /* Copy slice */
    for (int j = lo; j <= hi; ++j) {
        points[6][j] = points[5][j];
    }
    
    use_int(points[6][lo + 2].x);
}

int main(void) {
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_medium_slice_int();
    test_double_slice();
    test_char_slice();
    test_vla_slice();
    test_const_bounds();
    test_struct_slice();
    
    printf("All tests completed\n");
    
    /* Create a checksum to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += 1;  /* Dummy operation */
    
    return checksum - 1;  /* Return 0 */
}

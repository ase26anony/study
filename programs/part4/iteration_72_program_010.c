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
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        /* Store context */
        arr[3][j] = j * 10;
        
        /* Load context */
        int val = arr[3][j];
        use_int(val);
    }
    
    /* Another access pattern with count = 1 */
    int single_val = arr[3][7];
    use_int(single_val);
    arr[3][7] = 99;
}

/* Test 2: Multi-dimensional double array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[15][25] = {0.0};
    
    /* Constant bounds for larger slice */
    const int start = 3;
    const int end = 12;  /* count = 10 */
    
    volatile int vstart = start;
    volatile int vend = end;
    int lo_idx = vstart;
    int hi_idx = vend;
    
    /* Store operation on slice */
    for (int col = lo_idx; col <= hi_idx; ++col) {
        matrix[5][col] = col * 1.5;
    }
    
    /* Load operation from same slice */
    for (int col = lo_idx; col <= hi_idx; ++col) {
        double val = matrix[5][col];
        use_double(val);
    }
}

/* Test 3: Char array with medium slice, testing TYPE_SIZE calculation */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[8][64] = {0};
    
    /* Constant bounds */
    const int low = 10;
    const int high = 19;  /* count = 10, but char size is 1 */
    
    volatile int vl = low;
    volatile int vh = high;
    int l = vl;
    int h = vh;
    
    /* Mixed operations */
    for (int pos = l; pos <= h; ++pos) {
        /* Store */
        buffer[2][pos] = 'A' + (pos % 26);
        
        /* Load */
        char c = buffer[2][pos];
        use_int((int)c);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant slice bounds */
    const int slice_start = 5;
    const int slice_end = 14;  /* count = 10 */
    
    volatile int vs = slice_start;
    volatile int ve = slice_end;
    int s = vs;
    int e = ve;
    
    /* Process a row slice */
    for (int col = s; col <= e; ++col) {
        /* Read-modify-write */
        int old = vla[10][col];
        vla[10][col] = old * 2;
        use_int(vla[10][col]);
    }
}

/* Test 5: Struct array to test different elttype sizes */
struct mixed {
    int a;
    double b;
    char c[4];
};

static void __attribute__((noinline))
test_struct_slice(void) {
    struct mixed grid[5][8] = {0};
    
    /* Different slice sizes */
    const int bounds[][2] = {{1, 2}, {0, 5}};  /* count = 2 and 6 */
    
    for (int b = 0; b < 2; ++b) {
        volatile int vlo = bounds[b][0];
        volatile int vhi = bounds[b][1];
        int lo = vlo;
        int hi = vhi;
        
        for (int idx = lo; idx <= hi; ++idx) {
            /* Store */
            grid[2][idx].a = idx * 100;
            grid[2][idx].b = idx * 3.14;
            
            /* Load */
            use_int(grid[2][idx].a);
            use_double(grid[2][idx].b);
        }
    }
}

/* Test 6: Complex index calculation that simplifies to constants */
static void __attribute__((noinline))
test_computed_bounds(void) {
    int table[50][100] = {0};
    
    /* Computed but compile-time constant bounds */
    const int base = 20;
    volatile int vbase = base;
    int b = vbase;
    
    /* These should be recognized as constants by the middle-end */
    int lower = b + 3;    /* 23 */
    int upper = b + 7;    /* 27, count = 5 */
    
    /* Force computation in middle-end */
    volatile int vlower = lower;
    volatile int vupper = upper;
    int lo = vlower;
    int hi = vupper;
    
    /* Access pattern that might trigger MEM_P analysis */
    for (int x = lo; x <= hi; ++x) {
        table[25][x] = x * x;
        int result = table[25][x] + table[25][x-1];
        use_int(result);
    }
}

/* Main function that runs all tests */
int main(void) {
    volatile int checksum = 0;
    
    printf("Starting array slice tests...\n");
    
    /* Run all test functions */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_double();
    checksum += 2;
    
    test_char_slice();
    checksum += 3;
    
    test_vla_constant_size();
    checksum += 4;
    
    test_struct_slice();
    checksum += 5;
    
    test_computed_bounds();
    checksum += 6;
    
    /* Use checksum to prevent dead code elimination */
    volatile int final = checksum;
    printf("Tests completed. Checksum: %d\n", final);
    
    return final > 0 ? 0 : 1;
}

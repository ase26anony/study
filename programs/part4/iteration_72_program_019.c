/* Test program for expr.cc lines 7691-7700 */
#include <stdio.h>
#include <string.h>

/* Dummy functions to prevent optimization */
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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    volatile int vlo = lo;  /* Force middle-end analysis */
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
    arr[3][7] = single_val + 1;
    use_int(arr[3][7]);
}

/* Test 2: 2D double array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[15][25] = {0};
    
    /* Constant bounds */
    const int start = 3;
    const int end = 12;  /* count = 10 */
    
    volatile int vstart = start;
    volatile int vend = end;
    int lo_idx = vstart;
    int hi_idx = vend;
    
    /* Mixed store/load operations */
    for (int col = lo_idx; col <= hi_idx; ++col) {
        /* Store operation */
        matrix[5][col] = col * 1.5;
        
        /* Load operation with computation */
        double loaded = matrix[5][col];
        matrix[5][col] = loaded * 2.0;
        
        use_double(matrix[5][col]);
    }
    
    /* Block copy within same row (should trigger MEM_P logic) */
    for (int col = lo_idx; col <= hi_idx; ++col) {
        matrix[6][col] = matrix[5][col];
    }
}

/* Test 3: 3D char array with varying element sizes */
static void __attribute__((noinline))
test_char_array_slices(void) {
    char data[5][10][15] = {0};
    
    /* Test different slice sizes */
    const int bounds[][2] = {{1, 2}, {3, 8}, {9, 12}};
    
    for (int b = 0; b < 3; ++b) {
        volatile int vlo = bounds[b][0];
        volatile int vhi = bounds[b][1];
        int lo = vlo;
        int hi = vhi;
        
        /* Store context: initialize slice */
        for (int k = lo; k <= hi; ++k) {
            data[2][3][k] = 'A' + k;
        }
        
        /* Load context: read and process slice */
        for (int k = lo; k <= hi; ++k) {
            char c = data[2][3][k];
            data[2][4][k] = c + 32;  /* Convert to lowercase */
            use_int(data[2][4][k]);
        }
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant slice bounds */
    const int row_slice_start = 10;
    const int row_slice_end = 19;  /* count = 10 */
    
    volatile int vstart = row_slice_start;
    volatile int vend = row_slice_end;
    int lo = vstart;
    int hi = vend;
    
    /* Process a row slice */
    int sum = 0;
    for (int col = lo; col <= hi; ++col) {
        /* Both read and write */
        int val = vla[15][col];
        vla[15][col] = val * 2;
        sum += vla[15][col];
    }
    use_int(sum);
    
    /* Small slice in another dimension */
    const int small_lo = 5;
    const int small_hi = 6;  /* count = 2 */
    volatile int vsmall_lo = small_lo;
    volatile int vsmall_hi = small_hi;
    
    for (int row = vsmall_lo; row <= vsmall_hi; ++row) {
        vla[row][20] = row * 100;
        use_int(vla[row][20]);
    }
}

/* Test 5: Mixed types and access patterns */
static void __attribute__((noinline))
test_mixed_patterns(void) {
    struct Mixed {
        int a[8][12];
        double b[4][6];
        char c[20][30];
    } m;
    
    /* Initialize */
    memset(&m, 0, sizeof(m));
    
    /* Test int array slice with count = 3 */
    {
        const int lo = 2;
        const int hi = 4;  /* count = 3 */
        volatile int vlo = lo;
        volatile int vhi = hi;
        
        for (int j = vlo; j <= vhi; ++j) {
            m.a[3][j] = j * 100;
            int val = m.a[3][j];
            m.a[4][j] = val + 1;
            use_int(m.a[4][j]);
        }
    }
    
    /* Test double array slice with count = 5 */
    {
        const int lo = 1;
        const int hi = 5;  /* count = 5 */
        volatile int vlo = lo;
        volatile int vhi = hi;
        
        for (int j = vlo; j <= vhi; ++j) {
            m.b[2][j] = j * 1.1;
            double val = m.b[2][j];
            m.b[3][j] = val * 2.0;
            use_double(m.b[3][j]);
        }
    }
    
    /* Test char array slice with count = 8 */
    {
        const int lo = 10;
        const int hi = 17;  /* count = 8 */
        volatile int vlo = lo;
        volatile int vhi = hi;
        
        for (int j = vlo; j <= vhi; ++j) {
            m.c[5][j] = '0' + (j % 10);
            char val = m.c[5][j];
            m.c[6][j] = val + 1;
            use_int(m.c[6][j]);
        }
    }
}

/* Test 6: Pointer-based access with constant offsets */
static void __attribute__((noinline))
test_pointer_slices(void) {
    int buffer[50][40] = {0};
    
    /* Get pointer to middle of array */
    int (*ptr)[40] = &buffer[10];
    
    /* Constant slice bounds */
    const int offset_lo = 5;
    const int offset_hi = 14;  /* count = 10 */
    
    volatile int vlo = offset_lo;
    volatile int vhi = offset_hi;
    
    /* Access through pointer with constant bounds */
    for (int i = 0; i < 5; ++i) {
        for (int j = vlo; j <= vhi; ++j) {
            ptr[i][j] = i * 100 + j;
            int val = ptr[i][j];
            ptr[i+5][j] = val * 2;
            use_int(ptr[i+5][j]);
        }
    }
}

int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_double();
    checksum += 2;
    
    test_char_array_slices();
    checksum += 3;
    
    test_vla_constant_bounds();
    checksum += 4;
    
    test_mixed_patterns();
    checksum += 5;
    
    test_pointer_slices();
    checksum += 6;
    
    /* Final sink to prevent optimization */
    volatile int final_sink = checksum;
    printf("Test checksum: %d\n", final_sink);
    
    return 0;
}

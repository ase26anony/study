/* Test program for expr.cc lines 7691-7700 - constant bounds array slice operations */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) use_int(int val) {
    volatile int sink = val;
    return sink;
}

static double __attribute__((noinline, noipa)) use_double(double val) {
    volatile double sink = val;
    return sink;
}

static void __attribute__((noinline, noipa)) use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice - rvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        temp[j - actual_lo] = arr[3][j];
    }
    
    /* Store to slice - lvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[7][j] = temp[j - actual_lo] * 2;
    }
    
    /* Consume results */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        use_int(arr[3][j]);
        use_int(arr[7][j]);
    }
}

/* Test 2: Larger slice with int elements (count > 2) */
static void __attribute__((noinline)) test_large_int_slice(void) {
    int grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            grid[i][j] = (i << 16) | j;
        }
    }
    
    /* Constant bounds for larger slice */
    const int lo = 10;
    const int hi = 19;  /* count = 10 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Mixed lvalue/rvalue operations */
    int buffer[10];
    
    /* Read slice - rvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        buffer[j - actual_lo] = grid[25][j];
    }
    
    /* Write slice - lvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        grid[50][j] = buffer[j - actual_lo] + 1000;
    }
    
    /* Another write with different pattern */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        grid[75][j] = grid[25][j] + grid[50][j];
    }
    
    /* Consume */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        use_int(grid[25][j]);
        use_int(grid[50][j]);
        use_int(grid[75][j]);
    }
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds */
    const int lo = 3;
    const int hi = 8;  /* count = 6 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    double temp[6];
    
    /* Read slice */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        temp[j - actual_lo] = matrix[2][j];
    }
    
    /* Write slice */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        matrix[5][j] = temp[j - actual_lo] * 2.0;
    }
    
    /* Cross-copy between slices */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        matrix[8][j] = matrix[2][j] + matrix[5][j];
    }
    
    /* Consume */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        use_double(matrix[2][j]);
        use_double(matrix[5][j]);
        use_double(matrix[8][j]);
    }
}

/* Test 4: Char array with single element slice (count = 1) */
static void __attribute__((noinline)) test_char_single(void) {
    char bytes[50][100];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 100; j++) {
            bytes[i][j] = (i + j) & 0xFF;
        }
    }
    
    /* Single element - count = 1 */
    const int lo = 42;
    const int hi = 42;
    
    volatile int vlo = lo;
    int actual_lo = vlo;
    
    /* Both contexts for single element */
    char val = bytes[10][actual_lo];  /* rvalue */
    bytes[20][actual_lo] = val + 1;   /* lvalue */
    bytes[30][actual_lo] = bytes[10][actual_lo] + bytes[20][actual_lo]; /* mixed */
    
    /* Consume */
    use_int(bytes[10][actual_lo]);
    use_int(bytes[20][actual_lo]);
    use_int(bytes[30][actual_lo]);
}

/* Test 5: VLA with constant size (still VLA in syntax) */
static void __attribute__((noinline)) test_vla_constant(void) {
    const int n = 30;  /* Constant but VLA syntax */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds */
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Slice operations */
    int buffer[10];
    
    /* Read */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        buffer[j - actual_lo] = vla[10][j];
    }
    
    /* Write */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        vla[20][j] = buffer[j - actual_lo] * 3;
    }
    
    /* Consume */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        use_int(vla[10][j]);
        use_int(vla[20][j]);
    }
}

/* Test 6: Mixed operations in same function to trigger different paths */
static void __attribute__((noinline)) test_mixed_patterns(void) {
    int data[5][15];
    
    /* Initialize */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 15; j++) {
            data[i][j] = i * 20 + j;
        }
    }
    
    /* Test multiple slice sizes in same function */
    
    /* Pattern A: count = 2 */
    {
        const int lo_a = 3;
        const int hi_a = 4;
        volatile int vlo = lo_a;
        volatile int vhi = hi_a;
        
        int temp = data[1][vlo] + data[1][vhi];
        data[2][vlo] = temp;
        data[2][vhi] = temp;
    }
    
    /* Pattern B: count = 8 */
    {
        const int lo_b = 5;
        const int hi_b = 12;
        volatile int vlo = lo_b;
        volatile int vhi = hi_b;
        
        for (int j = vlo; j <= vhi; ++j) {
            data[3][j] = data[0][j] + data[1][j];
        }
    }
    
    /* Pattern C: count = 1 */
    {
        const int lo_c = 14;
        volatile int vlo = lo_c;
        
        data[4][vlo] = data[0][vlo] * 2;
    }
    
    /* Consume all */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 15; j++) {
            use_int(data[i][j]);
        }
    }
}

int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    test_small_slice();
    checksum += 1;
    
    test_large_int_slice();
    checksum += 2;
    
    test_double_slice();
    checksum += 3;
    
    test_char_single();
    checksum += 4;
    
    test_vla_constant();
    checksum += 5;
    
    test_mixed_patterns();
    checksum += 6;
    
    /* Final volatile sink */
    volatile int final_sink = checksum;
    
    printf("Test completed with checksum: %d\n", checksum);
    return 0;
}

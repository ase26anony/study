/* test_expr_coverage.c */
#include <stdio.h>
#include <stdint.h>

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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
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
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice - lvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[3][j] = temp[j - actual_lo] * 2;
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Another small slice: count = 1 */
    const int lo2 = 8;
    const int hi2 = 8;  /* count = 1 */
    
    volatile int vlo2 = lo2;
    int actual_lo2 = vlo2;
    
    /* Mixed operations on single element */
    int val = arr[5][actual_lo2];  /* Load */
    use_int(val);
    arr[5][actual_lo2] = val + 1;  /* Store */
    use_int(arr[5][actual_lo2]);
}

/* Test 2: Larger slice with count > 2, int type */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
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
    
    /* Buffer for copying */
    int buffer[10];
    
    /* Load slice - rvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        buffer[j - actual_lo] = grid[25][j];
        use_int(grid[25][j]);
    }
    
    /* Store slice - lvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        grid[25][j] = buffer[j - actual_lo] + 1000;
        use_int(grid[25][j]);
    }
    
    /* Another slice with different row */
    for (int j = 30; j <= 39; ++j) {  /* count = 10 */
        grid[50][j] = grid[25][j] * 2;
        use_int(grid[50][j]);
    }
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Slice with count = 5 */
    const int lo = 5;
    const int hi = 9;  /* count = 5 */
    
    volatile int vlo = lo;
    int actual_lo = vlo;
    
    /* Both load and store operations */
    double sum = 0.0;
    
    /* Load - rvalue */
    for (int j = actual_lo; j <= hi; ++j) {
        sum += matrix[2][j];
        use_double(matrix[2][j]);
    }
    
    /* Store - lvalue */
    for (int j = actual_lo; j <= hi; ++j) {
        matrix[2][j] = matrix[2][j] * 1.1;
        use_double(matrix[2][j]);
    }
    
    use_double(sum);
}

/* Test 4: Char array with varying slice sizes */
static void __attribute__((noinline))
test_char_slice(void) {
    char data[5][100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 100; ++j) {
            data[i][j] = (i + j) & 0x7F;
        }
    }
    
    /* Test different slice sizes */
    
    /* count = 3 */
    for (int j = 20; j <= 22; ++j) {
        data[0][j] = data[1][j] + 1;
        use_int(data[0][j]);
    }
    
    /* count = 15 */
    char buffer[15];
    for (int j = 40; j <= 54; ++j) {
        buffer[j - 40] = data[2][j];
        use_int(data[2][j]);
    }
    
    /* Store back with offset */
    for (int j = 40; j <= 54; ++j) {
        data[3][j] = buffer[j - 40] ^ 0x55;
        use_int(data[3][j]);
    }
}

/* Test 5: VLA with constant size */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds on VLA slice */
    const int lo = 10;
    const int hi = 15;  /* count = 6 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Operations on VLA slice */
    int temp[6];
    
    /* Load from VLA - rvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        temp[j - actual_lo] = vla[5][j];
        use_int(vla[5][j]);
    }
    
    /* Store to VLA - lvalue */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        vla[5][j] = temp[j - actual_lo] * 3;
        use_int(vla[5][j]);
    }
    
    /* Another slice with count = 2 */
    for (int j = 20; j <= 21; ++j) {
        vla[10][j] = vla[5][j] / 2;
        use_int(vla[10][j]);
    }
}

/* Test 6: Mixed operations with pointer arithmetic */
static void __attribute__((noinline))
test_mixed_operations(void) {
    int arr3d[3][10][15];
    
    /* Initialize 3D array */
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 15; ++k) {
                arr3d[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Fixed slice in middle dimension */
    const int dim2_lo = 3;
    const int dim2_hi = 7;  /* count = 5 */
    
    volatile int vlo = dim2_lo;
    int actual_lo = vlo;
    
    /* Process slice */
    for (int j = actual_lo; j <= dim2_hi; ++j) {
        /* Both load and store in same loop */
        int old_val = arr3d[1][j][5];  /* Load */
        arr3d[1][j][5] = old_val + 42;  /* Store */
        arr3d[2][j][5] = arr3d[1][j][5] * 2;  /* Load from one, store to another */
        
        use_int(old_val);
        use_int(arr3d[1][j][5]);
        use_int(arr3d[2][j][5]);
    }
    
    /* Small slice in third dimension */
    for (int k = 10; k <= 11; ++k) {  /* count = 2 */
        arr3d[0][5][k] = arr3d[1][5][k] + arr3d[2][5][k];
        use_int(arr3d[0][5][k]);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_int();
    checksum += 2;
    
    test_double_slice();
    checksum += 3;
    
    test_char_slice();
    checksum += 4;
    
    test_vla_slice();
    checksum += 5;
    
    test_mixed_operations();
    checksum += 6;
    
    printf("Test completed. Checksum: %d\n", checksum);
    
    return 0;
}

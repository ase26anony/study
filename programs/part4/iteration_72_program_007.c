/* Test program for expr.cc lines 7691-7700 */
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

/* Test 1: Small slice (count <= 2) - should take first branch */
static void __attribute__((noinline))
test_small_slice(void) {
    int arr[10][20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice (rvalue) */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = arr[3][j];
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice (lvalue) */
    for (int j = lo; j <= hi; j++) {
        arr[3][j] = temp[j - lo] * 2;
    }
    
    /* Verify */
    for (int j = lo; j <= hi; j++) {
        use_int(arr[3][j]);
    }
}

/* Test 2: Medium slice (count > 2, small element size) */
static void __attribute__((noinline))
test_medium_slice_char(void) {
    char grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            grid[i][j] = (char)((i + j) & 0xFF);
        }
    }
    
    /* Constant bounds with count = 10 */
    volatile int start = 10;
    volatile int end = 19;
    int lo = start;
    int hi = end;
    
    /* Mixed operations */
    char buffer[10];
    
    /* Read slice */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = grid[42][j];
        use_int(grid[42][j]);
    }
    
    /* Write slice */
    for (int j = lo; j <= hi; j++) {
        grid[42][j] = buffer[j - lo] + 1;
    }
    
    /* Read back */
    for (int j = lo; j <= hi; j++) {
        use_int(grid[42][j]);
    }
}

/* Test 3: Large slice with double elements */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Constant bounds, count = 15 */
    volatile int start = 2;
    volatile int end = 16;
    int lo = start;
    int hi = end;
    
    /* Operations on slice */
    double temp[15];
    
    /* Load context */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = matrix[5][j];
        use_double(matrix[5][j]);
    }
    
    /* Store context */
    for (int j = lo; j <= hi; j++) {
        matrix[5][j] = temp[j - lo] * 2.0;
    }
    
    /* Mixed load/store in same loop */
    for (int j = lo; j <= hi; j++) {
        matrix[5][j] = matrix[5][j] + 1.0;
        use_double(matrix[5][j]);
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int arr3d[5][10][15];
    
    /* Initialize */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Single element access with constant indices */
    volatile int idx1 = 2;
    volatile int idx2 = 5;
    volatile int idx3 = 7;
    int i = idx1;
    int j = idx2;
    int k = idx3;
    
    /* Both contexts */
    int val = arr3d[i][j][k];  /* Load */
    use_int(val);
    
    arr3d[i][j][k] = val * 3;  /* Store */
    use_int(arr3d[i][j][k]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_slice(void) {
    /* VLA with compile-time constant size */
    volatile int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant slice bounds */
    volatile int start = 8;
    volatile int end = 17;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Slice operations */
    int buffer[10];
    
    /* Read column slice */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = vla[15][j];
        use_int(vla[15][j]);
    }
    
    /* Write to different row */
    for (int j = lo; j <= hi; j++) {
        vla[20][j] = buffer[j - lo] + 100;
    }
    
    /* Cross-check */
    for (int j = lo; j <= hi; j++) {
        use_int(vla[15][j]);
        use_int(vla[20][j]);
    }
}

/* Test 6: Mixed slice sizes in same function */
static void __attribute__((noinline))
test_mixed_slices(void) {
    struct mixed {
        int a;
        double b;
        char c[8];
    } data[50][40];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 40; j++) {
            data[i][j].a = i + j;
            data[i][j].b = i * 0.1 + j * 0.01;
            for (int k = 0; k < 8; k++) {
                data[i][j].c[k] = (char)((i + j + k) & 0xFF);
            }
        }
    }
    
    /* Test different slice sizes */
    
    /* Size 2 slice */
    {
        volatile int s1 = 10;
        volatile int e1 = 11;
        int lo1 = s1;
        int hi1 = e1;
        
        for (int j = lo1; j <= hi1; j++) {
            data[25][j].a += data[25][j].b;
            use_double(data[25][j].b);
        }
    }
    
    /* Size 8 slice */
    {
        volatile int s2 = 20;
        volatile int e2 = 27;
        int lo2 = s2;
        int hi2 = e2;
        
        int temp[8];
        for (int j = lo2; j <= hi2; j++) {
            temp[j - lo2] = data[30][j].a;
        }
        
        for (int j = lo2; j <= hi2; j++) {
            data[30][j].a = temp[j - lo2] * 2;
            use_int(data[30][j].a);
        }
    }
}

/* Test 7: Pointer arithmetic with constant offsets */
static void __attribute__((noinline))
test_pointer_slice(void) {
    int buffer[1000];
    
    /* Initialize */
    for (int i = 0; i < 1000; i++) {
        buffer[i] = i * 3;
    }
    
    /* Constant bounds through pointer */
    volatile int offset = 100;
    volatile int length = 25;  /* count = 25 */
    int start = offset;
    int end = offset + length - 1;
    
    int *slice = &buffer[start];
    
    /* Operations on slice */
    int temp[25];
    
    /* Copy out */
    for (int i = 0; i < length; i++) {
        temp[i] = slice[i];
        use_int(slice[i]);
    }
    
    /* Modify and copy back */
    for (int i = 0; i < length; i++) {
        slice[i] = temp[i] + 1;
    }
    
    /* Verify */
    for (int i = 0; i < length; i++) {
        use_int(slice[i]);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations...\n");
    
    /* Run all tests */
    test_small_slice();
    checksum += 1;
    
    test_medium_slice_char();
    checksum += 2;
    
    test_large_slice_double();
    checksum += 3;
    
    test_single_element();
    checksum += 4;
    
    test_vla_constant_slice();
    checksum += 5;
    
    test_mixed_slices();
    checksum += 6;
    
    test_pointer_slice();
    checksum += 7;
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

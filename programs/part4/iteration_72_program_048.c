/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stdint.h>

/* Dummy functions to prevent early optimization */
static void __attribute__((noinline, noipa)) use_int(int x) {
    volatile static int sink;
    sink = x;
}

static void __attribute__((noinline, noipa)) use_double(double x) {
    volatile static double sink;
    sink = x;
}

static void __attribute__((noinline, noipa)) use_ptr(void *p) {
    volatile static void *sink;
    sink = p;
}

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[10][20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time but made opaque */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 (hi - lo + 1 = 6 - 5 + 1 = 2) */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice - rvalue context */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = arr[3][j];  /* arr[3][5..6] */
    }
    
    /* Store to slice - lvalue context */
    for (int j = lo; j <= hi; j++) {
        arr[7][j] = temp[j - lo] * 2;  /* arr[7][5..6] */
    }
    
    /* Use results to prevent elimination */
    for (int j = lo; j <= hi; j++) {
        use_int(arr[3][j]);
        use_int(arr[7][j]);
    }
}

/* Test 2: Multi-dimensional int array with count > 2 */
static void __attribute__((noinline)) test_large_slice(void) {
    int grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            grid[i][j] = (i << 8) | j;
        }
    }
    
    /* Constant bounds for larger slice */
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 (19 - 10 + 1 = 10) */
    int lo = start;
    int hi = end;
    
    /* Buffer for copying */
    int buffer[10];
    
    /* Load slice - rvalue context */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = grid[25][j];  /* grid[25][10..19] */
    }
    
    /* Store slice - lvalue context */
    for (int j = lo; j <= hi; j++) {
        grid[50][j] = buffer[j - lo] + 1000;  /* grid[50][10..19] */
    }
    
    /* Verify */
    for (int j = lo; j <= hi; j++) {
        use_int(grid[25][j]);
        use_int(grid[50][j]);
    }
}

/* Test 3: Double array with different element size */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Medium slice */
    volatile int start = 8;
    volatile int end = 12;  /* count = 5 (12 - 8 + 1 = 5) */
    int lo = start;
    int hi = end;
    
    double temp[5];
    
    /* Mixed operations */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = matrix[2][j];  /* load */
    }
    
    for (int j = lo; j <= hi; j++) {
        matrix[5][j] = temp[j - lo] * 2.0;  /* store */
    }
    
    /* Use results */
    for (int j = lo; j <= hi; j++) {
        use_double(matrix[2][j]);
        use_double(matrix[5][j]);
    }
}

/* Test 4: Char array with single element (count = 1) */
static void __attribute__((noinline)) test_char_single(void) {
    char bytes[50][100];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 100; j++) {
            bytes[i][j] = (i + j) & 0xFF;
        }
    }
    
    /* Single element slice */
    volatile int start = 42;
    volatile int end = 42;  /* count = 1 */
    int lo = start;
    int hi = end;
    
    char val;
    
    /* Single element access */
    val = bytes[10][lo];      /* load */
    bytes[20][lo] = val + 1;  /* store */
    
    use_int(bytes[10][lo]);
    use_int(bytes[20][lo]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant(void) {
    /* Constant size but using variable to declare VLA */
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Slice operations on VLA */
    volatile int start = 5;
    volatile int end = 9;  /* count = 5 */
    int lo = start;
    int hi = end;
    
    int temp[5];
    
    /* Load from VLA slice */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = vla[10][j];
    }
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; j++) {
        vla[15][j] = temp[j - lo] * 3;
    }
    
    /* Use results */
    for (int j = lo; j <= hi; j++) {
        use_int(vla[10][j]);
        use_int(vla[15][j]);
    }
}

/* Test 6: Mixed operations with count = 2 in different context */
static void __attribute__((noinline)) test_mixed_ops(void) {
    struct point { int x, y; } pts[20][30];
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            pts[i][j].x = i;
            pts[i][j].y = j;
        }
    }
    
    /* Two-element slice */
    volatile int start = 15;
    volatile int end = 16;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Direct assignment pattern that might trigger MEM_P */
    for (int j = lo; j <= hi; j++) {
        /* Load operation */
        int x_val = pts[5][j].x;
        int y_val = pts[5][j].y;
        
        /* Store operation */
        pts[10][j].x = x_val * 2;
        pts[10][j].y = y_val * 2;
        
        use_int(pts[5][j].x);
        use_int(pts[10][j].x);
    }
}

/* Test 7: Array of pointers - different MEM_P behavior */
static void __attribute__((noinline)) test_pointer_array(void) {
    int data[100];
    int *arr[10][5];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            arr[i][j] = &data[(i * 5 + j) % 100];
        }
    }
    
    /* Slice of pointers */
    volatile int start = 1;
    volatile int end = 3;  /* count = 3 */
    int lo = start;
    int hi = end;
    
    /* Access through pointer slice */
    for (int j = lo; j <= hi; j++) {
        int val = *arr[3][j];      /* load through pointer */
        *arr[7][j] = val + 100;    /* store through pointer */
        
        use_ptr(arr[3][j]);
        use_ptr(arr[7][j]);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_slice();      /* count = 2 */
    test_large_slice();      /* count = 10 */
    test_double_slice();     /* count = 5, double type */
    test_char_single();      /* count = 1, char type */
    test_vla_constant();     /* VLA with constant size */
    test_mixed_ops();        /* count = 2, struct type */
    test_pointer_array();    /* pointer array */
    
    /* Simple checksum to ensure execution */
    checksum = 1;
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

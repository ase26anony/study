/* Test program to trigger constant bounds analysis for array slices in GCC expr.cc */
#include <stdio.h>
#include <stddef.h>

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

/* Test 1: Small slice (count <= 2) with int array */
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
    volatile int start = 5;  /* Forces middle-end analysis */
    volatile int end = 6;    /* count = 2 (hi - lo + 1 = 6 - 5 + 1 = 2) */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice: rvalue context */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice: lvalue context */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = temp[j - lo] * 2;
    }
    
    /* Verify */
    for (int j = lo; j <= hi; ++j) {
        use_int(arr[3][j]);
    }
}

/* Test 2: Medium slice (count > 2) with char array */
static void __attribute__((noinline))
test_medium_slice_char(void) {
    char grid[50][100];
    
    /* Initialize */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 100; ++j) {
            grid[i][j] = (char)((i + j) & 0xFF);
        }
    }
    
    /* Constant bounds: count = 10 */
    volatile int start = 20;
    volatile int end = 29;   /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Mixed operations */
    char buffer[10];
    
    /* Read slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = grid[25][j];
        use_int(grid[25][j]);
    }
    
    /* Modify and write back */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = buffer[j - lo] + 1;
    }
    
    /* Another read */
    for (int j = lo; j <= hi; ++j) {
        use_int(grid[25][j]);
    }
}

/* Test 3: Large slice with double array - triggers TYPE_SIZE calculation */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 25; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Constant bounds: count = 15 */
    volatile int start = 5;
    volatile int end = 19;   /* count = 15 */
    int lo = start;
    int hi = end;
    
    /* Operations that should trigger MEM_P check with count > 2 */
    double temp[15];
    
    /* Load from slice */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = matrix[7][j];
        use_double(matrix[7][j]);
    }
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = temp[j - lo] * 2.0;
    }
    
    /* Final use */
    for (int j = lo; j <= hi; ++j) {
        use_double(matrix[7][j]);
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int arr[30][40];
    
    /* Initialize */
    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 40; ++j) {
            arr[i][j] = i * 40 + j;
        }
    }
    
    /* Single element: count = 1 */
    volatile int idx = 17;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Both contexts */
    int val = arr[10][lo];  /* rvalue */
    use_int(val);
    
    arr[10][lo] = val * 3;  /* lvalue */
    use_int(arr[10][lo]);
}

/* Test 5: VLA with constant size */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds within VLA */
    volatile int start = 10;
    volatile int end = 14;   /* count = 5 */
    int lo = start;
    int hi = end;
    
    /* Slice operations */
    int temp[5];
    
    /* Read */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = vla[15][j];
        use_int(vla[15][j]);
    }
    
    /* Write */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = temp[j - lo] + 100;
    }
    
    /* Read again */
    for (int j = lo; j <= hi; ++j) {
        use_int(vla[15][j]);
    }
}

/* Test 6: Mixed operations in same loop */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x;
        int y;
        double z;
    } points[20][30];
    
    /* Initialize */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 30; ++j) {
            points[i][j].x = i;
            points[i][j].y = j;
            points[i][j].z = i * 0.5 + j * 0.25;
        }
    }
    
    /* Constant bounds: count = 3 */
    volatile int start = 8;
    volatile int end = 10;   /* count = 3 */
    int lo = start;
    int hi = end;
    
    /* Complex element type tests TYPE_SIZE calculation */
    struct Point temp[3];
    
    /* Load slice */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = points[5][j];
        use_int(points[5][j].x);
        use_int(points[5][j].y);
        use_double(points[5][j].z);
    }
    
    /* Store slice */
    for (int j = lo; j <= hi; ++j) {
        points[5][j] = temp[j - lo];
        points[5][j].x += 1;
        points[5][j].y += 2;
        points[5][j].z *= 1.1;
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_medium_slice_char();
    checksum += 2;
    
    test_large_slice_double();
    checksum += 3;
    
    test_single_element();
    checksum += 4;
    
    test_vla_constant_size();
    checksum += 5;
    
    test_mixed_operations();
    checksum += 6;
    
    /* Print checksum to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

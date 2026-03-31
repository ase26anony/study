/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
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

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline))
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
        use_int(arr[3][j]);  /* Force rvalue use */
    }
    use_int(sum);
}

/* Test 2: Multi-dimensional int array with medium slice (count > 2) */
static void __attribute__((noinline))
test_medium_slice_int(void) {
    int grid[100][50];
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i * 100 + j;
        }
    }
    
    /* Mixed lvalue/rvalue operations on slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store operation */
        grid[25][j] = grid[25][j] * 2 + 1;
        
        /* Complex rvalue use */
        int temp = grid[25][j] + grid[24][j];
        use_int(temp);
    }
    
    /* Another slice with different bounds */
    volatile int start2 = 40;
    volatile int end2 = 49;  /* count = 10 */
    int lo2 = start2;
    int hi2 = end2;
    
    for (int j = lo2; j <= hi2; ++j) {
        grid[50][j] = j * 3;
        use_int(grid[50][j]);
    }
}

/* Test 3: Double array with varying slice sizes */
static void __attribute__((noinline))
test_double_slices(void) {
    double matrix[10][20];
    volatile int bounds[][2] = {{0, 1}, {5, 14}, {15, 19}};
    
    for (int b = 0; b < 3; ++b) {
        volatile int start = bounds[b][0];
        volatile int end = bounds[b][1];
        int lo = start;
        int hi = end;
        
        /* Store to slice */
        for (int j = lo; j <= hi; ++j) {
            matrix[5][j] = (double)(j * 1.5);
        }
        
        /* Read from slice with computation */
        double acc = 0.0;
        for (int j = lo; j <= hi; ++j) {
            acc += matrix[5][j];
            use_double(matrix[5][j]);  /* Rvalue context */
        }
        use_double(acc);
    }
}

/* Test 4: Char array with single element slice (count = 1) */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[8][64];
    volatile int pos = 32;
    int idx = pos;
    
    /* Single element access - count = 1 */
    buffer[4][idx] = 'A';  /* Store */
    char c = buffer[4][idx];  /* Load */
    use_int((int)c);
    
    /* Two element slice */
    volatile int pos2 = 40;
    int idx2 = pos2;
    buffer[4][idx2] = buffer[4][idx];  /* Both lvalue and rvalue */
    use_int((int)buffer[4][idx2]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    const int n = 30;
    int vla[n][n];
    volatile int start = 10;
    volatile int end = 25;  /* count = 16 */
    int lo = start;
    int hi = end;
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Slice operations on VLA */
    for (int j = lo; j <= hi; ++j) {
        /* Store operation */
        vla[15][j] = vla[15][j] * 3;
        
        /* Rvalue use with pointer arithmetic */
        int *ptr = &vla[15][j];
        use_ptr(ptr);
        use_int(*ptr);
    }
    
    /* Another slice with small count */
    volatile int start2 = 5;
    volatile int end2 = 6;  /* count = 2 */
    int lo2 = start2;
    int hi2 = end2;
    
    for (int j = lo2; j <= hi2; ++j) {
        vla[20][j] = j * 100;
        use_int(vla[20][j]);
    }
}

/* Test 6: Mixed operations triggering MEM_P checks */
static void __attribute__((noinline))
test_mixed_mem_ops(void) {
    struct Point {
        int x, y;
    } points[5][10];
    
    volatile int start = 2;
    volatile int end = 7;  /* count = 6 */
    int lo = start;
    int hi = end;
    
    /* Store to struct slice */
    for (int j = lo; j <= hi; ++j) {
        points[2][j].x = j * 10;
        points[2][j].y = j * 20;
    }
    
    /* Read from struct slice */
    for (int j = lo; j <= hi; ++j) {
        int sum = points[2][j].x + points[2][j].y;
        use_int(sum);
        
        /* Pointer to element */
        struct Point *p = &points[2][j];
        use_ptr(p);
    }
}

/* Test 7: Array with compile-time constant bounds directly */
static void __attribute__((noinline))
test_compile_time_constants(void) {
    int table[8][16];
    
    /* Direct constant bounds - should be analyzable */
    for (int j = 3; j <= 4; ++j) {  /* count = 2 */
        table[4][j] = 100 + j;
        use_int(table[4][j]);
    }
    
    for (int j = 5; j <= 14; ++j) {  /* count = 10 */
        table[4][j] = table[4][j-1] * 2;
        use_int(table[4][j]);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_medium_slice_int();
    checksum += 2;
    
    test_double_slices();
    checksum += 3;
    
    test_char_slice();
    checksum += 4;
    
    test_vla_constant_bounds();
    checksum += 5;
    
    test_mixed_mem_ops();
    checksum += 6;
    
    test_compile_time_constants();
    checksum += 7;
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

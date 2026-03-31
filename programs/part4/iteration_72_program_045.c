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

/* Test 1: Small slice (count <= 2) with int array */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    for (int j = lo; j <= hi; ++j) {
        /* Store operation */
        arr[3][j] = j * 2;
        
        /* Load operation */
        int val = arr[3][j];
        use_int(val);
    }
    
    /* Another small slice: count = 1 */
    lo = 8;
    hi = 8;
    for (int j = lo; j <= hi; ++j) {
        arr[4][j] = j * 3;
        use_int(arr[4][j]);
    }
}

/* Test 2: Larger slice (count > 2) with char array */
static void __attribute__((noinline))
test_large_slice_char(void) {
    char grid[100][50];
    
    /* Constant bounds for larger slice */
    volatile int start = 10;
    volatile int end = 25;  /* count = 16 */
    int lo = start;
    int hi = end;
    
    /* Mixed operations on char slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store */
        grid[20][j] = (char)(j % 256);
        
        /* Load and use */
        char c = grid[20][j];
        use_int((int)c);
    }
    
    /* Copy between slices - both lvalue and rvalue */
    lo = 30;
    hi = 45;  /* count = 16 */
    for (int j = lo; j <= hi; ++j) {
        /* grid[21] as rvalue source, grid[22] as lvalue target */
        grid[22][j - lo] = grid[21][j];
    }
}

/* Test 3: Double array with varying element sizes */
static void __attribute__((noinline))
test_double_array(void) {
    double matrix[10][20];
    
    /* Medium slice */
    volatile int start = 2;
    volatile int end = 8;  /* count = 7 */
    int lo = start;
    int hi = end;
    
    /* Operations on double slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = j * 1.5;
        double d = matrix[5][j];
        use_double(d);
    }
    
    /* Another slice with different bounds */
    lo = 12;
    hi = 19;  /* count = 8 */
    for (int j = lo; j <= hi; ++j) {
        matrix[6][j] = matrix[5][j - 10];
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    const int n = 30;
    double vla[n][n];
    
    /* Constant bounds within VLA */
    volatile int start = 5;
    volatile int end = 15;  /* count = 11 */
    int lo = start;
    int hi = end;
    
    /* Slice operations on VLA */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = j * 2.0;
        use_double(vla[10][j]);
    }
    
    /* Small slice in VLA */
    lo = 25;
    hi = 26;  /* count = 2 */
    for (int j = lo; j <= hi; ++j) {
        vla[11][j] = vla[10][j - 20];
    }
}

/* Test 5: Multi-dimensional access with mixed operations */
static void __attribute__((noinline))
test_mixed_operations(void) {
    int arr3d[5][10][15];
    
    /* Test various slice sizes */
    struct {
        int lo, hi;
    } tests[] = {
        {0, 0},    /* count = 1 */
        {3, 4},    /* count = 2 */
        {5, 10},   /* count = 6 */
        {8, 14},   /* count = 7 */
    };
    
    for (size_t t = 0; t < sizeof(tests)/sizeof(tests[0]); ++t) {
        volatile int vlo = tests[t].lo;
        volatile int vhi = tests[t].hi;
        int lo = vlo;
        int hi = vhi;
        
        /* Fixed first two dimensions, slice the third */
        for (int k = lo; k <= hi; ++k) {
            /* Store operation */
            arr3d[2][4][k] = k * 10 + t;
            
            /* Load and use */
            int val = arr3d[2][4][k];
            use_int(val);
            
            /* Copy between slices */
            if (k > lo) {
                arr3d[2][5][k] = arr3d[2][4][k - 1];
            }
        }
    }
}

/* Test 6: Struct array to test different TYPE_SIZE */
struct Point {
    double x, y, z;
};

static void __attribute__((noinline))
test_struct_array(void) {
    struct Point points[20][10];
    
    /* Slice of structs */
    volatile int start = 2;
    volatile int end = 5;  /* count = 4 */
    int lo = start;
    int hi = end;
    
    for (int j = lo; j <= hi; ++j) {
        points[5][j].x = j * 1.0;
        points[5][j].y = j * 2.0;
        points[5][j].z = j * 3.0;
        
        /* Access struct member */
        double sum = points[5][j].x + points[5][j].y + points[5][j].z;
        use_double(sum);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Execute all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_char();
    checksum += 2;
    
    test_double_array();
    checksum += 3;
    
    test_vla_constant_bounds();
    checksum += 4;
    
    test_mixed_operations();
    checksum += 5;
    
    test_struct_array();
    checksum += 6;
    
    /* Use checksum to prevent optimization */
    printf("Test completed with checksum: %d\n", checksum);
    
    return 0;
}

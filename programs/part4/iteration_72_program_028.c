/* test_expr_coverage.c */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile static int sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile static void *sink;
    sink = ptr;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile static double sink;
    sink = val;
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
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* load */
        arr[3][j] = temp[j - lo] * 2;  /* store */
    }
}

/* Test 2: Multi-dimensional int array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
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
    
    /* Block copy between slices - both lvalue and rvalue contexts */
    for (int j = lo; j <= hi; ++j) {
        grid[20][j] = grid[10][j];  /* grid[10][j] is rvalue, grid[20][j] is lvalue */
    }
    
    /* Compute checksum */
    volatile int checksum = 0;
    for (int j = lo; j <= hi; ++j) {
        checksum += grid[20][j];
    }
    use_int(checksum);
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    volatile int start = 5;
    volatile int end = 14;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = j * 1.5;
    }
    
    /* Load from slice and process */
    volatile double acc = 0.0;
    for (int j = lo; j <= hi; ++j) {
        acc += matrix[5][j];
    }
    use_double(acc);
    
    /* Cross-slice copy with different element size */
    for (int j = lo; j <= hi; ++j) {
        matrix[6][j] = matrix[5][j] * 2.0;
    }
}

/* Test 4: Char array with varying slice sizes */
static void __attribute__((noinline))
test_char_slices(void) {
    char buffer[5][100];
    volatile int start1 = 10;
    volatile int end1 = 11;  /* count = 2 */
    volatile int start2 = 20;
    volatile int end2 = 29;  /* count = 10 */
    
    /* Small slice operations */
    for (int j = start1; j <= end1; ++j) {
        buffer[0][j] = 'A' + j;
        buffer[1][j] = buffer[0][j];  /* Both lvalue and rvalue */
    }
    
    /* Larger slice operations */
    for (int j = start2; j <= end2; ++j) {
        buffer[2][j] = '0' + (j % 10);
        buffer[3][j] = buffer[2][j] + 32;  /* lowercase */
    }
    
    /* Mixed: small slice as source, large as destination */
    for (int j = 0; j <= (end1 - start1); ++j) {
        buffer[4][start2 + j] = buffer[0][start1 + j];
    }
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_slice(void) {
    volatile int n = 30;  /* Constant through volatility */
    int size = n;
    
    /* VLA with constant size */
    int vla[size][size];
    
    volatile int start = 5;
    volatile int end = 14;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize VLA */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            vla[i][j] = i * size + j;
        }
    }
    
    /* Slice operations on VLA */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = vla[0][j] * 3;  /* vla[0][j] is rvalue, vla[10][j] is lvalue */
    }
    
    /* Compute checksum */
    volatile int vla_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        vla_sum += vla[10][j];
    }
    use_int(vla_sum);
}

/* Test 6: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int arr[10][10];
    volatile int idx = 5;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Single element access in both contexts */
    arr[3][lo] = 42;  /* store */
    volatile int val = arr[3][lo];  /* load */
    use_int(val);
    
    /* Chain of single-element copies */
    arr[4][lo] = arr[3][lo];
    arr[5][lo] = arr[4][lo];
}

/* Test 7: Struct array to test different TYPE_SIZE */
struct point {
    int x;
    int y;
    double z;
};

static void __attribute__((noinline))
test_struct_slice(void) {
    struct point points[10][20];
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Initialize */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            points[i][j].x = i;
            points[i][j].y = j;
            points[i][j].z = i * j * 0.5;
        }
    }
    
    /* Copy slice of structs */
    for (int j = lo; j <= hi; ++j) {
        points[5][j] = points[0][j];  /* Both lvalue and rvalue with struct type */
    }
    
    /* Access individual fields in slice */
    volatile double z_sum = 0.0;
    for (int j = lo; j <= hi; ++j) {
        z_sum += points[5][j].z;
    }
    use_double(z_sum);
}

int main(void) {
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all test functions */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_char_slices();
    test_vla_constant_slice();
    test_single_element();
    test_struct_slice();
    
    printf("All tests completed\n");
    
    /* Force materialization of results */
    volatile int dummy = 0;
    use_int(dummy);
    
    return 0;
}

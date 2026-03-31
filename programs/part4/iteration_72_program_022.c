/* Test program to trigger constant bounds checking for array slices in GCC expr.cc */
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
    int arr[10][20] = {0};
    
    /* Force constant bounds through volatile */
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
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] * 2;
    use_int(arr[3][lo]);
}

/* Test 2: Larger slice (count > 2) with char array */
static void __attribute__((noinline))
test_large_slice_char(void) {
    char grid[100][50];
    
    /* Constant bounds known at compile time */
    const int lo = 10;
    const int hi = 25;  /* count = 16 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        grid[42][j] = (char)(j % 256);
    }
    
    /* Copy slice to another location */
    char buffer[50];
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = grid[42][j];
    }
    
    /* Use results to prevent elimination */
    char checksum = 0;
    for (int j = 0; j < (hi - lo + 1); ++j) {
        checksum ^= buffer[j];
    }
    use_int(checksum);
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[15][25];
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = 8;
    volatile int vhi = 18;  /* count = 11 */
    int lo = vlo;
    int hi = vhi;
    
    /* Write to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Read from slice and compute */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += matrix[7][j];
    }
    use_double(total);
    
    /* Cross-slice copy */
    for (int j = lo; j <= hi; ++j) {
        matrix[8][j] = matrix[7][j];
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int data[30][40];
    
    const int idx = 15;
    
    /* Both store and load on single element */
    data[10][idx] = 999;
    int val = data[10][idx];
    use_int(val);
    
    /* Pointer arithmetic that might trigger MEM_P */
    int *ptr = &data[10][idx];
    *ptr = *ptr + 1;
    use_int(*ptr);
}

/* Test 5: VLA with constant size */
static void __attribute__((noinline))
test_vla_constant_slice(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Constant bounds within VLA */
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = j * 3;
    }
    
    /* Copy slice row */
    int row_copy[30];
    for (int j = lo; j <= hi; ++j) {
        row_copy[j] = vla[10][j];
    }
    
    /* Use the data */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += row_copy[j];
    }
    use_int(sum);
}

/* Test 6: Mixed operations to trigger different paths */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x;
        int y;
        double z;
    } points[20][30];
    
    /* Test with count = 3 */
    const int lo = 10;
    const int hi = 12;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        points[5][j].x = j;
        points[5][j].y = j * 2;
        points[5][j].z = j * 0.5;
    }
    
    /* Load from slice */
    struct Point temp[3];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = points[5][j];
    }
    
    /* Use pointer to slice */
    struct Point *slice_ptr = &points[5][lo];
    use_ptr(slice_ptr);
    
    /* Modify through pointer */
    for (int j = 0; j < (hi - lo + 1); ++j) {
        slice_ptr[j].x += 1;
    }
}

/* Test 7: Nested slice access */
static void __attribute__((noinline))
test_nested_slice(void) {
    int cube[5][10][15];
    
    /* Outer slice */
    const int lo1 = 2;
    const int hi1 = 4;  /* count = 3 */
    
    /* Inner slice */
    const int lo2 = 5;
    const int hi2 = 9;  /* count = 5 */
    
    /* 3D slice operations */
    for (int i = lo1; i <= hi1; ++i) {
        for (int j = lo2; j <= hi2; ++j) {
            cube[i][j][7] = i * 100 + j;
        }
    }
    
    /* Extract 2D slice */
    int slice[3][5];
    for (int i = lo1; i <= hi1; ++i) {
        for (int j = lo2; j <= hi2; ++j) {
            slice[i - lo1][j - lo2] = cube[i][j][7];
        }
    }
    
    /* Use slice data */
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 5; ++j) {
            total += slice[i][j];
        }
    }
    use_int(total);
}

int main(void) {
    printf("Starting array slice tests...\n");
    
    /* Run all tests to trigger different paths */
    test_small_slice_int();      /* count = 2 */
    test_large_slice_char();     /* count = 16 */
    test_double_slice();         /* count = 11 */
    test_single_element();       /* count = 1 */
    test_vla_constant_slice();   /* count = 10 */
    test_mixed_operations();     /* count = 3 */
    test_nested_slice();         /* nested slices */
    
    printf("Tests completed.\n");
    
    /* Force materialization of results */
    volatile int final_check = 0;
    final_check = 1;
    
    return final_check - 1;
}

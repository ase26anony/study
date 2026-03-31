/* test_expr_array_bounds.c */
#include <stdio.h>
#include <string.h>

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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Write to slice (lvalue context) */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Read from slice (rvalue context) */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* Load from slice */
        arr[3][j] = temp[j - lo] * 2;  /* Store to slice */
    }
}

/* Test 2: Larger slice with count > 2 */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int matrix[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds for larger slice */
    const int start = 10;
    const int end = 25;  /* count = 16 */
    
    /* Use volatile to force middle-end analysis */
    volatile int v_start = start;
    volatile int v_end = end;
    int lo = v_start;
    int hi = v_end;
    
    /* Process a row slice - both read and write */
    int buffer[50];
    
    /* Read slice into buffer */
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = matrix[42][j];
    }
    
    /* Modify and write back */
    for (int j = lo; j <= hi; ++j) {
        matrix[42][j] = buffer[j - lo] + 1000;
    }
    
    /* Verify with reads */
    volatile int check = 0;
    for (int j = lo; j <= hi; ++j) {
        check += matrix[42][j];
    }
    use_int(check);
}

/* Test 3: Different element type (double) */
static void __attribute__((noinline))
test_double_slice(void) {
    double grid[20][30];
    
    /* Initialize with pattern */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 30; ++j) {
            grid[i][j] = i * 1.5 + j * 0.7;
        }
    }
    
    /* Constant bounds - count = 3 */
    const int lo_idx = 15;
    const int hi_idx = 17;
    
    /* Process column slice */
    double col_slice[20];
    
    /* Read column slice */
    for (int i = 0; i < 20; ++i) {
        col_slice[i] = grid[i][lo_idx];
    }
    
    /* Write modified values to a different column slice */
    for (int i = 0; i < 20; ++i) {
        grid[i][hi_idx] = col_slice[i] * 2.0;
    }
    
    /* Mixed access with different count */
    volatile double v_lo = 5.0;
    volatile double v_hi = 14.0;
    int lo2 = (int)v_lo;  /* Actually 5 */
    int hi2 = (int)v_hi;  /* Actually 14, count = 10 */
    
    for (int j = lo2; j <= hi2; ++j) {
        grid[10][j] = grid[10][j] + 1.0;
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    char bytes[256][128];
    
    /* Initialize */
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 128; ++j) {
            bytes[i][j] = (i + j) & 0xFF;
        }
    }
    
    /* Single element access pattern */
    const int row = 100;
    const int col = 64;
    
    /* Both read and write to same element */
    char old = bytes[row][col];
    bytes[row][col] = old ^ 0x55;
    
    /* Access adjacent elements to create potential slice */
    bytes[row][col-1] = bytes[row][col+1];  /* count = 1 for each side */
    
    use_int((int)bytes[row][col]);
}

/* Test 5: VLA with constant size */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    const int n = 40;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice */
    const int slice_start = 10;
    const int slice_end = 19;  /* count = 10 */
    
    /* Process diagonal slice */
    for (int i = slice_start; i <= slice_end; ++i) {
        vla[i][i] = vla[i][i] * 3;
    }
    
    /* Process row slice with both bounds */
    int row = 25;
    for (int j = slice_start; j <= slice_end; ++j) {
        vla[row][j] = vla[row-1][j] + vla[row+1][j];
    }
    
    /* Check result */
    volatile int total = 0;
    for (int i = slice_start; i <= slice_end; ++i) {
        total += vla[i][i];
    }
    use_int(total);
}

/* Test 6: Mixed operations with different counts */
static void __attribute__((noinline))
test_mixed_counts(void) {
    struct Point {
        int x, y;
    } points[50][60];
    
    /* Initialize */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 60; ++j) {
            points[i][j].x = i;
            points[i][j].y = j;
        }
    }
    
    /* Test count = 2 */
    {
        const int lo = 30;
        const int hi = 31;
        
        for (int j = lo; j <= hi; ++j) {
            points[10][j].x = points[10][j].y;
            points[10][j].y = j * 2;
        }
    }
    
    /* Test count = 8 */
    {
        const int lo = 10;
        const int hi = 17;
        
        /* Copy slice */
        struct Point temp[8];
        for (int j = lo; j <= hi; ++j) {
            temp[j - lo] = points[20][j];
        }
        
        /* Modify and copy back */
        for (int j = lo; j <= hi; ++j) {
            points[20][j] = temp[j - lo];
            points[20][j].x += 100;
        }
    }
}

int main(void) {
    printf("Testing array slice bounds analysis...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_single_element();
    test_vla_constant_size();
    test_mixed_counts();
    
    printf("All tests completed.\n");
    
    /* Create a checksum to prevent optimization */
    volatile int checksum = 0;
    int arr[10] = {0};
    for (int i = 0; i < 10; ++i) {
        arr[i] = i * 2;
        checksum += arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

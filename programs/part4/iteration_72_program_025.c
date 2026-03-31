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

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    volatile int start = 5;  /* Forces middle-end analysis */
    volatile int end = 6;    /* hi - lo + 1 = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - reading from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    use_int(sum);
    
    /* Another small slice: count = 1 */
    volatile int single = 8;
    int idx = single;
    arr[5][idx] = 100;
    use_int(arr[5][idx]);
}

/* Test 2: Multi-dimensional double array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[15][25] = {0.0};
    
    /* Constant bounds for larger slice */
    volatile int start = 3;
    volatile int end = 12;   /* hi - lo + 1 = 10 */
    int lo = start;
    int hi = end;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Load from slice */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += matrix[7][j];
        use_double(matrix[7][j]);
    }
    use_double(total);
    
    /* Mixed access pattern */
    for (int j = lo; j <= lo + 4; ++j) {  /* 5 elements */
        matrix[10][j] = matrix[7][j] * 2.0;
    }
}

/* Test 3: Char array with varying element sizes */
static void __attribute__((noinline))
test_char_array(void) {
    char buffer[50][100] = {0};
    
    /* Different slice sizes */
    volatile int bounds[][2] = {{10, 11}, {20, 29}, {40, 40}};
    
    for (int i = 0; i < 3; ++i) {
        int lo = bounds[i][0];
        int hi = bounds[i][1];
        
        /* Store pattern */
        for (int j = lo; j <= hi; ++j) {
            buffer[25][j] = (i + j) & 0xFF;
        }
        
        /* Load pattern */
        char checksum = 0;
        for (int j = lo; j <= hi; ++j) {
            checksum ^= buffer[25][j];
        }
        use_int(checksum);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Slice operations on VLA */
    volatile int slice_start = 5;
    volatile int slice_end = 14;  /* 10 elements */
    int lo = slice_start;
    int hi = slice_end;
    
    /* Row slice access */
    int row = 15;
    for (int j = lo; j <= hi; ++j) {
        vla[row][j] = vla[row][j] * 2 + 1;
    }
    
    /* Column slice access (non-contiguous in memory) */
    int col = 20;
    int col_sum = 0;
    for (int i = lo; i <= hi; ++i) {
        col_sum += vla[i][col];
        vla[i][col] = col_sum;
    }
    use_int(col_sum);
    
    /* Small VLA slice */
    volatile int small_end = lo + 1;  /* 2 elements */
    int small_hi = small_end;
    for (int j = lo; j <= small_hi; ++j) {
        vla[0][j] = -1;
        use_int(vla[0][j]);
    }
}

/* Test 5: Mixed operations to trigger MEM_P checks */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x, y;
    } grid[8][8];
    
    /* Initialize */
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            grid[i][j].x = i;
            grid[i][j].y = j;
        }
    }
    
    /* Various slice sizes with struct elements */
    volatile int ranges[][3] = {
        {2, 3, 0},  /* 2 elements, row 0 */
        {1, 4, 2},  /* 4 elements, row 2 */
        {5, 5, 5},  /* 1 element, row 5 */
        {0, 7, 7}   /* 8 elements, row 7 */
    };
    
    for (int r = 0; r < 4; ++r) {
        int lo = ranges[r][0];
        int hi = ranges[r][1];
        int row = ranges[r][2];
        
        /* Modify slice */
        for (int j = lo; j <= hi; ++j) {
            grid[row][j].x += j;
            grid[row][j].y -= j;
        }
        
        /* Read slice */
        int x_sum = 0, y_sum = 0;
        for (int j = lo; j <= hi; ++j) {
            x_sum += grid[row][j].x;
            y_sum += grid[row][j].y;
        }
        use_int(x_sum);
        use_int(y_sum);
    }
}

/* Test 6: Pointer-based slice operations */
static void __attribute__((noinline))
test_pointer_slices(void) {
    int data[100];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        data[i] = i * 3;
    }
    
    /* Constant-bound pointer arithmetic */
    volatile int p_start = 25;
    volatile int p_end = 34;  /* 10 elements */
    int *lo_ptr = &data[p_start];
    int *hi_ptr = &data[p_end];
    
    /* Process slice via pointers */
    int *p = lo_ptr;
    while (p <= hi_ptr) {
        *p = (*p) * 2 - 1;
        p++;
    }
    
    /* Verify */
    int verify = 0;
    for (int *p = lo_ptr; p <= hi_ptr; p++) {
        verify += *p;
        use_int(*p);
    }
    use_int(verify);
    
    /* Small pointer slice */
    volatile int small_p_end = p_start + 1;
    int *small_hi = &data[small_p_end];
    for (int *p = lo_ptr; p <= small_hi; p++) {
        *p = 0xABCD;
    }
}

int main(void) {
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_double();
    test_char_array();
    test_vla_constant();
    test_mixed_operations();
    test_pointer_slices();
    
    /* Final checksum */
    volatile int final_check = 42;
    printf("Test completed with check value: %d\n", final_check);
    
    return 0;
}

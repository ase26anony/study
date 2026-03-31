/* Test program for expr.cc lines 7691-7700 */
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

/* Test 1: 2D int array with count <= 2 */
static void __attribute__((noinline))
test_small_slice(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    volatile int vlo = lo;  /* Force middle-end analysis */
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[3][j] = j * 10;  /* Write to slice */
    }
    
    /* Load context (rvalue) */
    int sum = 0;
    for (int j = actual_lo; j <= actual_hi; ++j) {
        sum += arr[3][j];    /* Read from slice */
    }
    use_int(sum);
    
    /* Another pattern with count = 1 */
    const int single_lo = 8;
    const int single_hi = 8;  /* count = 1 */
    volatile int vsingle = single_lo;
    
    /* Mixed store/load in same expression */
    arr[4][vsingle] = arr[3][vsingle] * 2;
    use_int(arr[4][vsingle]);
}

/* Test 2: Larger slice with count > 2, small element size (char) */
static void __attribute__((noinline))
test_char_slice(void) {
    char grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = (i + j) % 256;
        }
    }
    
    /* Constant bounds for larger slice */
    const int lo = 10;
    const int hi = 25;  /* count = 16 */
    
    volatile int start = lo;
    volatile int end = hi;
    int slice_lo = start;
    int slice_hi = end;
    
    /* Store to slice */
    for (int j = slice_lo; j <= slice_hi; ++j) {
        grid[20][j] = 'A' + (j % 26);
    }
    
    /* Load from slice with computation */
    char buffer[50];
    for (int j = slice_lo; j <= slice_hi; ++j) {
        buffer[j - slice_lo] = grid[20][j];
    }
    use_ptr(buffer);
    
    /* Another access with different row */
    int checksum = 0;
    for (int j = slice_lo; j <= slice_hi; ++j) {
        checksum += grid[21][j];
    }
    use_int(checksum);
}

/* Test 3: Double array with count > 2, larger element size */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Different constant bounds */
    const int bounds[][2] = {{5, 7}, {10, 19}};  /* counts: 3 and 10 */
    
    for (int b = 0; b < 2; ++b) {
        volatile int vlo = bounds[b][0];
        volatile int vhi = bounds[b][1];
        int lo_idx = vlo;
        int hi_idx = vhi;
        
        /* Store operation */
        double val = 3.14159;
        for (int j = lo_idx; j <= hi_idx; ++j) {
            matrix[2][j] = val + j;
        }
        
        /* Load operation with aggregation */
        double total = 0.0;
        for (int j = lo_idx; j <= hi_idx; ++j) {
            total += matrix[2][j];
        }
        use_double(total);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];     /* VLA with constant dimension */
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds for slice */
    const int lo = 5;
    const int hi = 15;  /* count = 11 */
    
    volatile int vstart = lo;
    volatile int vend = hi;
    int slice_start = vstart;
    int slice_end = vend;
    
    /* Store to VLA slice */
    for (int j = slice_start; j <= slice_end; ++j) {
        vla[10][j] = j * 100;
    }
    
    /* Load from VLA slice */
    int extract[50];
    for (int j = slice_start; j <= slice_end; ++j) {
        extract[j - slice_start] = vla[10][j];
    }
    
    /* Use result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < (slice_end - slice_start + 1); ++i) {
        sum += extract[i];
    }
    use_int(sum);
}

/* Test 5: Mixed operations in single expression */
static void __attribute__((noinline))
test_mixed_operations(void) {
    int data[5][10];
    
    /* Initialize */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            data[i][j] = i * 10 + j;
        }
    }
    
    /* Multiple constant-bound slices */
    const struct {
        int row;
        int lo;
        int hi;
    } slices[] = {
        {1, 2, 3},   /* count = 2 */
        {2, 4, 9},   /* count = 6 */
        {3, 0, 0},   /* count = 1 */
    };
    
    for (int s = 0; s < 3; ++s) {
        volatile int vrow = slices[s].row;
        volatile int vlo = slices[s].lo;
        volatile int vhi = slices[s].hi;
        
        int row = vrow;
        int lo_idx = vlo;
        int hi_idx = vhi;
        
        /* Copy between slices (both store and load contexts) */
        for (int j = lo_idx; j <= hi_idx; ++j) {
            data[row+1][j] = data[row][j] * 2;  /* Mixed lvalue/rvalue */
        }
        
        /* Verify copy */
        int check = 0;
        for (int j = lo_idx; j <= hi_idx; ++j) {
            check += data[row+1][j] - data[row][j] * 2;
        }
        use_int(check);
    }
}

int main(void) {
    printf("Testing array slice operations...\n");
    
    /* Run all tests */
    test_small_slice();
    test_char_slice();
    test_double_slice();
    test_vla_slice();
    test_mixed_operations();
    
    printf("All tests completed.\n");
    
    /* Final volatile sink */
    volatile int final_sink = 42;
    return final_sink - 42;
}

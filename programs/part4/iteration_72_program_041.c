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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;  /* Write to slice */
    }
    
    /* Load context (rvalue) - MEM_P(target) should be true */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];  /* Read from slice */
    }
    use_int(sum);
    
    /* Mixed access pattern */
    for (int i = 0; i < 10; ++i) {
        /* Small slice access within larger loop */
        arr[i][lo] = i;
        arr[i][hi] = i * 2;
        use_int(arr[i][lo] + arr[i][hi]);
    }
}

/* Test 2: Larger slice with int array (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int matrix[50][100] = {0};
    
    /* Constant bounds for larger slice */
    const int start = 10;
    const int end = 25;  /* count = 16 */
    
    /* Write to large slice */
    for (int col = start; col <= end; ++col) {
        matrix[20][col] = col * 100;
    }
    
    /* Read from large slice */
    volatile int total = 0;
    for (int col = start; col <= end; ++col) {
        total += matrix[20][col];
    }
    use_int(total);
    
    /* Nested slice operations */
    for (int row = 0; row < 5; ++row) {
        /* Medium slice within outer loop */
        for (int col = start + row; col <= end - row; ++col) {
            matrix[row][col] = row * col;
            use_int(matrix[row][col]);
        }
    }
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline))
test_double_slice(void) {
    double grid[15][30] = {0.0};
    
    /* Different slice sizes */
    const int small_lo = 3, small_hi = 4;  /* count = 2 */
    const int large_lo = 5, large_hi = 20; /* count = 16 */
    
    /* Small slice operations */
    for (int j = small_lo; j <= small_hi; ++j) {
        grid[5][j] = j * 1.5;
        use_double(grid[5][j]);
    }
    
    /* Large slice operations */
    volatile double accum = 0.0;
    for (int j = large_lo; j <= large_hi; ++j) {
        grid[10][j] = j * 2.5;
        accum += grid[10][j];
    }
    use_double(accum);
    
    /* Mixed rvalue/lvalue in same expression */
    for (int j = 0; j < 5; ++j) {
        /* This creates complex array references */
        grid[j][small_lo] = grid[j][large_lo] * 2.0;
        use_double(grid[j][small_lo]);
    }
}

/* Test 4: Char array with different TYPE_SIZE */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[100][50] = {0};
    
    /* Char has small TYPE_SIZE, affecting the byte count calculation */
    const int c_lo = 10, c_hi = 35;  /* count = 26, but byte size small */
    
    /* Initialize slice */
    for (int j = c_lo; j <= c_hi; ++j) {
        buffer[25][j] = (j % 26) + 'A';
    }
    
    /* Copy slice to another location */
    for (int j = c_lo; j <= c_hi; ++j) {
        buffer[26][j] = buffer[25][j];  /* Both lvalue and rvalue */
    }
    
    /* Verify copy */
    volatile char check = 0;
    for (int j = c_lo; j <= c_hi; ++j) {
        check ^= buffer[25][j] ^ buffer[26][j];
    }
    use_int((int)check);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    /* VLA with compile-time constant size */
    const int n = 40;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice access */
    const int vla_lo = 15, vla_hi = 25;  /* count = 11 */
    
    /* Process slice */
    volatile int vla_sum = 0;
    for (int row = 0; row < 10; ++row) {
        for (int col = vla_lo; col <= vla_hi; ++col) {
            vla[row][col] *= 2;          /* lvalue */
            vla_sum += vla[row][col];    /* rvalue */
        }
    }
    use_int(vla_sum);
    
    /* Small slice within VLA */
    const int tiny_lo = 5, tiny_hi = 6;  /* count = 2 */
    for (int col = tiny_lo; col <= tiny_hi; ++col) {
        vla[30][col] = -1;
        use_int(vla[30][col]);
    }
}

/* Test 6: Complex index expressions that simplify to constants */
static void __attribute__((noinline))
test_complex_indices(void) {
    int arr3d[5][10][15] = {0};
    
    /* Volatile to force middle-end analysis */
    volatile int base = 3;
    int lo_idx = base + 2;  /* = 5 */
    int hi_idx = base * 2;  /* = 6 */
    /* lo_idx = 5, hi_idx = 6, count = 2 */
    
    /* These should be recognized as constant bounds after optimization */
    for (int k = lo_idx; k <= hi_idx; ++k) {
        arr3d[2][4][k] = k * 100;
        use_int(arr3d[2][4][k]);
    }
    
    /* Another pattern with arithmetic */
    const int offset = 7;
    int slice_lo = offset - 2;  /* = 5 */
    int slice_hi = offset - 1;  /* = 6 */
    
    for (int plane = 0; plane < 3; ++plane) {
        for (int k = slice_lo; k <= slice_hi; ++k) {
            arr3d[plane][k][1] = plane + k;
            use_int(arr3d[plane][k][1]);
        }
    }
}

/* Test 7: Very large count to test TYPE_SIZE * count calculation */
static void __attribute__((noinline))
test_large_count(void) {
    short big[1000][100] = {0};  /* short: TYPE_SIZE = 16 bits */
    
    /* Large slice but small element size */
    const int big_lo = 0, big_hi = 99;  /* count = 100 */
    
    /* Initialize large slice */
    for (int j = big_lo; j <= big_hi; ++j) {
        big[500][j] = j * 2;
    }
    
    /* Process in chunks */
    volatile long big_total = 0;
    for (int chunk = 0; chunk < 10; ++chunk) {
        int chunk_lo = big_lo + chunk * 10;
        int chunk_hi = chunk_lo + 9;  /* count = 10 for each chunk */
        
        for (int j = chunk_lo; j <= chunk_hi; ++j) {
            big[500][j] += 1;
            big_total += big[500][j];
        }
    }
    use_int((int)(big_total & 0xFFFFFFFF));
}

int main(void) {
    printf("Starting array slice tests...\n");
    
    /* Run all test functions */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_char_slice();
    test_vla_slice();
    test_complex_indices();
    test_large_count();
    
    printf("All tests completed.\n");
    
    /* Final volatile sink to prevent optimization */
    volatile int final_check = 0;
    for (int i = 0; i < 100; ++i) {
        final_check += i;
    }
    printf("Checksum: %d\n", final_check);
    
    return 0;
}

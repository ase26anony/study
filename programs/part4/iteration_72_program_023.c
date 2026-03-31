/* expr_coverage.c - Targeting GCC expr.cc lines 7691-7700 */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) use_int(int val) {
    volatile int sink = val;
    return sink;
}

static double __attribute__((noinline, noipa)) use_double(double val) {
    volatile double sink = val;
    return sink;
}

static void __attribute__((noinline, noipa)) use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline, noipa)) 
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
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 1000;
    }
    
    /* Load context (rvalue) - reading from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[7][lo] = arr[3][hi];  /* Both lvalue and rvalue */
    use_int(arr[7][lo]);
}

/* Test 2: Multi-dimensional double array with medium slice (count > 2) */
static void __attribute__((noinline, noipa)) 
test_medium_slice_double(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 25; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds: count = 10 (15 - 6 + 1 = 10) */
    volatile int low_idx = 6;
    volatile int high_idx = 15;
    int lo = low_idx;
    int hi = high_idx;
    
    /* Store operation on slice */
    for (int i = lo; i <= hi; ++i) {
        matrix[8][i] = i * 2.5;
    }
    
    /* Load operation from slice */
    double total = 0.0;
    for (int i = lo; i <= hi; ++i) {
        total += matrix[8][i];
    }
    use_double(total);
    
    /* Another slice with different elttype size implications */
    for (int i = lo; i <= hi; i += 2) {
        matrix[12][i] = matrix[8][i] * 2.0;
    }
}

/* Test 3: Char array with varying slice sizes */
static void __attribute__((noinline, noipa)) 
test_char_slices(void) {
    char buffer[100][50];
    
    /* Fill with pattern */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            buffer[i][j] = (i + j) % 256;
        }
    }
    
    /* Test count = 1 (single element) */
    volatile int idx1 = 10;
    volatile int idx2 = 10;  /* hi = lo, count = 1 */
    int lo1 = idx1;
    int hi1 = idx2;
    
    /* Single element access - both read and write */
    buffer[5][lo1] = buffer[3][hi1] + 1;
    use_int(buffer[5][lo1]);
    
    /* Test count = 5 */
    volatile int start3 = 20;
    volatile int end3 = 24;  /* count = 5 */
    int lo3 = start3;
    int hi3 = end3;
    
    /* Block copy within same row */
    for (int j = lo3; j <= hi3; ++j) {
        buffer[7][j] = buffer[9][j];
    }
    
    /* Cross-row slice access */
    char temp[5];
    for (int j = 0; j <= (hi3 - lo3); ++j) {
        temp[j] = buffer[7][lo3 + j];
    }
    use_ptr(temp);
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline, noipa)) 
test_vla_constant_bounds(void) {
    /* VLA with compile-time constant size */
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds for slice */
    volatile int vla_start = 8;
    volatile int vla_end = 18;  /* count = 11 */
    int lo = vla_start;
    int hi = vla_end;
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = j * 100;
    }
    
    /* Load from VLA slice */
    int vla_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        vla_sum += vla[10][j];
    }
    use_int(vla_sum);
    
    /* Mixed slice operations */
    for (int j = lo; j <= hi; j += 3) {
        vla[15][j] = vla[10][j] * 2;
    }
}

/* Test 5: Struct array to test different TYPE_SIZE */
struct MixedData {
    char c;
    int i;
    double d;
};

static void __attribute__((noinline, noipa)) 
test_struct_slice(void) {
    struct MixedData grid[20][10];
    
    /* Initialize */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 10; ++j) {
            grid[i][j].c = 'A' + (i + j) % 26;
            grid[i][j].i = i * 100 + j;
            grid[i][j].d = i * 1.1 + j * 0.1;
        }
    }
    
    /* Slice with count = 3 */
    volatile int s_start = 2;
    volatile int s_end = 4;  /* count = 3 */
    int lo = s_start;
    int hi = s_end;
    
    /* Access struct slice */
    for (int j = lo; j <= hi; ++j) {
        grid[5][j].i = grid[3][j].i * 2;
        grid[5][j].d = grid[3][j].d + 1.0;
    }
    
    /* Read slice back */
    int struct_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        struct_sum += grid[5][j].i;
    }
    use_int(struct_sum);
}

/* Test 6: Edge case - exactly count = 2 with different element types */
static void __attribute__((noinline, noipa)) 
test_edge_cases(void) {
    long long big[5][40];
    
    /* Initialize */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 40; ++j) {
            big[i][j] = (long long)i * 1000 + j;
        }
    }
    
    /* Exactly count = 2 */
    volatile int e1 = 15;
    volatile int e2 = 16;  /* count = 2 */
    int lo = e1;
    int hi = e2;
    
    /* Two-element slice operations */
    big[2][lo] = big[1][hi];
    big[2][hi] = big[1][lo];
    
    /* Force use of results */
    use_int((int)(big[2][lo] + big[2][hi]));
    
    /* Another slice with count = 4 */
    volatile int e3 = 20;
    volatile int e4 = 23;  /* count = 4 */
    int lo2 = e3;
    int hi2 = e4;
    
    for (int j = lo2; j <= hi2; ++j) {
        big[3][j] = big[2][j] * 2;
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all test functions */
    test_small_slice_int();
    checksum += 1;
    
    test_medium_slice_double();
    checksum += 2;
    
    test_char_slices();
    checksum += 3;
    
    test_vla_constant_bounds();
    checksum += 4;
    
    test_struct_slice();
    checksum += 5;
    
    test_edge_cases();
    checksum += 6;
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

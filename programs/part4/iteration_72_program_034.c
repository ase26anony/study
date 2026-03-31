/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <string.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) use_int(int x) {
    volatile int sink = x;
    return sink;
}

static void __attribute__((noinline, noipa)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
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
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] + 100;
    use_ptr(&arr[0][0]);
}

/* Test 2: Multi-dimensional double array with medium slice (count > 2) */
static void __attribute__((noinline)) test_medium_slice(void) {
    double matrix[15][25];
    
    /* Constant bounds that should be deducible */
    const int c_low = 8;
    const int c_high = 18;  /* count = 11 */
    volatile int v_low = c_low;
    volatile int v_high = c_high;
    int lo = v_low;
    int hi = v_high;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Copy slice to another location */
    double buffer[50];
    for (int j = 0; j <= (hi - lo); ++j) {
        buffer[j] = matrix[7][lo + j];
    }
    
    /* Use results to prevent elimination */
    volatile double check = matrix[7][lo] + matrix[7][hi];
    (void)check;
}

/* Test 3: Char array with varying element size considerations */
static void __attribute__((noinline)) test_char_array(void) {
    char grid[100][50];
    
    /* Different count values to test different paths */
    const struct {
        int lo;
        int hi;
    } tests[] = {
        {10, 10},  /* count = 1 */
        {20, 21},  /* count = 2 */
        {30, 39},  /* count = 10 */
    };
    
    for (size_t t = 0; t < sizeof(tests)/sizeof(tests[0]); ++t) {
        volatile int v_lo = tests[t].lo;
        volatile int v_hi = tests[t].hi;
        int lo = v_lo;
        int hi = v_hi;
        
        /* Both store and load operations */
        for (int j = lo; j <= hi; ++j) {
            grid[50][j] = 'A' + (j % 26);
        }
        
        char temp = 0;
        for (int j = lo; j <= hi; ++j) {
            temp ^= grid[50][j];
        }
        use_int(temp);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Constant bounds within VLA */
    volatile int start = 5;
    volatile int end = 15;  /* count = 11 */
    int lo = start;
    int hi = end;
    
    /* Initialize a diagonal slice */
    for (int i = lo; i <= hi; ++i) {
        vla[i][i] = i * i;
    }
    
    /* Copy slice to linear array */
    int linear[50];
    for (int i = 0; i <= (hi - lo); ++i) {
        linear[i] = vla[lo + i][lo + i];
    }
    
    /* Use to prevent elimination */
    volatile int check = linear[0] + linear[hi - lo];
    (void)check;
}

/* Test 5: Mixed types and access patterns */
static void __attribute__((noinline)) test_mixed_patterns(void) {
    struct mixed {
        int a;
        double b;
        char c[4];
    } data[20][15];
    
    /* Test different slice sizes */
    const int bounds[][2] = {
        {2, 3},   /* count = 2 */
        {4, 13},  /* count = 10 */
    };
    
    for (int b = 0; b < 2; ++b) {
        volatile int v_lo = bounds[b][0];
        volatile int v_hi = bounds[b][1];
        int lo = v_lo;
        int hi = v_hi;
        
        /* Store to slice */
        for (int j = lo; j <= hi; ++j) {
            data[10][j].a = j * 100;
            data[10][j].b = j * 0.5;
            data[10][j].c[0] = 'X';
        }
        
        /* Load from slice with computation */
        int total_a = 0;
        double total_b = 0.0;
        for (int j = lo; j <= hi; ++j) {
            total_a += data[10][j].a;
            total_b += data[10][j].b;
        }
        
        use_int(total_a);
        volatile double sink_b = total_b;
        (void)sink_b;
    }
}

/* Test 6: Three-dimensional array with constant plane slice */
static void __attribute__((noinline)) test_3d_slice(void) {
    int cube[5][10][15];
    
    /* Constant bounds in multiple dimensions */
    const int z_low = 3;
    const int z_high = 7;  /* count = 5 */
    volatile int v_zlow = z_low;
    volatile int v_zhigh = z_high;
    int lo_z = v_zlow;
    int hi_z = v_zhigh;
    
    /* Access a planar slice (constant x, varying y, varying z) */
    const int fixed_x = 2;
    
    /* Store to the slice */
    for (int z = lo_z; z <= hi_z; ++z) {
        for (int y = 0; y < 10; ++y) {
            cube[fixed_x][y][z] = y * 100 + z;
        }
    }
    
    /* Load from the slice */
    int slice_sum = 0;
    for (int z = lo_z; z <= hi_z; ++z) {
        for (int y = 0; y < 10; ++y) {
            slice_sum += cube[fixed_x][y][z];
        }
    }
    
    use_int(slice_sum);
}

int main(void) {
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_slice();
    test_medium_slice();
    test_char_array();
    test_vla_constant();
    test_mixed_patterns();
    test_3d_slice();
    
    printf("All tests completed.\n");
    
    /* Final checksum to ensure execution */
    volatile int final_check = 0;
    for (int i = 0; i < 10; i++) {
        final_check ^= i;
    }
    printf("Checksum: %d\n", final_check);
    
    return 0;
}

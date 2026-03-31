/* Test program for expr.cc lines 7691-7700 - constant bounds array slice operations */

#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static __attribute__((noinline, noipa)) void use_int(int x) {
    volatile static int sink;
    sink = x;
}

static __attribute__((noinline, noipa)) void use_double(double x) {
    volatile static double sink;
    sink = x;
}

static __attribute__((noinline, noipa)) void use_ptr(void *p) {
    volatile static void *sink;
    sink = p;
}

/* Test 1: 2D int array with small slice (count <= 2) */
static __attribute__((noinline)) void test_small_slice(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice - rvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        temp[j - actual_lo] = arr[3][j];
        use_int(arr[3][j]);
    }
    
    /* Store to slice - lvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[3][j] = temp[j - actual_lo] * 2;
    }
    
    /* Verify */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        use_int(arr[3][j]);
    }
}

/* Test 2: 2D double array with medium slice (count > 2) */
static __attribute__((noinline)) void test_medium_slice(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 25; j++) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds - count = 10 */
    const int start = 8;
    const int end = 17;
    
    volatile int vstart = start;
    volatile int vend = end;
    int actual_start = vstart;
    int actual_end = vend;
    
    double buffer[10];
    
    /* Load from slice - rvalue */
    for (int j = actual_start; j <= actual_end; ++j) {
        buffer[j - actual_start] = matrix[7][j];
        use_double(matrix[7][j]);
    }
    
    /* Store to slice - lvalue */
    for (int j = actual_start; j <= actual_end; ++j) {
        matrix[7][j] = buffer[j - actual_start] * 1.1;
    }
    
    /* Mixed access pattern */
    for (int j = actual_start; j <= actual_end; j += 2) {
        matrix[7][j] = matrix[7][j + 1] + 1.0;  /* Load and store in same stmt */
    }
}

/* Test 3: 3D char array with varying element sizes */
static __attribute__((noinline)) void test_char_array(void) {
    char cube[5][10][15];
    
    /* Initialize */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                cube[i][j][k] = (i + j + k) & 0xFF;
            }
        }
    }
    
    /* Test single element (count = 1) */
    {
        const int lo = 7;
        const int hi = 7;
        volatile int v = lo;
        int idx = v;
        
        /* Both contexts */
        char val = cube[2][3][idx];  /* Load */
        use_int(val);
        cube[2][3][idx] = val + 1;   /* Store */
    }
    
    /* Test 4 elements (count = 4) */
    {
        const int lo = 2;
        const int hi = 5;
        volatile int vlo = lo;
        volatile int vhi = hi;
        int actual_lo = vlo;
        int actual_hi = vhi;
        
        char temp[4];
        
        /* Load slice */
        for (int k = actual_lo; k <= actual_hi; ++k) {
            temp[k - actual_lo] = cube[1][4][k];
        }
        
        /* Store slice with transformation */
        for (int k = actual_lo; k <= actual_hi; ++k) {
            cube[1][4][k] = temp[k - actual_lo] ^ 0x55;
            use_int(cube[1][4][k]);
        }
    }
}

/* Test 4: VLA with constant size expression */
static __attribute__((noinline)) void test_vla_constant(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds for slice */
    const int row_start = 10;
    const int row_end = 19;  /* count = 10 */
    
    volatile int vs = row_start;
    volatile int ve = row_end;
    int start = vs;
    int end = ve;
    
    int row_copy[10];
    
    /* Load entire row slice */
    for (int j = start; j <= end; ++j) {
        row_copy[j - start] = vla[5][j];
    }
    
    /* Store modified values back */
    for (int j = start; j <= end; ++j) {
        vla[5][j] = row_copy[j - start] + 1000;
        use_int(vla[5][j]);
    }
    
    /* Column slice with count = 3 */
    const int col_idx = 15;
    const int col_start = 8;
    const int col_end = 10;  /* count = 3 */
    
    volatile int vcs = col_start;
    volatile int vce = col_end;
    int cstart = vcs;
    int cend = vce;
    
    for (int i = cstart; i <= cend; ++i) {
        vla[i][col_idx] = vla[i][col_idx] * 2;  /* Load and store */
        use_int(vla[i][col_idx]);
    }
}

/* Test 5: Mixed types and complex index calculations that simplify to constants */
static __attribute__((noinline)) void test_mixed_types(void) {
    struct mixed {
        int a;
        double b;
        char c[4];
    } data[20][15];
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 15; j++) {
            data[i][j].a = i * 100 + j;
            data[i][j].b = i * 0.5 + j * 0.25;
            for (int k = 0; k < 4; k++) {
                data[i][j].c[k] = (i + j + k) & 0xFF;
            }
        }
    }
    
    /* Complex but constant bounds calculation */
    const int base = 3;
    const int offset = 4;
    const int lo_idx = base + 1;      /* 4 */
    const int hi_idx = base + offset; /* 7, count = 4 */
    
    /* Use arithmetic that should simplify to constants */
    volatile int vbase = base;
    int computed_lo = vbase + 1;
    int computed_hi = vbase + offset;
    
    /* Access struct slice */
    struct mixed temp[4];
    
    /* Load slice */
    for (int j = computed_lo; j <= computed_hi; ++j) {
        temp[j - computed_lo] = data[10][j];
        use_int(data[10][j].a);
    }
    
    /* Store slice with modification */
    for (int j = computed_lo; j <= computed_hi; ++j) {
        data[10][j].a = temp[j - computed_lo].a + 500;
        data[10][j].b = temp[j - computed_lo].b * 1.5;
        use_ptr(&data[10][j]);
    }
}

/* Test 6: Edge case - exactly count = 2 with different element sizes */
static __attribute__((noinline)) void test_edge_cases(void) {
    long long big_arr[8][12];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 12; j++) {
            big_arr[i][j] = (long long)i << 32 | j;
        }
    }
    
    /* Test count = 2 with 8-byte elements */
    const int edge_lo = 5;
    const int edge_hi = 6;
    
    volatile int velo = edge_lo;
    volatile int vehi = edge_hi;
    int lo = velo;
    int hi = vehi;
    
    long long pair[2];
    
    /* Load pair */
    pair[0] = big_arr[3][lo];
    pair[1] = big_arr[3][hi];
    use_ptr(&pair[0]);
    
    /* Store swapped */
    big_arr[3][lo] = pair[1];
    big_arr[3][hi] = pair[0];
    use_ptr(&big_arr[3][lo]);
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Starting array slice tests...\n");
    
    /* Run all tests */
    test_small_slice();
    checksum += 1;
    
    test_medium_slice();
    checksum += 2;
    
    test_char_array();
    checksum += 3;
    
    test_vla_constant();
    checksum += 4;
    
    test_mixed_types();
    checksum += 5;
    
    test_edge_cases();
    checksum += 6;
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

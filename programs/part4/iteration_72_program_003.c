/* Test program for expr.cc lines 7691-7700 */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile int sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile double sink = val;
    (void)sink;
}

/* Test 1: Small count (count <= 2) with int array */
static void __attribute__((noinline))
test_small_count_int(int arr[][20]) {
    /* Constant bounds that will be known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Access slice in both lvalue and rvalue contexts */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        /* Rvalue context: read from slice */
        int val = arr[3][j];
        use_int(val);
        
        /* Lvalue context: write to slice */
        arr[3][j] = val * 2;
    }
    
    /* Another pattern with count = 1 */
    const int lo2 = 8;
    const int hi2 = 8;  /* count = 1 */
    volatile int vlo2 = lo2;
    int lo_idx2 = vlo2;
    
    /* Mixed access pattern */
    arr[4][lo_idx2] = arr[3][lo_idx2] + 1;
    use_int(arr[4][lo_idx2]);
}

/* Test 2: Larger count with char array (small element size) */
static void __attribute__((noinline))
test_large_count_char(char arr[][50]) {
    /* Constant bounds: count = 10 */
    const int lo = 10;
    const int hi = 19;
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Both read and write operations on the slice */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        /* Read from slice */
        char val = arr[5][j];
        use_int((int)val);
        
        /* Write to slice */
        arr[5][j] = val + 1;
    }
    
    /* Copy slice to another location (memcpy-like pattern) */
    for (int j = 0; j <= (hi_idx - lo_idx); ++j) {
        arr[6][j] = arr[5][lo_idx + j];
    }
}

/* Test 3: Double array with medium count */
static void __attribute__((noinline))
test_double_array(double arr[][15]) {
    /* count = 5 */
    const int lo = 3;
    const int hi = 7;
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Operations that might trigger MEM_P analysis */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        /* Rvalue use */
        double val = arr[2][j];
        use_double(val);
        
        /* Lvalue use with computation */
        arr[2][j] = val * 1.5;
    }
    
    /* Another slice with count = 3 */
    const int lo2 = 10;
    const int hi2 = 12;  /* count = 3 */
    volatile int vlo2 = lo2;
    volatile int vhi2 = hi2;
    
    for (int j = vlo2; j <= vhi2; ++j) {
        arr[3][j] = arr[2][j] + 2.0;
    }
}

/* Test 4: VLA with constant size (affects MEM_P analysis) */
static void __attribute__((noinline))
test_vla_constant(void) {
    /* VLA declared with constant size */
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice access */
    const int lo = 5;
    const int hi = 14;  /* count = 10 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Mixed lvalue/rvalue operations */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        /* Read */
        int val = vla[10][j];
        use_int(val);
        
        /* Write */
        vla[10][j] = val * 3;
    }
    
    /* Small slice: count = 2 */
    const int lo2 = 20;
    const int hi2 = 21;
    volatile int vlo2 = lo2;
    
    vla[15][vlo2] = vla[10][vlo2];
    vla[15][vlo2 + 1] = vla[10][vlo2 + 1];
    
    use_ptr(&vla[0][0]);
}

/* Test 5: Three-dimensional array with slice operations */
static void __attribute__((noinline))
test_3d_array(int arr[][10][10]) {
    /* Access a 2D slice from 3D array */
    const int lo = 2;
    const int hi = 5;  /* count = 4 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Nested loops with constant bounds */
    for (int i = lo_idx; i <= hi_idx; ++i) {
        for (int j = 0; j < 3; ++j) {  /* Small inner loop */
            /* Both read and write */
            int val = arr[1][i][j];
            arr[1][i][j] = val + i + j;
            use_int(arr[1][i][j]);
        }
    }
    
    /* Very small slice: count = 1 */
    const int fixed_idx = 7;
    volatile int vfixed = fixed_idx;
    
    arr[2][vfixed][vfixed] = arr[1][vfixed][vfixed] * 2;
    use_int(arr[2][vfixed][vfixed]);
}

/* Test 6: Mixed element sizes to vary TYPE_SIZE calculation */
static void __attribute__((noinline))
test_mixed_sizes(void) {
    struct mixed {
        char c;
        int i;
        double d;
    } arr[100][10];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 10; ++j) {
            arr[i][j].c = 'A' + (i + j) % 26;
            arr[i][j].i = i * 100 + j;
            arr[i][j].d = i * 1.0 + j * 0.1;
        }
    }
    
    /* Slice with count = 3 */
    const int lo = 3;
    const int hi = 5;
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    
    /* Access different members to trigger different elttype sizes */
    for (int j = vlo; j <= vhi; ++j) {
        /* Access char member (smallest) */
        char c_val = arr[50][j].c;
        use_int((int)c_val);
        
        /* Access int member */
        int i_val = arr[50][j].i;
        arr[50][j].i = i_val + 1;
        
        /* Access double member (largest) */
        double d_val = arr[50][j].d;
        arr[50][j].d = d_val * 2.0;
        use_double(arr[50][j].d);
    }
}

int main(void) {
    /* Declare multi-dimensional arrays with different types */
    int grid[100][50];
    double matrix[10][20];
    char buffer[30][50];
    int cube[5][10][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i * 50 + j;
        }
    }
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 50; ++j) {
            buffer[i][j] = 'a' + (i + j) % 26;
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {
                cube[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Run all tests */
    test_small_count_int(grid);
    test_large_count_char(buffer);
    test_double_array(matrix);
    test_vla_constant();
    test_3d_array(cube);
    test_mixed_sizes();
    
    /* Compute simple checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            checksum += grid[i][j];
            checksum += (int)buffer[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

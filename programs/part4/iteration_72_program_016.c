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

/* Test 1: Multi-dimensional array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
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
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice - lvalue context */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[3][j] = temp[j - actual_lo] * 2;
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Another small slice: count = 1 */
    const int lo2 = 10;
    const int hi2 = 10;  /* count = 1 */
    
    /* Mixed access pattern */
    arr[5][lo2] = arr[5][hi2] + 1;
    use_int(arr[5][lo2]);
}

/* Test 2: Larger slice with different element types */
static void __attribute__((noinline))
test_large_slice(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 25; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds for larger slice */
    const int lo = 3;
    const int hi = 12;  /* count = 10 > 2 */
    
    volatile int vstart = lo;
    volatile int vend = hi;
    int start = vstart;
    int end = vend;
    
    double buffer[10];
    
    /* Load slice - rvalue */
    for (int j = start; j <= end; ++j) {
        buffer[j - start] = matrix[7][j];
        /* Use through opaque function */
        volatile double sink = matrix[7][j];
        (void)sink;
    }
    
    /* Store slice - lvalue */
    for (int j = start; j <= end; ++j) {
        matrix[7][j] = buffer[j - start] * 2.0;
        volatile double sink = matrix[7][j];
        (void)sink;
    }
    
    /* Another test with char type (different TYPE_SIZE) */
    char char_grid[30][40];
    
    const int clo = 5;
    const int chi = 20;  /* count = 16 > 2 */
    
    /* Initialize char array */
    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 40; ++j) {
            char_grid[i][j] = (i + j) % 256;
        }
    }
    
    /* Mixed access pattern */
    for (int j = clo; j <= chi; ++j) {
        char_grid[10][j] = char_grid[10][j] + 1;
        use_int(char_grid[10][j]);
    }
}

/* Test 3: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds */
    const int lo = 10;
    const int hi = 19;  /* count = 10 > 2 */
    
    /* Use volatile intermediate */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    int temp[10];
    
    /* Load from VLA slice */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        temp[j - actual_lo] = vla[15][j];
        use_int(vla[15][j]);
    }
    
    /* Store to VLA slice */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        vla[15][j] = temp[j - actual_lo] * 3;
        use_int(vla[15][j]);
    }
    
    /* Small slice in VLA */
    const int lo2 = 5;
    const int hi2 = 6;  /* count = 2 */
    
    vla[5][lo2] = vla[5][hi2] * 2;
    use_int(vla[5][lo2]);
}

/* Test 4: Complex multi-dimensional access with mixed sizes */
static void __attribute__((noinline))
test_mixed_types(void) {
    /* Different element sizes to test TYPE_SIZE calculation */
    struct mixed {
        int a;
        double b;
        char c[4];
    } data[8][12];
    
    /* Initialize */
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 12; ++j) {
            data[i][j].a = i * 100 + j;
            data[i][j].b = i * 2.5 + j * 0.5;
            for (int k = 0; k < 4; ++k) {
                data[i][j].c[k] = (i + j + k) % 256;
            }
        }
    }
    
    /* Test with count = 3 (just above threshold) */
    const int lo = 2;
    const int hi = 4;  /* count = 3 > 2 */
    
    volatile int idx_start = lo;
    volatile int idx_end = hi;
    int start = idx_start;
    int end = idx_end;
    
    /* Access slice in both contexts */
    for (int j = start; j <= end; ++j) {
        /* Load - rvalue */
        int val = data[3][j].a;
        /* Store - lvalue */
        data[3][j].a = val + 100;
        use_int(data[3][j].a);
    }
    
    /* Another slice with count = 2 */
    const int lo2 = 8;
    const int hi2 = 9;  /* count = 2 */
    
    data[6][lo2].b = data[6][hi2].b * 1.5;
    volatile double sink = data[6][lo2].b;
    (void)sink;
}

/* Test 5: Array of pointers (affects MEM_P analysis) */
static void __attribute__((noinline))
test_pointer_array(void) {
    int buffer[100];
    int *ptr_arr[10][5];
    
    /* Initialize buffer and pointer array */
    for (int i = 0; i < 100; ++i) {
        buffer[i] = i * 2;
    }
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            ptr_arr[i][j] = &buffer[i * 5 + j];
        }
    }
    
    /* Constant slice bounds */
    const int lo = 1;
    const int hi = 3;  /* count = 3 > 2 */
    
    /* Access through pointer array */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        /* Dereference - load from memory */
        sum += *ptr_arr[2][j];
        /* Store through pointer */
        *ptr_arr[2][j] = sum;
    }
    use_int(sum);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run all tests */
    test_small_slice();
    checksum += 1;
    
    test_large_slice();
    checksum += 2;
    
    test_vla_constant();
    checksum += 3;
    
    test_mixed_types();
    checksum += 4;
    
    test_pointer_array();
    checksum += 5;
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

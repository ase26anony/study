/* Test program for expr.cc lines 7691-7700 */
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

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile static double sink;
    sink = val;
}

/* Test 1: Multi-dimensional array with count <= 2 */
static void __attribute__((noinline))
test_small_slice(void) {
    int arr[10][20];
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;  /* Write to slice */
    }
    
    /* Load context (rvalue) - MEM_P(target) might differ */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];  /* Read from slice */
    }
    use_int(sum);
    
    /* Mixed access with volatile index to force middle-end analysis */
    volatile int vlo = 7;
    volatile int vhi = 8;  /* count = 2 */
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[4][j] = arr[3][j] + 1;  /* Both load and store */
    }
}

/* Test 2: Larger slice with count > 2, small element size (char) */
static void __attribute__((noinline))
test_char_slice(void) {
    char matrix[50][100];  /* Small element type */
    
    /* Constant bounds for larger slice */
    const int start = 10;
    const int end = 25;  /* count = 16 > 2 */
    
    /* Initialize slice */
    for (int j = start; j <= end; ++j) {
        matrix[20][j] = (char)(j % 256);
    }
    
    /* Copy slice to another row */
    for (int j = start; j <= end; ++j) {
        matrix[21][j] = matrix[20][j];  /* Both MEM_P contexts */
    }
    
    /* Use volatile to consume result */
    volatile char check = matrix[21][start + 5];
    use_int((int)check);
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline))
test_double_slice(void) {
    double grid[15][30];
    
    /* Different constant bounds */
    const int lo_idx = 3;
    const int hi_idx = 12;  /* count = 10 > 2 */
    
    /* Store operation on slice */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        grid[5][j] = j * 1.5;
    }
    
    /* Load and compute */
    volatile double acc = 0.0;
    for (int j = lo_idx; j <= hi_idx; ++j) {
        acc += grid[5][j];
    }
    use_double(acc);
    
    /* Another slice with different bounds */
    const int lo2 = 20;
    const int hi2 = 29;  /* count = 10 > 2 */
    
    for (int j = lo2; j <= hi2; ++j) {
        grid[6][j] = grid[5][j - 17];  /* Offset access */
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int table[8][12];
    
    const int single_idx = 7;
    
    /* Single element store */
    table[4][single_idx] = 999;
    
    /* Single element load */
    volatile int val = table[4][single_idx];
    use_int(val);
    
    /* Two-element slice using volatile bounds */
    volatile int v1 = 3;
    volatile int v2 = 4;  /* count = 2 */
    int idx1 = v1;
    int idx2 = v2;
    
    table[5][idx1] = 100;
    table[5][idx2] = 200;
    
    volatile int sum2 = table[5][idx1] + table[5][idx2];
    use_int(sum2);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 25;  /* Constant size */
    int vla[n][n];     /* VLA with constant size */
    
    /* Constant bounds */
    const int vla_lo = 5;
    const int vla_hi = 15;  /* count = 11 > 2 */
    
    /* Initialize VLA slice */
    for (int j = vla_lo; j <= vla_hi; ++j) {
        vla[10][j] = j * 3;
    }
    
    /* Copy slice within VLA */
    for (int j = vla_lo; j <= vla_hi; ++j) {
        vla[11][j] = vla[10][j] * 2;
    }
    
    /* Use result */
    volatile int vla_check = vla[11][vla_lo + 3];
    use_int(vla_check);
}

/* Test 6: Mixed types and complex index calculations */
static void __attribute__((noinline))
test_mixed_slices(void) {
    struct Mixed {
        int a;
        char b;
        double c;
    } data[20][15];
    
    /* Various slice sizes */
    const int bounds[][2] = {{1, 2}, {3, 8}, {10, 14}};  /* counts: 2, 6, 5 */
    
    for (int slice = 0; slice < 3; ++slice) {
        const int lo = bounds[slice][0];
        const int hi = bounds[slice][1];
        
        /* Store to slice */
        for (int j = lo; j <= hi; ++j) {
            data[5][j].a = j * 10;
            data[5][j].b = (char)j;
            data[5][j].c = j * 0.5;
        }
        
        /* Load from slice to different row */
        for (int j = lo; j <= hi; ++j) {
            data[6][j].a = data[5][j].a + 1;
            data[6][j].b = data[5][j].b;
            data[6][j].c = data[5][j].c * 2.0;
        }
    }
    
    volatile int check = data[6][3].a;
    use_int(check);
}

/* Test 7: Array of pointers (affects MEM_P analysis) */
static void __attribute__((noinline))
test_pointer_array(void) {
    int base[100];
    int *ptr_arr[10][5];
    
    /* Initialize base array */
    for (int i = 0; i < 100; ++i) {
        base[i] = i * 2;
    }
    
    /* Constant slice of pointers */
    const int plo = 1;
    const int phi = 3;  /* count = 3 > 2 */
    
    /* Store pointers to slice */
    for (int j = plo; j <= phi; ++j) {
        ptr_arr[2][j] = &base[j * 10];
    }
    
    /* Dereference through pointer slice */
    volatile int ptr_sum = 0;
    for (int j = plo; j <= phi; ++j) {
        ptr_sum += *ptr_arr[2][j];
    }
    use_int(ptr_sum);
}

int main(void) {
    printf("Testing array slice bounds analysis...\n");
    
    /* Run all tests to exercise different paths */
    test_small_slice();
    test_char_slice();
    test_double_slice();
    test_single_element();
    test_vla_slice();
    test_mixed_slices();
    test_pointer_array();
    
    /* Create a simple checksum */
    volatile int checksum = 0;
    for (int i = 0; i < 10; ++i) {
        checksum += i;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return 0;
}

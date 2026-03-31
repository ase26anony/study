/* Test program for expr.cc lines 7691-7700 - constant bounds array slice operations */
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
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - reading from slice */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* Load */
        arr[3][j] = temp[j - lo] * 2;  /* Store */
    }
}

/* Test 2: Multi-dimensional double array with medium slice (count > 2) */
static void __attribute__((noinline))
test_medium_slice_double(void) {
    double matrix[15][25] = {0};
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = 8;
    volatile int vhi = 18;
    int lo = vlo;  /* Constant 8 */
    int hi = vhi;  /* Constant 18, count = 11 */
    
    /* Store to slice */
    for (int col = lo; col <= hi; ++col) {
        matrix[7][col] = col * 1.5;
    }
    
    /* Read from slice */
    volatile double acc = 0.0;
    for (int col = lo; col <= hi; ++col) {
        acc += matrix[7][col];
    }
    use_double(acc);
    
    /* Block copy within same row */
    for (int col = lo; col <= hi; ++col) {
        matrix[8][col] = matrix[7][col];  /* Both load and store */
    }
}

/* Test 3: Char array with single element slice (count = 1) */
static void __attribute__((noinline))
test_single_char_slice(void) {
    char buffer[50][100];
    
    /* Single element access */
    const int idx = 42;
    
    /* Store */
    buffer[25][idx] = 'X';
    
    /* Load */
    volatile char c = buffer[25][idx];
    use_int((int)c);
    
    /* Two-element slice in different dimension */
    buffer[25][idx] = 'A';
    buffer[25][idx + 1] = 'B';  /* count = 2 when considered together? */
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Constant bounds within VLA */
    const int start = 10;
    const int end = 15;  /* count = 6 */
    
    /* Initialize slice */
    for (int i = start; i <= end; ++i) {
        vla[20][i] = i * 100;
    }
    
    /* Copy slice to another row */
    for (int i = start; i <= end; ++i) {
        vla[21][i] = vla[20][i];
    }
    
    /* Use volatile to prevent optimization */
    volatile int check = 0;
    for (int i = start; i <= end; ++i) {
        check += vla[21][i];
    }
    use_int(check);
}

/* Test 5: Mixed types and access patterns */
static void __attribute__((noinline))
test_mixed_patterns(void) {
    struct Mixed {
        int a;
        double b;
        char c[4];
    } grid[5][10];
    
    /* Different slice sizes */
    const int bounds[][2] = {{0, 1}, {2, 5}, {6, 9}};  /* count = 2, 4, 4 */
    
    for (int range = 0; range < 3; ++range) {
        int lo = bounds[range][0];
        int hi = bounds[range][1];
        
        /* Store pattern */
        for (int col = lo; col <= hi; ++col) {
            grid[2][col].a = col * 10;
            grid[2][col].b = col * 0.5;
        }
        
        /* Load pattern */
        volatile int sum_a = 0;
        volatile double sum_b = 0.0;
        for (int col = lo; col <= hi; ++col) {
            sum_a += grid[2][col].a;
            sum_b += grid[2][col].b;
        }
        use_int(sum_a);
        use_double(sum_b);
    }
}

/* Test 6: Pointer-based access with constant offsets */
static void __attribute__((noinline))
test_pointer_slice(void) {
    int array[100][50];
    
    /* Get pointer to middle of row */
    int *row_ptr = &array[42][0];
    
    /* Constant offset slice */
    const int offset_lo = 10;
    const int offset_hi = 19;  /* count = 10 */
    
    /* Store through pointer */
    for (int i = offset_lo; i <= offset_hi; ++i) {
        row_ptr[i] = i * 7;
    }
    
    /* Load through pointer */
    volatile int total = 0;
    for (int i = offset_lo; i <= offset_hi; ++i) {
        total += row_ptr[i];
    }
    use_int(total);
    
    /* Use pointer to force MEM_P consideration */
    use_ptr(row_ptr + offset_lo);
}

int main(void) {
    printf("Testing constant bounds array slice operations...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_medium_slice_double();
    test_single_char_slice();
    test_vla_constant_bounds();
    test_mixed_patterns();
    test_pointer_slice();
    
    /* Create a checksum to ensure execution */
    volatile int checksum = 0;
    
    /* Simple array to compute checksum */
    int final_array[5][5];
    const int final_lo = 1;
    const int final_hi = 3;  /* count = 3 */
    
    for (int i = 0; i < 5; i++) {
        for (int j = final_lo; j <= final_hi; j++) {
            final_array[i][j] = i * 10 + j;
            checksum += final_array[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}

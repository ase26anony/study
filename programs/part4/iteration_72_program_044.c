/* Test program for expr.cc lines 7691-7700 coverage */
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

/* Test 1: Multi-dimensional array with small slice (count <= 2) */
static void __attribute__((noinline, noipa)) 
test_small_slice(int arr[][50], int row) {
    volatile int start = 5;  /* Force middle-end analysis */
    volatile int end = 6;    /* hi - lo + 1 = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[row][j] = row * 100 + j;
    }
    
    /* Load context (rvalue) - reading from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[row][j];
    }
    use_int(sum);
}

/* Test 2: Multi-dimensional array with medium slice (count > 2, small element size) */
static void __attribute__((noinline, noipa))
test_char_slice(char arr[][100], int row) {
    volatile int start = 10;
    volatile int end = 25;  /* count = 16, char size = 1, total = 16 bytes */
    int lo = start;
    int hi = end;
    
    /* Mixed store and load operations */
    for (int j = lo; j <= hi; ++j) {
        /* Store operation */
        arr[row][j] = (char)((row + j) & 0xFF);
        
        /* Immediate load operation */
        char val = arr[row][j];
        use_int((int)val);
    }
}

/* Test 3: Multi-dimensional array with large slice (count > 2, larger element size) */
static void __attribute__((noinline, noipa))
test_double_slice(double arr[][30], int row) {
    volatile int start = 0;
    volatile int end = 9;  /* count = 10, double size = 8, total = 80 bytes */
    int lo = start;
    int hi = end;
    
    /* Write to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[row][j] = (double)(row * 10 + j);
    }
    
    /* Read from slice with computation */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += arr[row][j];
    }
    use_double(total);
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline, noipa))
test_single_element(int arr[][40], int row, int col) {
    volatile int idx = col;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Store single element */
    arr[row][lo] = row * 40 + col;
    
    /* Load single element */
    int val = arr[row][hi];
    use_int(val);
}

/* Test 5: VLA with constant size but dynamic type */
static void __attribute__((noinline, noipa))
test_vla_slice(int n) {
    /* VLA with constant size expression */
    int vla[n][n];
    
    volatile int start = 2;
    volatile int end = 5;  /* count = 4 */
    int lo = start;
    int hi = end;
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Access slice of VLA */
    int row = n / 2;
    int sum = 0;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        vla[row][j] = -vla[row][j];
    }
    
    /* Load from slice */
    for (int j = lo; j <= hi; ++j) {
        sum += vla[row][j];
    }
    use_int(sum);
}

/* Test 6: Complex index calculation that simplifies to constants */
static void __attribute__((noinline, noipa))
test_computed_bounds(int arr[][60], int row) {
    volatile int base = 15;
    int lo = base + 0;  /* Compiler should see as constant 15 */
    int hi = base + 4;  /* Compiler should see as constant 19, count = 5 */
    
    /* Store with computed bounds */
    for (int j = lo; j <= hi; ++j) {
        arr[row][j] = (row << 8) | j;
    }
    
    /* Load with same bounds */
    int checksum = 0;
    for (int j = lo; j <= hi; ++j) {
        checksum ^= arr[row][j];
    }
    use_int(checksum);
}

/* Test 7: Different element sizes to test TYPE_SIZE calculation */
static void __attribute__((noinline, noipa))
test_mixed_sizes(void) {
    struct mixed {
        char c;
        int i;
        double d;
    } arr[20][10];
    
    volatile int start = 3;
    volatile int end = 7;  /* count = 5 */
    int lo = start;
    int hi = end;
    
    int row = 5;
    
    /* Store to struct array slice */
    for (int j = lo; j <= hi; ++j) {
        arr[row][j].c = (char)(row + j);
        arr[row][j].i = row * 100 + j;
        arr[row][j].d = (double)(row) / (j + 1);
    }
    
    /* Load from struct array slice */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += arr[row][j].d + arr[row][j].i;
    }
    use_double(total);
}

int main(void) {
    /* Declare multi-dimensional arrays with different types */
    int grid[100][50];
    char buffer[80][100];
    double matrix[25][30];
    int data[20][40];
    int vla_size = 30;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i * 50 + j;
        }
    }
    
    for (int i = 0; i < 80; ++i) {
        for (int j = 0; j < 100; ++j) {
            buffer[i][j] = (char)((i + j) & 0xFF);
        }
    }
    
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 30; ++j) {
            matrix[i][j] = (double)(i * 30 + j) / 10.0;
        }
    }
    
    /* Run all tests to exercise different code paths */
    test_small_slice(grid, 10);          /* count = 2 */
    test_char_slice(buffer, 20);         /* count = 16, char type */
    test_double_slice(matrix, 5);        /* count = 10, double type */
    test_single_element(data, 8, 12);    /* count = 1 */
    test_vla_slice(vla_size);            /* VLA with constant size */
    test_computed_bounds(grid, 30);      /* count = 5, computed bounds */
    test_mixed_sizes();                  /* struct type, count = 5 */
    
    /* Aggregate results and print checksum */
    volatile int final_checksum = 0;
    
    /* Sample some values to prevent dead code elimination */
    for (int i = 0; i < 10; ++i) {
        final_checksum += grid[i][i];
        final_checksum += (int)buffer[i][i];
        final_checksum += (int)matrix[i][i];
    }
    
    printf("Test completed. Final checksum: %d\n", final_checksum);
    return 0;
}

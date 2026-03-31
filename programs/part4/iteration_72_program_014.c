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

/* Test 1: Small slice (count <= 2) - should take first branch */
static void __attribute__((noinline))
test_small_slice(int arr[][50]) {
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Write to slice (lvalue context) */
    for (int j = lo; j <= hi; ++j) {
        arr[10][j] = j * 2;
    }
    
    /* Read from slice (rvalue context) */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[10][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[10][lo] = arr[10][hi] * 3;
    use_int(arr[10][lo]);
}

/* Test 2: Medium slice (count > 2, small element size) */
static void __attribute__((noinline))
test_medium_slice_char(char arr[][100]) {
    /* Use volatile to force middle-end analysis */
    volatile int start = 10;
    const int lo = start;  /* Constant through propagation */
    const int hi = 25;     /* count = 16 */
    
    /* Store context */
    for (int j = lo; j <= hi; ++j) {
        arr[5][j] = (char)(j % 256);
    }
    
    /* Load context */
    volatile char checksum = 0;
    for (int j = lo; j <= hi; ++j) {
        checksum ^= arr[5][j];
    }
    use_int((int)checksum);
}

/* Test 3: Large slice with double elements (count > 2, larger TYPE_SIZE) */
static void __attribute__((noinline))
test_large_slice_double(double matrix[][20]) {
    /* Constant bounds */
    const int lo = 0;
    const int hi = 9;  /* count = 10 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[3][j] = j * 1.5;
    }
    
    /* Copy slice to another location */
    double buffer[10];
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = matrix[3][j];
    }
    
    /* Use results to prevent elimination */
    volatile double total = 0.0;
    for (int j = 0; j < 10; ++j) {
        total += buffer[j];
    }
    use_double(total);
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(int grid[][30]) {
    const int lo = 15;
    const int hi = 15;  /* count = 1 */
    
    /* Both store and load on same element */
    grid[7][lo] = 999;
    volatile int val = grid[7][hi];
    use_int(val);
    
    /* Chain of operations */
    grid[8][lo] = grid[7][hi] * 2;
    use_int(grid[8][lo]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 40;
    int vla[n][n];
    
    /* Initialize with pattern */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice operation */
    const int lo = 10;
    const int hi = 19;  /* count = 10 */
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        vla[5][j] = vla[5][j] * 3;
    }
    
    /* Load from slice */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[5][j];
    }
    use_int(sum);
}

/* Test 6: Multi-dimensional slice with varying element types */
static void __attribute__((noinline))
test_mixed_types(void) {
    struct Mixed {
        char c;
        int i;
        double d;
    } data[10][15];
    
    /* Constant bounds */
    volatile int start = 3;
    const int lo = start;  /* Becomes constant 3 */
    const int hi = 7;      /* count = 5 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        data[2][j].c = 'A' + j;
        data[2][j].i = j * 100;
        data[2][j].d = j * 0.5;
    }
    
    /* Copy slice */
    struct Mixed copy[5];
    for (int j = lo; j <= hi; ++j) {
        copy[j - lo] = data[2][j];
    }
    
    /* Use to prevent elimination */
    volatile char c_sum = 0;
    volatile int i_sum = 0;
    for (int j = 0; j < 5; ++j) {
        c_sum += copy[j].c;
        i_sum += copy[j].i;
    }
    use_int((int)c_sum);
    use_int(i_sum);
}

/* Test 7: Array of pointers slice */
static void __attribute__((noinline))
test_pointer_array(void) {
    int buffer[100];
    int *ptr_array[10][20];
    
    /* Initialize buffer */
    for (int i = 0; i < 100; ++i) {
        buffer[i] = i * 2;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            ptr_array[i][j] = &buffer[(i * 20 + j) % 100];
        }
    }
    
    /* Constant bounds slice */
    const int lo = 5;
    const int hi = 8;  /* count = 4 */
    
    /* Access through slice */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        *ptr_array[3][j] = *ptr_array[3][j] + 1;
        sum += *ptr_array[3][j];
    }
    use_int(sum);
}

int main(void) {
    /* Declare multi-dimensional arrays with different types */
    int grid[100][50];
    char char_array[20][100];
    double matrix[10][20];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i + j;
        }
    }
    
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 100; ++j) {
            char_array[i][j] = (char)((i * j) % 256);
        }
    }
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            matrix[i][j] = i * 0.1 + j * 0.01;
        }
    }
    
    /* Run all tests */
    test_small_slice(grid);
    test_medium_slice_char(char_array);
    test_large_slice_double(matrix);
    test_single_element(grid);
    test_vla_slice();
    test_mixed_types();
    test_pointer_array();
    
    /* Compute and print checksum */
    volatile int checksum = 0;
    for (int i = 0; i < 50; ++i) {
        checksum ^= grid[10][i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

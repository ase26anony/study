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
use_double(double val) {
    volatile static double sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile static void *sink;
    sink = ptr;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
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
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = arr[3][j] + 1;  /* Both load and store */
    }
}

/* Test 2: Multi-dimensional int array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int grid[100][50] = {0};
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = (j - lo) * 100;
    }
    
    /* Copy slice to another row */
    for (int j = lo; j <= hi; ++j) {
        grid[26][j] = grid[25][j];  /* Load from one slice, store to another */
    }
    
    /* Compute checksum */
    int total = 0;
    for (int j = lo; j <= hi; ++j) {
        total += grid[25][j] + grid[26][j];
    }
    use_int(total);
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    volatile int start = 0;
    volatile int end = 4;  /* count = 5 */
    int lo = start;
    int hi = end;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = j * 3.14;
    }
    
    /* Load from slice and transform */
    for (int j = lo; j <= hi; ++j) {
        matrix[6][j] = matrix[5][j] * 2.0;
    }
    
    /* Mixed operation */
    double acc = 0.0;
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = matrix[5][j] + matrix[6][j];
        acc += matrix[5][j];
    }
    use_double(acc);
}

/* Test 4: Char array with small elements */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[8][64];
    volatile int start = 32;
    volatile int end = 63;  /* count = 32 */
    int lo = start;
    int hi = end;
    
    /* Fill slice with pattern */
    for (int j = lo; j <= hi; ++j) {
        buffer[3][j] = (j % 26) + 'A';
    }
    
    /* Copy slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[4][j] = buffer[3][j];
    }
    
    /* Verify copy */
    char check = 0;
    for (int j = lo; j <= hi; ++j) {
        check ^= buffer[3][j] ^ buffer[4][j];
    }
    use_int((int)check);
}

/* Test 5: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int arr[15][25] = {0};
    volatile int idx = 12;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Store single element */
    arr[7][lo] = 0xABCD;
    
    /* Load and modify */
    arr[7][lo] = arr[7][lo] + 1;
    
    /* Use in expression */
    int val = arr[7][lo] * 2;
    use_int(val);
}

/* Test 6: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    const int n = 30;
    int vla[n][n];
    volatile int start = 5;
    volatile int end = 14;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize diagonal slice */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j] = j * j;
    }
    
    /* Copy to another diagonal */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j+1] = vla[j][j];
    }
    
    /* Compute sum */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[j][j] + vla[j][j+1];
    }
    use_int(sum);
}

/* Test 7: Mixed slice sizes in same function */
static void __attribute__((noinline))
test_mixed_slices(void) {
    int data[5][100];
    
    /* Small slice (count = 2) */
    {
        volatile int s1 = 10;
        volatile int e1 = 11;
        int lo1 = s1;
        int hi1 = e1;
        
        for (int j = lo1; j <= hi1; ++j) {
            data[0][j] = data[0][j] + j;
        }
    }
    
    /* Medium slice (count = 5) */
    {
        volatile int s2 = 20;
        volatile int e2 = 24;
        int lo2 = s2;
        int hi2 = e2;
        
        for (int j = lo2; j <= hi2; ++j) {
            data[1][j] = data[0][j] * 2;
        }
    }
    
    /* Larger slice (count = 20) */
    {
        volatile int s3 = 50;
        volatile int e3 = 69;
        int lo3 = s3;
        int hi3 = e3;
        
        for (int j = lo3; j <= hi3; ++j) {
            data[2][j] = data[1][j % 5 + 20] + j;
        }
    }
}

/* Test 8: Pointer-based slice access */
static void __attribute__((noinline))
test_pointer_slice(void) {
    int table[8][16];
    volatile int base = 4;
    volatile int offset = 7;
    int lo = base;
    int hi = offset;  /* count = 4 */
    
    /* Get pointer to row slice */
    int *row_slice = &table[3][lo];
    
    /* Initialize through pointer */
    for (int j = 0; j <= (hi - lo); ++j) {
        row_slice[j] = (j + 1) * 100;
    }
    
    /* Copy to another row using pointer arithmetic */
    int *dest_slice = &table[4][lo];
    for (int j = 0; j <= (hi - lo); ++j) {
        dest_slice[j] = row_slice[j];
    }
    
    use_ptr(row_slice);
    use_ptr(dest_slice);
}

int main(void) {
    printf("Testing array slice operations...\n");
    
    /* Execute all tests */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_char_slice();
    test_single_element();
    test_vla_constant_size();
    test_mixed_slices();
    test_pointer_slice();
    
    /* Create a volatile sink for final result */
    volatile int result = 0;
    result += 1;  /* Prevent elimination */
    
    printf("All tests completed.\n");
    return 0;
}

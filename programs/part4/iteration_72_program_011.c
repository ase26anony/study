/* Test program for expr.cc lines 7691-7700 - array slice operations with constant bounds */
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
test_small_int_slice(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    volatile int vlo = lo;  /* Force middle-end analysis */
    volatile int vhi = hi;
    int actual_lo = vlo;
    int actual_hi = vhi;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        /* Store operation - lvalue context */
        arr[3][j] = j * 10;
        
        /* Load operation - rvalue context */
        int val = arr[3][j];
        use_int(val);
    }
    
    /* Another access with count = 1 */
    int single_val = arr[3][7];
    arr[3][7] = single_val + 1;
    use_int(arr[3][7]);
}

/* Test 2: Larger slice with int array (count > 2) */
static void __attribute__((noinline))
test_large_int_slice(void) {
    int grid[100][50] = {0};
    
    /* Constant bounds for larger slice */
    const int start = 10;
    const int end = 19;  /* count = 10 */
    
    volatile int vstart = start;
    volatile int vend = end;
    int lo_idx = vstart;
    int hi_idx = vend;
    
    /* Mixed store/load operations */
    for (int col = lo_idx; col <= hi_idx; ++col) {
        /* Store to slice */
        grid[25][col] = col * 100;
    }
    
    /* Load from same slice */
    int sum = 0;
    for (int col = lo_idx; col <= hi_idx; ++col) {
        sum += grid[25][col];
    }
    use_int(sum);
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20] = {0.0};
    
    /* Constant bounds */
    const int rlo = 2;
    const int rhi = 5;  /* count = 4 */
    
    volatile int vrlo = rlo;
    volatile int vrhi = rhi;
    int lo = vrlo;
    int hi = vrhi;
    
    /* Store operation */
    for (int i = lo; i <= hi; ++i) {
        matrix[5][i] = i * 1.5;
    }
    
    /* Load operation to different array */
    double copy[4] = {0.0};
    for (int i = 0; i <= hi - lo; ++i) {
        copy[i] = matrix[5][lo + i];
        use_double(copy[i]);
    }
}

/* Test 4: Char array with varying element size */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[8][64] = {0};
    
    /* Very small slice */
    const int clo = 30;
    const int chi = 31;  /* count = 2 */
    
    volatile int vclo = clo;
    volatile int vchi = chi;
    int lo = vclo;
    int hi = vchi;
    
    /* Store char slice */
    for (int pos = lo; pos <= hi; ++pos) {
        buffer[3][pos] = 'A' + (pos % 26);
    }
    
    /* Load char slice */
    char chars[2];
    for (int i = 0; i <= hi - lo; ++i) {
        chars[i] = buffer[3][lo + i];
        use_int(chars[i]);
    }
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    /* VLA with constant size */
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant slice bounds */
    const int vlo = 10;
    const int vhi = 15;  /* count = 6 */
    
    volatile int vvlo = vlo;
    volatile int vvhi = vhi;
    int lo = vvlo;
    int hi = vvhi;
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[20][j] = j * 1000;
    }
    
    /* Load from VLA slice */
    int vla_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        vla_sum += vla[20][j];
    }
    use_int(vla_sum);
}

/* Test 6: Mixed operations to trigger MEM_P check */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x, y;
    } points[5][10];
    
    /* Different slice sizes in same function */
    
    /* Small slice (count = 1) */
    {
        const int idx = 5;
        volatile int vidx = idx;
        int i = vidx;
        
        points[2][i].x = 100;
        points[2][i].y = 200;
        
        int x_val = points[2][i].x;
        int y_val = points[2][i].y;
        use_int(x_val + y_val);
    }
    
    /* Medium slice (count = 3) */
    {
        const int mlo = 2;
        const int mhi = 4;  /* count = 3 */
        
        volatile int vmlo = mlo;
        volatile int vmhi = mhi;
        int lo = vmlo;
        int hi = vmhi;
        
        for (int j = lo; j <= hi; ++j) {
            points[3][j].x = j * 10;
            points[3][j].y = j * 20;
        }
        
        /* Load slice to local array */
        struct Point local[3];
        for (int j = 0; j <= hi - lo; ++j) {
            local[j] = points[3][lo + j];
            use_int(local[j].x);
        }
    }
}

/* Test 7: Array of pointers - different MEM_P behavior */
static void __attribute__((noinline))
test_pointer_array(void) {
    int data[100];
    int *ptr_array[5][10];
    
    /* Initialize data */
    for (int i = 0; i < 100; ++i) {
        data[i] = i * 3;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            ptr_array[i][j] = &data[(i * 10 + j) % 100];
        }
    }
    
    /* Constant slice on pointer array */
    const int plo = 3;
    const int phi = 6;  /* count = 4 */
    
    volatile int vplo = plo;
    volatile int vphi = phi;
    int lo = vplo;
    int hi = vphi;
    
    /* Store through pointers in slice */
    for (int j = lo; j <= hi; ++j) {
        *ptr_array[2][j] = j * 50;
    }
    
    /* Load through pointers in slice */
    int ptr_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        ptr_sum += *ptr_array[2][j];
    }
    use_int(ptr_sum);
}

int main(void) {
    printf("Testing array slice operations with constant bounds\n");
    
    /* Execute all test functions */
    test_small_int_slice();
    test_large_int_slice();
    test_double_slice();
    test_char_slice();
    test_vla_slice();
    test_mixed_operations();
    test_pointer_array();
    
    /* Create a checksum to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += 1;  /* Dummy operation to ensure execution */
    
    printf("All tests completed (checksum: %d)\n", checksum);
    return 0;
}

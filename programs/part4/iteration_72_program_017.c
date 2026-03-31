/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

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
static void __attribute__((noinline, noipa)) 
test_small_slice(int arr[][20]) {
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int actual_lo = vlo;  /* Forces middle-end analysis */
    int actual_hi = vhi;
    
    /* Another slice with constant bounds */
    for (int j = actual_lo; j <= actual_hi; ++j) {
        arr[7][j] = arr[3][j] + 1;
    }
}

/* Test 2: Larger slice with count > 2, int type */
static void __attribute__((noinline, noipa))
test_large_int_slice(int arr[][50]) {
    /* Constant bounds for larger slice */
    const int lo = 10;
    const int hi = 19;  /* count = 10 */
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[5][j] = j * 100;
    }
    
    /* Load from slice */
    int checksum = 0;
    for (int j = lo; j <= hi; ++j) {
        checksum ^= arr[5][j];
    }
    use_int(checksum);
    
    /* Copy between slices (both lvalue and rvalue contexts) */
    for (int j = lo; j <= hi; ++j) {
        arr[6][j] = arr[5][j] * 2;
    }
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline, noipa))
test_double_slice(double matrix[][30]) {
    const int lo = 3;
    const int hi = 7;  /* count = 5 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[2][j] = j * 1.5;
    }
    
    /* Use volatile to force middle-end analysis */
    volatile int vstart = lo;
    volatile int vend = hi;
    int start = vstart;
    int end = vend;
    
    /* Process slice */
    double acc = 0.0;
    for (int j = start; j <= end; ++j) {
        acc += matrix[2][j];
        matrix[3][j] = matrix[2][j] * 2.0;
    }
    
    /* Force use of result */
    volatile double sink = acc;
}

/* Test 4: Char array with varying slice sizes */
static void __attribute__((noinline, noipa))
test_char_slice(char buffer[][100]) {
    /* Test count = 1 */
    {
        const int lo = 25;
        const int hi = 25;  /* count = 1 */
        
        buffer[0][lo] = 'A';
        char c = buffer[0][lo];
        use_int(c);
    }
    
    /* Test count = 2 */
    {
        const int lo = 30;
        const int hi = 31;  /* count = 2 */
        
        for (int j = lo; j <= hi; ++j) {
            buffer[1][j] = 'B' + (j - lo);
        }
        
        char sum = 0;
        for (int j = lo; j <= hi; ++j) {
            sum += buffer[1][j];
        }
        use_int(sum);
    }
    
    /* Test count > 2 with char (small element size) */
    {
        const int lo = 40;
        const int hi = 49;  /* count = 10 */
        
        /* Store pattern */
        for (int j = lo; j <= hi; ++j) {
            buffer[2][j] = '0' + (j - lo);
        }
        
        /* Load and transform */
        for (int j = lo; j <= hi; ++j) {
            buffer[3][j] = buffer[2][j] + 1;
        }
    }
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline, noipa))
test_vla_slice(void) {
    const int n = 30;  /* Constant size for VLA */
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice access */
    const int lo = 10;
    const int hi = 15;  /* count = 6 */
    
    /* Process row slice */
    int row_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        row_sum += vla[5][j];
        vla[6][j] = vla[5][j] * 3;
    }
    use_int(row_sum);
    
    /* Process column slice */
    int col_sum = 0;
    for (int i = lo; i <= hi; ++i) {
        col_sum += vla[i][7];
        vla[i][8] = vla[i][7] + 100;
    }
    use_int(col_sum);
}

/* Test 6: Mixed operations with struct elements */
struct point {
    int x;
    int y;
    double weight;
};

static void __attribute__((noinline, noipa))
test_struct_slice(struct point grid[][40]) {
    const int lo = 8;
    const int hi = 12;  /* count = 5 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        grid[1][j].x = j * 2;
        grid[1][j].y = j * 3;
        grid[1][j].weight = j * 0.5;
    }
    
    /* Copy and modify slice */
    for (int j = lo; j <= hi; ++j) {
        grid[2][j].x = grid[1][j].x + 1;
        grid[2][j].y = grid[1][j].y - 1;
        grid[2][j].weight = grid[1][j].weight * 2.0;
    }
    
    /* Use volatile bounds */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int start = vlo;
    int end = vhi;
    
    /* Process with volatile bounds */
    double total_weight = 0.0;
    for (int j = start; j <= end; ++j) {
        total_weight += grid[2][j].weight;
    }
    volatile double sink = total_weight;
}

/* Test 7: Nested loops with constant bounds */
static void __attribute__((noinline, noipa))
test_nested_slice(int arr3d[][20][30]) {
    /* Constant bounds for middle dimension */
    const int lo = 5;
    const int hi = 9;  /* count = 5 */
    
    /* 3D slice access */
    for (int i = 0; i < 10; ++i) {
        for (int j = lo; j <= hi; ++j) {
            for (int k = 0; k < 5; ++k) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Extract and process 2D slice */
    int slice_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        for (int k = 0; k < 5; ++k) {
            slice_sum += arr3d[3][j][k];
        }
    }
    use_int(slice_sum);
}

int main(void) {
    /* Declare multi-dimensional arrays of different types */
    int grid[100][50] = {{0}};
    double matrix[10][30] = {{0.0}};
    char buffer[5][100] = {{0}};
    struct point point_grid[5][40];
    int arr3d[10][20][30];
    
    /* Run all tests */
    test_small_slice(grid);
    test_large_int_slice(grid);
    test_double_slice(matrix);
    test_char_slice(buffer);
    test_vla_slice();
    test_struct_slice(point_grid);
    test_nested_slice(arr3d);
    
    /* Compute and print a simple checksum */
    int checksum = 0;
    
    /* Sample some values from each array */
    for (int i = 0; i < 10; ++i) {
        checksum ^= grid[i][15];
    }
    
    for (int i = 0; i < 5; ++i) {
        checksum ^= (int)matrix[i][10];
    }
    
    for (int i = 0; i < 5; ++i) {
        checksum ^= buffer[i][45];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

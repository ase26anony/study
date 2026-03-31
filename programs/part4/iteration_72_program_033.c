/* Test program for expr.cc lines 7691-7700 - constant bounds array slicing */

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

/* Test 1: Multi-dimensional array with small slice (count <= 2) */
static void __attribute__((noinline, noipa)) 
test_small_slice(int arr[][20]) {
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Use volatile to force middle-end analysis */
    volatile int vlo = lo;
    volatile int vhi = hi;
    int lo_idx = vlo;
    int hi_idx = vhi;
    
    /* Access slice in both lvalue and rvalue contexts */
    for (int i = lo_idx; i <= hi_idx; ++i) {
        /* rvalue context - load from slice */
        int val = arr[3][i];
        use_int(val);
        
        /* lvalue context - store to slice */
        arr[3][i] = i * 2;
    }
    
    /* Another access pattern with count = 1 */
    int single = arr[3][lo_idx];
    arr[3][lo_idx] = single + 1;
}

/* Test 2: Larger slice with different element type */
static void __attribute__((noinline, noipa))
test_large_slice_double(double matrix[][30]) {
    /* Constant bounds for larger slice */
    const int start = 10;
    const int end = 19;  /* count = 10 */
    
    volatile int vstart = start;
    volatile int vend = end;
    int lo = vstart;
    int hi = vend;
    
    /* Mixed lvalue/rvalue operations on the slice */
    for (int col = lo; col <= hi; ++col) {
        /* Read from slice */
        double temp = matrix[5][col];
        matrix[5][col] = temp * 1.5;  /* Write to slice */
    }
    
    /* Block copy within the same row - triggers MEM_P analysis */
    for (int col = lo; col <= hi; ++col) {
        matrix[6][col] = matrix[5][col];
    }
}

/* Test 3: Char array with medium slice */
static void __attribute__((noinline, noipa))
test_char_slice(char buffer[][100]) {
    const int lo = 20;
    const int hi = 29;  /* count = 10 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int start = vlo;
    int end = vhi;
    
    /* Access char slice */
    for (int i = start; i <= end; ++i) {
        buffer[2][i] = buffer[1][i] + 1;
    }
    
    /* Smaller slice within the same operation */
    buffer[2][start] = 'A';
    buffer[2][start + 1] = 'B';  /* count = 2 for this pattern */
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline, noipa))
test_vla_slice(void) {
    const int n = 40;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice access */
    const int slice_start = 15;
    const int slice_end = 24;  /* count = 10 */
    
    volatile int vs = slice_start;
    volatile int ve = slice_end;
    int lo = vs;
    int hi = ve;
    
    /* Process slice in row 10 */
    for (int j = lo; j <= hi; ++j) {
        int old = vla[10][j];
        vla[10][j] = old * 3;
    }
    
    /* Copy slice to another row */
    for (int j = lo; j <= hi; ++j) {
        vla[11][j] = vla[10][j];
    }
    
    use_ptr(vla);
}

/* Test 5: Mixed operations with different counts */
static void __attribute__((noinline, noipa))
test_mixed_counts(int grid[][50]) {
    /* Test count = 1 */
    {
        const int pos = 25;
        volatile int vpos = pos;
        int idx = vpos;
        
        int val = grid[7][idx];
        grid[7][idx] = val + 100;
        grid[8][idx] = grid[7][idx];
    }
    
    /* Test count = 2 */
    {
        const int lo = 30;
        const int hi = 31;
        volatile int vlo = lo;
        volatile int vhi = hi;
        int start = vlo;
        int end = vhi;
        
        for (int i = start; i <= end; ++i) {
            grid[9][i] = grid[9][i] * 2;
        }
    }
    
    /* Test count = 8 (larger than 2) */
    {
        const int lo = 0;
        const int hi = 7;
        volatile int vlo = lo;
        volatile int vhi = hi;
        int start = vlo;
        int end = vhi;
        
        /* Block operation that should trigger MEM_P with count > 2 */
        for (int i = start; i <= end; ++i) {
            grid[10][i] = i * grid[11][i];
        }
    }
}

/* Test 6: Structure array to test different TYPE_SIZE */
struct point {
    int x;
    int y;
    double z;
};

static void __attribute__((noinline, noipa))
test_struct_slice(struct point points[][20]) {
    const int lo = 5;
    const int hi = 8;  /* count = 4 */
    
    volatile int vlo = lo;
    volatile int vhi = hi;
    int start = vlo;
    int end = vhi;
    
    /* Access structure slice */
    for (int i = start; i <= end; ++i) {
        points[1][i].x = points[0][i].x;
        points[1][i].y = points[0][i].y + 1;
        points[1][i].z = points[0][i].z * 2.0;
    }
}

int main(void) {
    /* Declare multi-dimensional arrays with different types */
    int grid[100][50];
    double matrix[15][30];
    char buffer[10][100];
    struct point points[5][20];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i * 50 + j;
        }
    }
    
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 30; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 100; ++j) {
            buffer[i][j] = (i + j) % 256;
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 20; ++j) {
            points[i][j].x = i * 20 + j;
            points[i][j].y = (i * 20 + j) * 2;
            points[i][j].z = (i * 20 + j) * 0.1;
        }
    }
    
    /* Run all tests */
    test_small_slice(grid);
    test_large_slice_double(matrix);
    test_char_slice(buffer);
    test_vla_slice();
    test_mixed_counts(grid);
    test_struct_slice(points);
    
    /* Compute simple checksum to ensure execution */
    volatile int checksum = 0;
    for (int i = 0; i < 50; ++i) {
        checksum += grid[3][i];
        checksum += (int)matrix[5][i % 30];
        checksum += buffer[2][i % 100];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

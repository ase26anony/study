/* test_expr_coverage.c */
#include <stdio.h>
#include <stdint.h>

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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - write to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - read from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] + 100;
    use_int(arr[3][lo]);
}

/* Test 2: 2D int array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int arr[15][25] = {0};
    volatile int start = 3;
    volatile int end = 12;  /* count = 10 */
    const int lo = start;   /* Constant through propagation */
    const int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        arr[7][j] = j * 7;
    }
    
    /* Copy slice to another row */
    for (int j = lo; j <= hi; ++j) {
        arr[8][j] = arr[7][j] * 2;
    }
    
    /* Verify with volatile read */
    volatile int check = 0;
    for (int j = lo; j <= hi; ++j) {
        check += arr[8][j];
    }
    use_int(check);
}

/* Test 3: 2D char array with varying element size */
static void __attribute__((noinline))
test_char_array(void) {
    char grid[50][100];
    volatile int r = 10;
    volatile int c_start = 20;
    volatile int c_end = 29;  /* count = 10 */
    const int lo = c_start;
    const int hi = c_end;
    
    /* Fill slice with pattern */
    for (int j = lo; j <= hi; ++j) {
        grid[r][j] = (j % 26) + 'A';
    }
    
    /* Copy slice to different row */
    for (int j = lo; j <= hi; ++j) {
        grid[r+1][j] = grid[r][j] + 32;  /* to lowercase */
    }
    
    /* Use in expression */
    char first = grid[r][lo];
    char last = grid[r][hi];
    use_int(first + last);
}

/* Test 4: 2D double array - different TYPE_SIZE */
static void __attribute__((noinline))
test_double_array(void) {
    double matrix[8][16];
    volatile int row = 4;
    volatile int col_start = 2;
    volatile int col_end = 5;  /* count = 4 */
    const int lo = col_start;
    const int hi = col_end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[row][j] = j * 1.5;
    }
    
    /* Compute using slice */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += matrix[row][j];
    }
    
    /* Store back to different slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[row+1][j] = matrix[row][j] / total;
    }
    
    use_double(matrix[row+1][lo]);
}

/* Test 5: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int data[30][40];
    volatile int x = 15;
    volatile int y = 25;
    const int idx = y;  /* Single element */
    
    /* Both store and load on same element */
    data[x][idx] = 999;
    int val = data[x][idx] * 2;
    data[x+1][idx] = val;
    
    use_int(data[x+1][idx]);
}

/* Test 6: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    const int lo = start;
    const int hi = end;
    
    /* Diagonal slice operation */
    for (int i = lo; i <= hi; ++i) {
        vla[i][i] = i * i;
    }
    
    /* Anti-diagonal copy */
    for (int i = lo; i <= hi; ++i) {
        vla[i][n-1-i] = vla[i][i];
    }
    
    /* Force materialization */
    int sum = 0;
    for (int i = lo; i <= hi; ++i) {
        sum += vla[i][i] + vla[i][n-1-i];
    }
    use_int(sum);
}

/* Test 7: Mixed operations in same function */
static void __attribute__((noinline))
test_mixed_operations(void) {
    int table[12][18];
    
    /* First: small slice (count = 2) */
    {
        volatile int s1 = 5;
        volatile int e1 = 6;
        const int l1 = s1, h1 = e1;
        
        for (int j = l1; j <= h1; ++j) {
            table[2][j] = j * 11;
        }
        
        int tmp = table[2][l1] + table[2][h1];
        table[3][l1] = tmp;
    }
    
    /* Second: larger slice (count = 8) */
    {
        volatile int s2 = 8;
        volatile int e2 = 15;
        const int l2 = s2, h2 = e2;
        
        for (int j = l2; j <= h2; ++j) {
            table[4][j] = table[2][j % 6] * 3;
        }
        
        /* Cross-slice copy */
        for (int j = l2; j <= h2; ++j) {
            table[5][j] = table[4][j] + table[3][j % 2];
        }
    }
    
    use_ptr(&table[0][0]);
}

/* Test 8: Struct array to test complex elttype */
struct point {
    int x;
    int y;
    double z;
};

static void __attribute__((noinline))
test_struct_array(void) {
    struct point pts[5][10];
    volatile int start = 2;
    volatile int end = 4;  /* count = 3 */
    const int lo = start;
    const int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        pts[2][j].x = j * 10;
        pts[2][j].y = j * 20;
        pts[2][j].z = j * 1.1;
    }
    
    /* Copy slice */
    for (int j = lo; j <= hi; ++j) {
        pts[3][j] = pts[2][j];
        pts[3][j].x += 5;
    }
    
    use_double(pts[3][lo].z);
}

int main(void) {
    printf("Testing array slice operations...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_int();
    test_char_array();
    test_double_array();
    test_single_element();
    test_vla_constant();
    test_mixed_operations();
    test_struct_array();
    
    /* Create a checksum to prevent optimization */
    volatile int checksum = 0;
    checksum += 1;  /* Dummy computation */
    
    printf("Done. Checksum: %d\n", checksum);
    return 0;
}

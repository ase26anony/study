/* expr_coverage.c - Target coverage for expr.cc lines 7691-7700 */

#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) use_int(int val) { 
    volatile int sink = val; 
    (void)sink;
}
static void __attribute__((noinline, noipa)) use_double(double val) { 
    volatile double sink = val; 
    (void)sink;
}
static void __attribute__((noinline, noipa)) use_char(char val) { 
    volatile char sink = val; 
    (void)sink;
}

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline))
test_small_slice_int(int arr[][50], int row) {
    /* Force constant bounds through volatile */
    volatile int vlo = get_const_low();
    volatile int vhi = get_small_high();
    int lo = vlo;  /* Should be 5 */
    int hi = vhi;  /* Should be 6 */
    
    /* count = 2 (6-5+1) triggers count <= 2 branch */
    for (int j = lo; j <= hi; ++j) {
        /* Store context (lvalue) */
        arr[row][j] = row * 100 + j;
        
        /* Load context (rvalue) */
        int val = arr[row][j];
        use_int(val);
    }
}

/* Test 2: Multi-dimensional int array with count > 2 */
static void __attribute__((noinline))
test_large_slice_int(int arr[][50], int row) {
    volatile int vlo = get_const_low();
    volatile int vhi = get_const_high();
    int lo = vlo;  /* Should be 5 */
    int hi = vhi;  /* Should be 10 */
    
    /* count = 6 (10-5+1) triggers count > 2 branch */
    for (int j = lo; j <= hi; ++j) {
        /* Mixed store and load operations */
        if (j % 2 == 0) {
            /* Store operation */
            arr[row][j] = row * 1000 + j * 10;
        } else {
            /* Load operation */
            int val = arr[row][j];
            use_int(val);
        }
    }
}

/* Test 3: Double array with different element size */
static void __attribute__((noinline))
test_double_slice(double matrix[][20], int row) {
    volatile int vlo = 3;
    volatile int vhi = 12;
    int lo = vlo;  /* 3 */
    int hi = vhi;  /* 12 */
    
    /* count = 10, TYPE_SIZE(double) * count calculation */
    for (int j = lo; j <= hi; ++j) {
        /* Store context */
        matrix[row][j] = (row + 1) * 3.14159 * j;
        
        /* Load context */
        double val = matrix[row][j];
        use_double(val);
    }
}

/* Test 4: Char array - small element size affects TYPE_SIZE calculation */
static void __attribute__((noinline))
test_char_slice(char buffer[][100], int row) {
    volatile int vlo = 20;
    volatile int vhi = 45;
    int lo = vlo;  /* 20 */
    int hi = vhi;  /* 45 */
    
    /* count = 26, but TYPE_SIZE(char) = 1 */
    for (int j = lo; j <= hi; ++j) {
        /* Alternate between store and load */
        if ((j - lo) % 3 == 0) {
            buffer[row][j] = (row + j) % 26 + 'A';
        } else {
            char val = buffer[row][j];
            use_char(val);
        }
    }
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(int size) {
    /* VLA declared with constant size (size is constant 30) */
    int vla[size][size];
    
    /* Initialize VLA */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            vla[i][j] = i * size + j;
        }
    }
    
    volatile int vlo = 8;
    volatile int vhi = 15;
    int lo = vlo;  /* 8 */
    int hi = vhi;  /* 15 */
    
    /* Process a slice - count = 8 */
    for (int i = 0; i < size; ++i) {
        for (int j = lo; j <= hi; ++j) {
            /* Store operation */
            vla[i][j] = vla[i][j] * 2 + 1;
            
            /* Load operation */
            int val = vla[i][j];
            use_int(val);
        }
    }
}

/* Test 6: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(int arr[][50], int row) {
    volatile int idx = 25;
    int lo = idx;  /* 25 */
    int hi = idx;  /* 25 */
    
    /* count = 1 triggers count <= 2 branch */
    /* Store */
    arr[row][lo] = 0xABCD;
    
    /* Load */
    int val = arr[row][hi];
    use_int(val);
}

/* Test 7: Two element slice (count = 2) with pointer arithmetic */
static void __attribute__((noinline))
test_two_elements(double arr[][40], int row) {
    volatile int vlo = 18;
    volatile int vhi = 19;
    int lo = vlo;  /* 18 */
    int hi = vhi;  /* 19 */
    
    /* Direct pointer access to create MEM_P pattern */
    double *start = &arr[row][lo];
    double *end = &arr[row][hi];
    
    /* Store through pointers */
    for (double *p = start; p <= end; ++p) {
        *p = (p - start) * 1.5;
    }
    
    /* Load through pointers */
    for (double *p = start; p <= end; ++p) {
        double val = *p;
        use_double(val);
    }
}

int main(void) {
    /* Multi-dimensional arrays with constant bounds */
    int grid[100][50];
    double matrix[10][20];
    char buffer[30][100];
    
    volatile int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int(grid, 7);      /* count = 2 */
    test_large_slice_int(grid, 3);      /* count = 6 */
    test_double_slice(matrix, 5);       /* count = 10, double type */
    test_char_slice(buffer, 2);         /* count = 26, char type */
    test_vla_slice(30);                 /* VLA with constant size */
    test_single_element(grid, 9);       /* count = 1 */
    test_two_elements(matrix, 2);       /* count = 2, pointer access */
    
    /* Compute simple checksum to prevent dead code elimination */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            checksum += (int)matrix[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

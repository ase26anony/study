/* test_expr_coverage.c */
#include <stdio.h>
#include <string.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) sink_int(int val) { volatile int sink = val; (void)sink; }
static void __attribute__((noinline, noipa)) sink_double(double val) { volatile double sink = val; (void)sink; }
static void __attribute__((noinline, noipa)) sink_char(char val) { volatile char sink = val; (void)sink; }

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice_int(void) {
    int arr[100][50];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 50; ++j)
            arr[i][j] = i * 50 + j;
    
    /* Use volatile to force middle-end analysis */
    volatile int start = 5;
    int lo = start;  /* lo = 5 */
    int hi = lo + 1; /* hi = 6, count = 2 */
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice: rvalue context */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[20][j];
    }
    
    /* Store to slice: lvalue context */
    for (int j = lo; j <= hi; ++j) {
        arr[20][j] = temp[j - lo] * 2;
    }
    
    /* Consume results */
    sink_int(arr[20][lo]);
    sink_int(arr[20][hi]);
}

/* Test 2: Larger slice with count > 2, int type */
static void __attribute__((noinline)) test_large_slice_int(void) {
    int grid[100][50];
    
    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 50; ++j)
            grid[i][j] = i * 100 + j * 3;
    
    /* Constant bounds through opaque calls */
    int lo = get_const_low();  /* 5 */
    int hi = get_const_high(); /* 10, count = 6 */
    
    /* Mixed operations on slice */
    int buffer[6];
    
    /* Read slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = grid[30][j];
    }
    
    /* Process and write back */
    for (int j = lo; j <= hi; ++j) {
        grid[30][j] = buffer[j - lo] + grid[30][j-1];
    }
    
    /* Force materialization */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += grid[30][j];
    }
    sink_int(sum);
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[10][20];
    
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 20; ++j)
            matrix[i][j] = i * 1.5 + j * 0.7;
    
    /* Different element size (double) */
    volatile int vlo = 8;
    int lo = vlo;  /* 8 */
    int hi = lo + 3; /* 11, count = 4 */
    
    double temp[4];
    
    /* Copy slice out */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = matrix[5][j];
    }
    
    /* Transform and copy back */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = temp[j - lo] * 0.5;
    }
    
    /* Consume */
    sink_double(matrix[5][lo]);
    sink_double(matrix[5][hi]);
}

/* Test 4: Char array with single element (count = 1) */
static void __attribute__((noinline)) test_char_single(void) {
    char buffer[256][128];
    
    for (int i = 0; i < 256; ++i)
        for (int j = 0; j < 128; ++j)
            buffer[i][j] = (i + j) & 0xFF;
    
    /* Single element access */
    volatile int idx = 100;
    int lo = idx;  /* 100 */
    int hi = lo;   /* 100, count = 1 */
    
    /* Both contexts for single element */
    char val = buffer[50][lo];  /* load */
    buffer[50][lo] = val ^ 0x55; /* store */
    
    sink_char(buffer[50][lo]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant_slice(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            vla[i][j] = i * n + j;
    
    /* Constant bounds */
    volatile int start = 15;
    int lo = start;  /* 15 */
    int hi = lo + 4; /* 19, count = 5 */
    
    /* Access slice */
    int slice[5];
    
    /* Load from VLA slice */
    for (int j = lo; j <= hi; ++j) {
        slice[j - lo] = vla[10][j];
    }
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = slice[j - lo] * 3;
    }
    
    /* Verify */
    volatile int check = 0;
    for (int j = lo; j <= hi; ++j) {
        check += vla[10][j];
    }
    sink_int(check);
}

/* Test 6: Mixed operations triggering MEM_P check */
static void __attribute__((noinline)) test_mixed_operations(void) {
    struct Data {
        int a[10][20];
        double b[5][15];
    } data;
    
    /* Initialize */
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 20; ++j)
            data.a[i][j] = i * 20 + j;
    
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 15; ++j)
            data.b[i][j] = i * 1.1 + j * 0.3;
    
    /* Different slice sizes in same function */
    
    /* Small slice: count = 2 */
    {
        volatile int lo_small = 3;
        int lo = lo_small;
        int hi = lo + 1;
        
        int temp[2];
        for (int j = lo; j <= hi; ++j) {
            temp[j - lo] = data.a[5][j];
        }
        for (int j = lo; j <= hi; ++j) {
            data.a[5][j] = temp[j - lo] + 1;
        }
    }
    
    /* Larger slice: count = 8 */
    {
        int lo = 8;
        int hi = 15;  /* count = 8 */
        
        double temp[8];
        for (int j = lo; j <= hi; ++j) {
            temp[j - lo] = data.b[2][j];
        }
        for (int j = lo; j <= hi; ++j) {
            data.b[2][j] = temp[j - lo] * 2.0;
        }
    }
    
    /* Consume results */
    sink_int(data.a[5][3]);
    sink_double(data.b[2][8]);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_int();
    checksum += 2;
    
    test_double_slice();
    checksum += 3;
    
    test_char_single();
    checksum += 4;
    
    test_vla_constant_slice();
    checksum += 5;
    
    test_mixed_operations();
    checksum += 6;
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

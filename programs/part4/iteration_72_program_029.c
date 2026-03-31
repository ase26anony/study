/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) sink_int(int val) { 
    volatile int sink = val; 
    (void)sink;
}
static void __attribute__((noinline, noipa)) sink_double(double val) { 
    volatile double sink = val; 
    (void)sink;
}
static void __attribute__((noinline, noipa)) sink_char(char val) { 
    volatile char sink = val; 
    (void)sink;
}

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice_int(void) {
    int arr[100][50];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Use volatile to force middle-end analysis */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[20][j] = j * 1000;  /* Constant row, variable column but bounded */
    }
    
    /* Load context (rvalue) - reading from slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[20][j];
    }
    sink_int(sum);
    
    /* Another pattern with count = 1 */
    volatile int single = 7;
    int idx = single;
    arr[30][idx] = 9999;  /* Store */
    int val = arr[30][idx];  /* Load */
    sink_int(val);
}

/* Test 2: Double array with larger count (>2) */
static void __attribute__((noinline)) test_large_slice_double(void) {
    double matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int low = 3;
    volatile int high = 12;  /* count = 10 */
    int lo = low;
    int hi = high;
    
    /* Mixed store/load operations */
    for (int j = lo; j <= hi; ++j) {
        /* Store operation */
        matrix[5][j] = matrix[5][j] * 2.0;
    }
    
    /* Load operation into another array */
    double buffer[20];
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = matrix[5][j];
    }
    
    /* Consume results */
    double total = 0.0;
    for (int j = 0; j <= (hi - lo); j++) {
        total += buffer[j];
    }
    sink_double(total);
}

/* Test 3: Char array with varying element size considerations */
static void __attribute__((noinline)) test_char_array_slice(void) {
    char grid[50][100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 100; j++) {
            grid[i][j] = (i + j) % 256;
        }
    }
    
    /* Test with count = 3 (just above threshold) */
    volatile int c_low = 40;
    volatile int c_high = 42;  /* count = 3 */
    int lo = c_low;
    int hi = c_high;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = 'A' + (j - lo);
    }
    
    /* Load from slice */
    char chars[10];
    for (int j = lo; j <= hi; ++j) {
        chars[j - lo] = grid[25][j];
        sink_char(chars[j - lo]);
    }
    
    /* Another test with count = 15 (TYPE_SIZE * count calculation) */
    volatile int big_low = 60;
    volatile int big_high = 74;  /* count = 15 */
    int lo2 = big_low;
    int hi2 = big_high;
    
    /* Block copy within same row */
    for (int j = lo2; j <= hi2; ++j) {
        grid[30][j] = grid[25][j - 5];
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant_bounds(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds through opaque function */
    int low = get_const_low();
    int high = get_const_high();  /* count = 6 */
    
    /* Force analysis with volatile intermediate */
    volatile int v_low = low;
    volatile int v_high = high;
    int lo = v_low;
    int hi = v_high;
    
    /* Store operation on VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = vla[15][j] * 3;
    }
    
    /* Load operation from VLA slice */
    int extract[20];
    for (int j = lo; j <= hi; ++j) {
        extract[j - lo] = vla[15][j];
    }
    
    /* Consume */
    int sum = 0;
    for (int j = 0; j <= (hi - lo); j++) {
        sum += extract[j];
    }
    sink_int(sum);
}

/* Test 5: Mixed operations triggering MEM_P checks */
static void __attribute__((noinline)) test_mixed_operations(void) {
    struct Point {
        int x;
        int y;
        double z;
    } points[20][30];
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            points[i][j].x = i;
            points[i][j].y = j;
            points[i][j].z = i * 0.5 + j * 0.25;
        }
    }
    
    /* Different count values in same function */
    
    /* Case 1: count = 2 */
    volatile int low1 = 10;
    volatile int high1 = 11;
    int lo1 = low1;
    int hi1 = high1;
    
    for (int j = lo1; j <= hi1; ++j) {
        points[5][j].x = points[5][j].y * 2;
    }
    
    /* Case 2: count = 8 (TYPE_SIZE * count calculation for struct) */
    volatile int low2 = 20;
    volatile int high2 = 27;
    int lo2 = low2;
    int hi2 = high2;
    
    /* Block operation that might trigger MEM_P */
    for (int j = lo2; j <= hi2; ++j) {
        points[10][j].z = points[10][j].z + 1.0;
    }
    
    /* Load a slice */
    double z_vals[30];
    for (int j = lo2; j <= hi2; ++j) {
        z_vals[j - lo2] = points[10][j].z;
    }
    
    /* Consume */
    double z_sum = 0.0;
    for (int j = 0; j <= (hi2 - lo2); j++) {
        z_sum += z_vals[j];
    }
    sink_double(z_sum);
}

/* Test 6: Array of pointers (non-MEM_P case) */
static void __attribute__((noinline)) test_pointer_array(void) {
    int data[100];
    int *arr[50][40];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 40; j++) {
            arr[i][j] = &data[(i * 40 + j) % 100];
        }
    }
    
    /* Constant bounds */
    volatile int low = 15;
    volatile int high = 25;  /* count = 11 */
    int lo = low;
    int hi = high;
    
    /* Dereference through pointer array slice */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += *arr[20][j];
        *arr[20][j] = sum;  /* Store through pointer */
    }
    sink_int(sum);
}

int main(void) {
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_double();
    test_char_array_slice();
    test_vla_constant_bounds();
    test_mixed_operations();
    test_pointer_array();
    
    printf("All tests completed.\n");
    
    /* Simple checksum to ensure execution */
    volatile int checksum = 0;
    checksum += get_const_low();
    checksum += get_const_high();
    checksum += get_small_high();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

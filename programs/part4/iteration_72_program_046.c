/* test_expr_coverage.c */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) use_int(int val) { 
    asm volatile("" : : "r"(val) : "memory"); 
}
static void __attribute__((noinline, noipa)) use_double(double val) { 
    asm volatile("" : : "r"(val) : "memory"); 
}
static void __attribute__((noinline, noipa)) use_ptr(void *ptr) { 
    asm volatile("" : : "r"(ptr) : "memory"); 
}

/* Test 1: Multi-dimensional int array with count > 2 */
static void __attribute__((noinline)) test_int_array_slice(void) {
    int arr[100][50] = {0};
    volatile int start = get_const_low();  /* Should be 5 */
    volatile int end = get_const_high();   /* Should be 10 */
    
    int lo = start;  /* lo_index = 5 */
    int hi = end;    /* hi_index = 10 */
    
    /* Write to slice (lvalue context) */
    for (int j = lo; j <= hi; ++j) {
        arr[20][j] = j * 2;  /* Store operation */
    }
    
    /* Read from slice (rvalue context) */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[20][j];  /* Load operation */
    }
    
    use_int(sum);
    
    /* Mixed load/store in same loop */
    for (int j = lo; j <= hi; ++j) {
        arr[21][j] = arr[20][j] + 1;  /* Both load and store */
    }
}

/* Test 2: Small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    double matrix[30][40] = {0};
    volatile int start = 8;
    volatile int end = get_small_high();  /* Should be 6, but will use start..start+1 */
    
    int lo = start;      /* lo_index = 8 */
    int hi = start + 1;  /* hi_index = 9, count = 2 */
    
    /* Write to 2-element slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[15][j] = j * 1.5;
    }
    
    /* Read from 2-element slice */
    double prod = 1.0;
    for (int j = lo; j <= hi; ++j) {
        prod *= matrix[15][j];
    }
    
    use_double(prod);
}

/* Test 3: Different element types to vary TYPE_SIZE */
static void __attribute__((noinline)) test_char_array(void) {
    char buffer[1000][100] = {0};
    volatile int start = 20;
    volatile int end = 50;  /* count = 31 */
    
    int lo = start;  /* lo_index = 20 */
    int hi = end;    /* hi_index = 50 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[99][j] = (j % 26) + 'A';
    }
    
    /* Copy slice to another location */
    for (int j = lo; j <= hi; ++j) {
        buffer[98][j] = buffer[99][j];
    }
    
    /* Use pointer to prevent elimination */
    use_ptr(&buffer[98][lo]);
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline)) test_single_element(void) {
    long long bigarr[50][60] = {0};
    volatile int idx = 42;
    
    int lo = idx;  /* lo_index = 42 */
    int hi = idx;  /* hi_index = 42, count = 1 */
    
    /* Single element store */
    bigarr[25][lo] = 0x123456789ABCDEFLL;
    
    /* Single element load */
    long long val = bigarr[25][hi];
    use_int((int)(val >> 32));
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant_size(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];     /* VLA with constant dimensions */
    
    volatile int start = 5;
    volatile int end = 15;  /* count = 11 */
    
    int lo = start;  /* lo_index = 5 */
    int hi = end;    /* hi_index = 15 */
    
    /* Initialize diagonal slice */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j] = j * 3;
    }
    
    /* Copy to another slice */
    for (int j = lo; j <= hi; ++j) {
        vla[j][j+1] = vla[j][j] + 1;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[j][j] + vla[j][j+1];
    }
    
    use_int(sum);
}

/* Test 6: Mixed operations with different counts */
static void __attribute__((noinline)) test_mixed_counts(void) {
    struct point { double x, y, z; } cloud[100][200];
    
    /* Test count = 3 */
    volatile int lo1 = 10;
    volatile int hi1 = 12;  /* count = 3 */
    
    for (int j = lo1; j <= hi1; ++j) {
        cloud[50][j].x = j * 0.1;
        cloud[50][j].y = j * 0.2;
        cloud[50][j].z = j * 0.3;
    }
    
    /* Test count = 8 (larger than 2, TYPE_SIZE * count calculation) */
    volatile int lo2 = 30;
    volatile int hi2 = 37;  /* count = 8, element size = 24 bytes, total = 192 bytes */
    
    for (int j = lo2; j <= hi2; ++j) {
        cloud[51][j] = cloud[50][j - 20];  /* Struct copy */
    }
    
    use_ptr(&cloud[51][lo2]);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Execute all tests to trigger different paths */
    test_int_array_slice();
    checksum += 1;
    
    test_small_slice();
    checksum += 2;
    
    test_char_array();
    checksum += 3;
    
    test_single_element();
    checksum += 4;
    
    test_vla_constant_size();
    checksum += 5;
    
    test_mixed_counts();
    checksum += 6;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

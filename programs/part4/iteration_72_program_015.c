/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <string.h>

/* Opaque functions to prevent premature optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_const_two(void) { return 2; }
static int __attribute__((noinline, noipa)) get_const_large(void) { return 50; }
static void __attribute__((noinline, noipa)) sink_int(int val) { asm volatile("" : : "r"(val)); }
static void __attribute__((noinline, noipa)) sink_double(double val) { asm volatile("" : : "r"(val)); }
static void __attribute__((noinline, noipa)) sink_char(char val) { asm volatile("" : : "r"(val)); }

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice_int(void) {
    int arr[100][50];
    volatile int start = 5;  /* Force middle-end analysis */
    int lo = start;          /* Should become constant 5 */
    int hi = lo + 1;         /* Should become constant 6, count = 2 */
    
    /* Write to slice (lvalue context) */
    for (int j = lo; j <= hi; ++j) {
        arr[20][j] = j * 2;
    }
    
    /* Read from slice (rvalue context) */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[20][j];
    }
    sink_int(sum);
    
    /* Another pattern with count = 1 */
    volatile int single = 8;
    int idx = single;
    arr[30][idx] = 42;
    sink_int(arr[30][idx]);
}

/* Test 2: Double array with larger count (>2) */
static void __attribute__((noinline)) test_large_slice_double(void) {
    double matrix[10][20];
    volatile int base = 3;
    int lo = base;           /* Constant 3 */
    int hi = lo + 9;         /* Constant 12, count = 10 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = j * 1.5;
    }
    
    /* Copy slice to another location */
    double copy[10];
    for (int j = lo; j <= hi; ++j) {
        copy[j - lo] = matrix[5][j];
    }
    
    /* Use results */
    double total = 0.0;
    for (int i = 0; i < 10; ++i) {
        total += copy[i];
    }
    sink_double(total);
}

/* Test 3: Char array with varying element sizes */
static void __attribute__((noinline)) test_char_array_mixed(void) {
    char buffer[100][80];
    volatile int low = 10;
    volatile int high = 15;
    int lo = low;    /* Constant 10 */
    int hi = high;   /* Constant 15, count = 6 */
    
    /* Mixed operations on slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[25][j] = 'A' + (j - lo);
    }
    
    /* Read and transform */
    char temp[6];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = buffer[25][j] + 1;
    }
    
    /* Use in expression */
    char check = 0;
    for (int i = 0; i < 6; ++i) {
        check ^= temp[i];
    }
    sink_char(check);
}

/* Test 4: VLA with constant size (affects MEM_P analysis) */
static void __attribute__((noinline)) test_vla_constant_bounds(void) {
    int n = 30;  /* Constant but VLA declaration */
    int vla[n][n];
    volatile int vlo = 5;
    volatile int vhi = 8;
    int lo = vlo;    /* Constant 5 */
    int hi = vhi;    /* Constant 8, count = 4 */
    
    /* Access VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = j * 3;
    }
    
    /* Read back */
    int vsum = 0;
    for (int j = lo; j <= hi; ++j) {
        vsum += vla[15][j];
    }
    sink_int(vsum);
}

/* Test 5: Multi-dimensional with very small type (1 byte) */
static void __attribute__((noinline)) test_tiny_elements(void) {
    unsigned char tiny[100][100];
    volatile int tlo = 20;
    volatile int thi = 25;
    int lo = tlo;    /* Constant 20 */
    int hi = thi;    /* Constant 25, count = 6 */
    
    /* Fill slice */
    for (int j = lo; j <= hi; ++j) {
        tiny[50][j] = (j * 7) & 0xFF;
    }
    
    /* Copy to int array (type conversion) */
    int ints[6];
    for (int j = lo; j <= hi; ++j) {
        ints[j - lo] = tiny[50][j];
    }
    
    int tsum = 0;
    for (int i = 0; i < 6; ++i) {
        tsum += ints[i];
    }
    sink_int(tsum);
}

/* Test 6: Exactly count = 2 case with different element type */
static void __attribute__((noinline)) test_exactly_two(void) {
    long long big[50][40];
    volatile int two_low = 18;
    int lo = two_low;        /* Constant 18 */
    int hi = lo + 1;         /* Constant 19, count = 2 */
    
    /* Two-element slice operation */
    big[25][lo] = 0x12345678;
    big[25][hi] = 0x9ABCDEF0;
    
    long long pair_sum = big[25][lo] + big[25][hi];
    sink_int(pair_sum & 0xFFFFFFFF);
}

/* Test 7: Mixed bounds from function calls (still constant) */
static void __attribute__((noinline)) test_function_bounds(void) {
    float data[60][70];
    
    /* Get bounds from opaque functions */
    int lo = get_const_low();    /* Should be analyzed as constant 5 */
    int hi = get_const_high();   /* Should be analyzed as constant 10 */
    /* count = 6 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        data[30][j] = j * 0.5f;
    }
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int j = lo; j <= hi; ++j) {
        checksum += data[30][j];
    }
    
    /* Force use */
    int int_checksum = (int)checksum;
    sink_int(int_checksum);
}

int main(void) {
    volatile int result = 0;
    
    /* Execute all tests */
    test_small_slice_int();
    result += 1;
    
    test_large_slice_double();
    result += 2;
    
    test_char_array_mixed();
    result += 3;
    
    test_vla_constant_bounds();
    result += 4;
    
    test_tiny_elements();
    result += 5;
    
    test_exactly_two();
    result += 6;
    
    test_function_bounds();
    result += 7;
    
    /* Print checksum to prevent optimization */
    printf("Test result checksum: %d\n", result);
    
    return 0;
}

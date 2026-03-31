/* Test program for expr.cc lines 7691-7700 */
#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 15; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) sink_int(int val) { (void)val; }
static void __attribute__((noinline, noipa)) sink_double(double val) { (void)val; }
static void __attribute__((noinline, noipa)) sink_char(char val) { (void)val; }

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[100][50];
    volatile int start = 5;  /* Force middle-end analysis */
    int lo = start;          /* lo = 5 */
    int hi = lo + 1;         /* hi = 6, count = 2 */
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[10][j] = j * 2;  /* Constant bounds: 5..6 */
    }
    
    /* Load context (rvalue) */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[10][j];   /* Access same slice */
    }
    sink_int(sum);
}

/* Test 2: Larger slice with count > 2, int type */
static void __attribute__((noinline)) test_large_int_slice(void) {
    int grid[100][50];
    volatile int low = get_const_low();   /* 5 */
    volatile int high = get_const_high(); /* 15 */
    int lo = low;
    int hi = high;
    /* count = 15 - 5 + 1 = 11 > 2 */
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        grid[20][j] = j * 3;
    }
    
    /* Read from slice */
    volatile int total = 0;
    for (int j = lo; j <= hi; ++j) {
        total += grid[20][j];
    }
    sink_int(total);
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[10][20];
    volatile int low = 2;
    volatile int high = 8;  /* count = 7 > 2 */
    int lo = low;
    int hi = high;
    
    /* Mixed store/load pattern */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = j * 1.5;          /* Store */
        double val = matrix[5][j] * 2.0; /* Load */
        sink_double(val);
    }
}

/* Test 4: Char array with single element (count = 1) */
static void __attribute__((noinline)) test_char_single(void) {
    char buffer[100][80];
    volatile int idx = 42;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Store single element */
    buffer[30][lo] = 'A';
    
    /* Load single element */
    char c = buffer[30][hi];
    sink_char(c);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_slice(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];     /* VLA with constant dimensions */
    
    volatile int low = 10;
    volatile int high = 19;  /* count = 10 > 2 */
    int lo = low;
    int hi = high;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = j * 4;
    }
    
    /* Process slice */
    volatile int check = 0;
    for (int j = lo; j <= hi; ++j) {
        check ^= vla[15][j];  /* XOR to prevent elimination */
    }
    sink_int(check);
}

/* Test 6: Mixed access patterns in same function */
static void __attribute__((noinline)) test_mixed_patterns(void) {
    int data[50][40];
    
    /* Pattern A: count = 2 */
    {
        volatile int a_low = 8;
        volatile int a_high = 9;
        for (int j = a_low; j <= a_high; ++j) {
            data[10][j] = data[10][j] + 1;  /* Both load and store */
        }
    }
    
    /* Pattern B: count = 12 */
    {
        volatile int b_low = 20;
        volatile int b_high = 31;
        for (int j = b_low; j <= b_high; ++j) {
            data[25][j] = j * 7;
        }
        
        volatile int sum = 0;
        for (int j = b_low; j <= b_high; ++j) {
            sum += data[25][j];
        }
        sink_int(sum);
    }
}

/* Test 7: Struct array to test different elttype sizes */
struct medium_struct {
    int a;
    int b;
    char c[8];
};

static void __attribute__((noinline)) test_struct_slice(void) {
    struct medium_struct table[20][30];
    
    volatile int low = 5;
    volatile int high = 14;  /* count = 10 > 2, TYPE_SIZE larger */
    int lo = low;
    int hi = high;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        table[10][j].a = j;
        table[10][j].b = j * 2;
    }
    
    /* Access slice */
    volatile int result = 0;
    for (int j = lo; j <= hi; ++j) {
        result += table[10][j].a + table[10][j].b;
    }
    sink_int(result);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Execute all tests */
    test_small_slice();
    checksum += 1;
    
    test_large_int_slice();
    checksum += 2;
    
    test_double_slice();
    checksum += 3;
    
    test_char_single();
    checksum += 4;
    
    test_vla_slice();
    checksum += 5;
    
    test_mixed_patterns();
    checksum += 6;
    
    test_struct_slice();
    checksum += 7;
    
    /* Print checksum to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

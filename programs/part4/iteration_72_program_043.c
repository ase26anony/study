/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_const_two(void) { return 2; }
static int __attribute__((noinline, noipa)) get_const_large(void) { return 30; }
static void __attribute__((noinline, noipa)) use_int(int val) { asm volatile("" : : "r"(val) : "memory"); }
static void __attribute__((noinline, noipa)) use_double(double val) { asm volatile("" : : "r"(&val) : "memory"); }

/* Test 1: Multi-dimensional int array with count <= 2 */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[100][50];
    
    /* Force constant bounds through volatile */
    volatile int const_low = 5;
    volatile int const_high = 6;  /* count = 2 */
    int lo = const_low;
    int hi = const_high;
    
    /* Both lvalue (store) and rvalue (load) contexts */
    for (int j = lo; j <= hi; ++j) {
        /* Store context - lvalue */
        arr[20][j] = j * 2;
        
        /* Load context - rvalue */
        int val = arr[20][j];
        use_int(val);
    }
    
    /* Another pattern with count = 1 */
    volatile int single_low = 7;
    volatile int single_high = 7;  /* count = 1 */
    int slo = single_low;
    int shi = single_high;
    
    for (int j = slo; j <= shi; ++j) {
        arr[30][j] = j * 3;
        use_int(arr[30][j]);
    }
}

/* Test 2: Larger slice with count > 2, small element size (char) */
static void __attribute__((noinline)) test_char_slice(void) {
    char grid[200][100];
    
    volatile int start = 10;
    volatile int end = 25;  /* count = 16 > 2 */
    int lo = start;
    int hi = end;
    
    /* Mixed operations on char slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store to slice */
        grid[50][j] = (char)(j & 0xFF);
        
        /* Read from slice */
        char c = grid[50][j];
        use_int((int)c);
    }
}

/* Test 3: Double array with count > 2, larger element size */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[40][60];
    
    volatile int d_low = 3;
    volatile int d_high = 15;  /* count = 13 > 2, TYPE_SIZE(double) * 13 */
    int lo = d_low;
    int hi = d_high;
    
    /* Operations on double slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store context */
        matrix[10][j] = j * 1.5;
        
        /* Load context */
        double d = matrix[10][j];
        use_double(d);
    }
}

/* Test 4: Variable Length Array with constant size expression */
static void __attribute__((noinline)) test_vla_slice(void) {
    int n = 30;  /* Constant but not literal */
    int vla[n][n];
    
    volatile int v_low = 2;
    volatile int v_high = 8;  /* count = 7 > 2 */
    int lo = v_low;
    int hi = v_high;
    
    /* Both directions of copy */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = j * 4;
        use_int(vla[15][j]);
    }
    
    /* Another slice with different bounds */
    volatile int v_low2 = 20;
    volatile int v_high2 = 29;  /* count = 10 > 2 */
    int lo2 = v_low2;
    int hi2 = v_high2;
    
    int temp[10];
    for (int j = lo2; j <= hi2; ++j) {
        /* Load from VLA slice */
        temp[j - lo2] = vla[15][j];
    }
    
    /* Use temp to prevent elimination */
    for (int j = 0; j < 10; ++j) {
        use_int(temp[j]);
    }
}

/* Test 5: Mixed operations triggering different paths */
static void __attribute__((noinline)) test_mixed_operations(void) {
    int data[80][40];
    
    /* Pattern 1: count = 2 */
    {
        volatile int low = 35;
        volatile int high = 36;
        int lo = low;
        int hi = high;
        
        for (int j = lo; j <= hi; ++j) {
            data[25][j] = data[25][j] + 1;
            use_int(data[25][j]);
        }
    }
    
    /* Pattern 2: count = 3 (just over threshold) */
    {
        volatile int low = 10;
        volatile int high = 12;
        int lo = low;
        int hi = high;
        
        for (int j = lo; j <= hi; ++j) {
            data[60][j] = j * 7;
        }
        
        /* Read back in separate loop */
        for (int j = lo; j <= hi; ++j) {
            use_int(data[60][j]);
        }
    }
    
    /* Pattern 3: Larger count with different element type */
    {
        short shorts[50][100];
        volatile int low = 20;
        volatile int high = 45;  /* count = 26 > 2 */
        int lo = low;
        int hi = high;
        
        for (int j = lo; j <= hi; ++j) {
            shorts[30][j] = (short)(j * 100);
            use_int((int)shorts[30][j]);
        }
    }
}

/* Test 6: Complex index expressions that simplify to constants */
static void __attribute__((noinline)) test_complex_indices(void) {
    long big[100][200];
    
    /* Complex but constant bounds */
    volatile int base = 100;
    volatile int offset = 5;
    int lo = base - offset;      /* = 95 */
    int hi = base + offset - 1;  /* = 104, count = 10 > 2 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        big[50][j] = j * 1000L;
    }
    
    /* Copy slice to another location */
    long copy[10];
    for (int j = lo; j <= hi; ++j) {
        copy[j - lo] = big[50][j];
        use_int((int)(copy[j - lo] & 0xFFFFFFFF));
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Execute all tests */
    test_small_slice();
    checksum += 1;
    
    test_char_slice();
    checksum += 2;
    
    test_double_slice();
    checksum += 3;
    
    test_vla_slice();
    checksum += 4;
    
    test_mixed_operations();
    checksum += 5;
    
    test_complex_indices();
    checksum += 6;
    
    /* Print checksum to ensure execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* expr_coverage.c - Program to trigger constant bounds array slice logic in GCC's expr.cc */

#include <stdio.h>
#include <stddef.h>

/* Dummy opaque functions to prevent early optimization */
static int __attribute__((noinline, noipa)) get_const_low(void) { return 5; }
static int __attribute__((noinline, noipa)) get_const_high(void) { return 10; }
static int __attribute__((noinline, noipa)) get_small_high(void) { return 6; }
static void __attribute__((noinline, noipa)) sink_int(int val) { 
    volatile static int sink; 
    sink = val; 
}
static void __attribute__((noinline, noipa)) sink_double(double val) { 
    volatile static double sink; 
    sink = val; 
}
static void __attribute__((noinline, noipa)) sink_char(char val) { 
    volatile static char sink; 
    sink = val; 
}

/* Test 1: Multi-dimensional int array with count > 2 */
static void __attribute__((noinline)) test_int_array_slice(void) {
    int arr[100][50];
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Use volatile to force middle-end analysis */
    volatile int const_low = get_const_low();  /* Should be 5 */
    volatile int const_high = get_const_high(); /* Should be 10 */
    
    int lo = const_low;  /* 5 */
    int hi = const_high; /* 10 */
    
    /* Access slice: count = 10 - 5 + 1 = 6 > 2 */
    /* This should trigger: count > 2 && MEM_P(target) check */
    for (int row = 0; row < 3; ++row) {
        /* Write to slice (lvalue context) */
        for (int j = lo; j <= hi; ++j) {
            arr[row][j] = row * 1000 + j;
        }
        
        /* Read from slice (rvalue context) */
        int sum = 0;
        for (int j = lo; j <= hi; ++j) {
            sum += arr[row][j];
        }
        sink_int(sum);
    }
}

/* Test 2: Small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    double matrix[20][30];
    
    /* Initialize */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 30; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    volatile int start = 8;
    volatile int end = get_small_high();  /* Returns 6, but we'll adjust */
    
    int lo = start;      /* 8 */
    int hi = start + 1;  /* 9, count = 2 */
    
    /* This should trigger: count <= 2 branch */
    for (int i = 5; i < 8; ++i) {
        /* Mixed lvalue/rvalue accesses */
        matrix[i][lo] = matrix[i-1][hi] * 2.0;
        matrix[i][hi] = matrix[i][lo] + 1.0;
        
        sink_double(matrix[i][lo]);
        sink_double(matrix[i][hi]);
    }
    
    /* Another case: count = 1 */
    int single_idx = 15;
    for (int i = 0; i < 5; ++i) {
        matrix[i][single_idx] = i * 3.14;
        double val = matrix[i][single_idx];
        sink_double(val);
    }
}

/* Test 3: Char array with different element size */
static void __attribute__((noinline)) test_char_array(void) {
    char buffer[256][128];
    
    /* Initialize */
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 128; ++j) {
            buffer[i][j] = (i + j) & 0xFF;
        }
    }
    
    volatile int low = 32;
    volatile int high = 95;  /* count = 64 */
    
    int lo = low;   /* 32 */
    int hi = high;  /* 95 */
    
    /* Large slice of chars: TYPE_SIZE = 8 bits, count = 64, total = 512 bits */
    for (int row = 0; row < 10; ++row) {
        /* Write operation */
        for (int j = lo; j <= hi; ++j) {
            buffer[row][j] = (row * j) & 0xFF;
        }
        
        /* Read operation */
        char checksum = 0;
        for (int j = lo; j <= hi; ++j) {
            checksum ^= buffer[row][j];
        }
        sink_char(checksum);
    }
}

/* Test 4: VLA with constant size (still analyzed as VLA) */
static void __attribute__((noinline)) test_vla_constant_size(void) {
    const int n = 40;  /* Constant but VLA declaration */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    volatile int vla_low = 10;
    volatile int vla_high = 25;  /* count = 16 */
    
    int lo = vla_low;
    int hi = vla_high;
    
    /* Access slice in VLA */
    for (int i = 0; i < 5; ++i) {
        /* Write */
        for (int j = lo; j <= hi; ++j) {
            vla[i][j] = (i << 16) | j;
        }
        
        /* Read */
        int sum = 0;
        for (int j = lo; j <= hi; ++j) {
            sum += vla[i][j];
        }
        sink_int(sum);
    }
}

/* Test 5: Mixed types and access patterns */
static void __attribute__((noinline)) test_mixed_patterns(void) {
    struct Mixed {
        int a[10][20];
        double b[5][15];
        char c[30][40];
    } m;
    
    /* Initialize */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            m.a[i][j] = i * 20 + j;
        }
    }
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 15; ++j) {
            m.b[i][j] = i * 1.1 + j * 0.1;
        }
    }
    
    volatile int low1 = 3;
    volatile int high1 = 7;   /* count = 5 */
    volatile int low2 = 10;
    volatile int high2 = 11;  /* count = 2 */
    
    /* Multiple slices with different counts */
    for (int i = 0; i < 3; ++i) {
        /* Count = 5 > 2 */
        for (int j = low1; j <= high1; ++j) {
            m.a[i][j] = m.a[i][j] * 2;
        }
        
        /* Count = 2 */
        for (int j = low2; j <= high2; ++j) {
            m.b[i][j] = m.b[i][j] + 1.0;
        }
        
        sink_int(m.a[i][low1]);
        sink_double(m.b[i][low2]);
    }
}

int main(void) {
    volatile int result = 0;
    
    printf("Testing array slice constant bounds logic...\n");
    
    /* Run all tests */
    test_int_array_slice();
    test_small_slice();
    test_char_array();
    test_vla_constant_size();
    test_mixed_patterns();
    
    /* Force execution and prevent dead code elimination */
    result = 1;
    
    printf("Tests completed. Result: %d\n", result);
    
    return 0;
}

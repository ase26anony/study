/* test_expr_coverage.c */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent premature optimization */
static int __attribute__((noinline, noipa)) use_int(int val) {
    volatile int sink = val;
    return sink;
}

static void __attribute__((noinline, noipa)) use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Mixed lvalue/rvalue contexts */
    for (int i = 0; i < 10; i++) {
        /* Store context (lvalue) */
        for (int j = lo; j <= hi; j++) {
            arr[i][j] = i * 100 + j;
        }
        
        /* Load context (rvalue) */
        volatile int sum = 0;
        for (int j = lo; j <= hi; j++) {
            sum += arr[i][j];
        }
        use_int(sum);
    }
    
    /* Force analysis with volatile index */
    volatile int start = 3;
    int lo2 = start;  /* start is constant 3, but volatile forces middle-end analysis */
    int hi2 = 4;      /* count = 2 */
    
    /* Another access pattern */
    for (int j = lo2; j <= hi2; j++) {
        arr[0][j] = use_int(arr[1][j]);
    }
}

/* Test 2: Larger slice with int elements (count > 2) */
static void __attribute__((noinline)) test_large_int_slice(void) {
    int matrix[50][100] = {0};
    
    /* Constant bounds for larger slice */
    const int lo = 10;
    const int hi = 25;  /* count = 16 */
    
    /* Store operation on slice */
    for (int i = 0; i < 50; i++) {
        for (int j = lo; j <= hi; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Load operation from slice */
    volatile long total = 0;
    for (int i = 20; i < 30; i++) {
        for (int j = lo; j <= hi; j++) {
            total += matrix[i][j];
        }
    }
    use_int(total);
    
    /* Different element size variation */
    {
        int sub[5][15] = {0};
        const int slo = 2;
        const int shi = 12;  /* count = 11 */
        
        /* Both directions */
        for (int i = 0; i < 5; i++) {
            /* Write to slice */
            for (int j = slo; j <= shi; j++) {
                sub[i][j] = i + j;
            }
            
            /* Read from slice */
            volatile int row_sum = 0;
            for (int j = slo; j <= shi; j++) {
                row_sum += sub[i][j];
            }
            use_int(row_sum);
        }
    }
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline)) test_double_slice(void) {
    double grid[20][30];
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            grid[i][j] = i * 0.5 + j * 0.3;
        }
    }
    
    /* Constant bounds */
    const int lo = 5;
    const int hi = 15;  /* count = 11 */
    
    /* Process slice */
    volatile double accum = 0.0;
    for (int i = 5; i < 15; i++) {
        /* Read slice */
        for (int j = lo; j <= hi; j++) {
            accum += grid[i][j];
        }
        
        /* Write to slice */
        for (int j = lo; j <= hi; j++) {
            grid[i][j] = accum * 0.1;
        }
    }
    
    /* Small slice within the same array */
    {
        const int slo = 25;
        const int shi = 26;  /* count = 2 */
        
        for (int i = 0; i < 5; i++) {
            grid[i][slo] = grid[i][shi] * 2.0;
        }
    }
}

/* Test 4: Char array with different element size */
static void __attribute__((noinline)) test_char_slice(void) {
    char buffer[100][80];
    
    /* Fill with pattern */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 80; j++) {
            buffer[i][j] = (i + j) & 0x7F;
        }
    }
    
    /* Multiple slice sizes */
    const int lo1 = 10;
    const int hi1 = 11;  /* count = 2 */
    
    const int lo2 = 20;
    const int hi2 = 40;  /* count = 21 */
    
    /* Process both slices */
    volatile int checksum = 0;
    
    /* Small slice */
    for (int i = 0; i < 50; i++) {
        for (int j = lo1; j <= hi1; j++) {
            checksum += buffer[i][j];
            buffer[i][j] = checksum & 0x7F;
        }
    }
    
    /* Large slice */
    for (int i = 50; i < 100; i++) {
        for (int j = lo2; j <= hi2; j++) {
            checksum += buffer[i][j];
            buffer[i][j] = (checksum * 3) & 0x7F;
        }
    }
    
    use_int(checksum);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_slice(void) {
    const int n = 40;  /* Constant size */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds */
    const int lo = 15;
    const int hi = 35;  /* count = 21 */
    
    /* Process slice - both read and write */
    volatile long sum = 0;
    
    for (int i = 10; i < 30; i++) {
        /* Read from slice */
        for (int j = lo; j <= hi; j++) {
            sum += vla[i][j];
        }
        
        /* Write to slice */
        for (int j = lo; j <= hi; j++) {
            vla[i][j] = (sum + i + j) & 0xFFF;
        }
    }
    
    /* Small slice within VLA */
    {
        const int slo = 5;
        const int shi = 6;  /* count = 2 */
        
        for (int i = 0; i < 10; i++) {
            vla[i][slo] = vla[i][shi] * 2;
        }
    }
    
    use_int(sum);
}

/* Test 6: Mixed operations with volatile bounds */
static void __attribute__((noinline)) test_mixed_volatile_bounds(void) {
    int data[60][70] = {0};
    
    /* Volatile but constant values */
    volatile int vlo = 25;
    volatile int vhi = 35;
    
    int lo = vlo;  /* 25 */
    int hi = vhi;  /* 35 - count = 11 */
    
    /* Initialize slice */
    for (int i = 0; i < 60; i++) {
        for (int j = lo; j <= hi; j++) {
            data[i][j] = i * 1000 + j;
        }
    }
    
    /* Copy between slices */
    for (int i = 0; i < 30; i++) {
        /* Source slice (rvalue) */
        volatile int row_sum = 0;
        for (int j = lo; j <= hi; j++) {
            row_sum += data[i][j];
        }
        
        /* Destination slice (lvalue) */
        for (int j = lo; j <= hi; j++) {
            data[i + 30][j] = row_sum + j;
        }
    }
    
    /* Very small slice */
    {
        const int tiny_lo = 60;
        const int tiny_hi = 60;  /* count = 1 */
        
        for (int i = 0; i < 60; i++) {
            data[i][tiny_lo] = use_int(data[i][tiny_lo]);
        }
    }
}

int main(void) {
    volatile int result = 0;
    
    printf("Starting array slice tests...\n");
    
    /* Run all tests */
    test_small_slice();
    result += 1;
    
    test_large_int_slice();
    result += 2;
    
    test_double_slice();
    result += 3;
    
    test_char_slice();
    result += 4;
    
    test_vla_slice();
    result += 5;
    
    test_mixed_volatile_bounds();
    result += 6;
    
    printf("Tests completed. Result: %d\n", result);
    
    return result != 21 ? 1 : 0;
}

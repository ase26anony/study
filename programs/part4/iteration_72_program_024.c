/* Test program for expr.cc lines 7691-7700 - constant bounds array slicing */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile int sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 2;
    }
    
    /* Load context (rvalue) */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] + 1;
    use_int(arr[3][lo]);
}

/* Test 2: Multi-dimensional double array with medium slice (count > 2) */
static void __attribute__((noinline))
test_medium_slice_double(void) {
    double matrix[15][25] = {0};
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Read from slice */
    double total = 0.0;
    for (int j = lo; j <= hi; ++j) {
        total += matrix[7][j];
    }
    volatile double sink = total;
    
    /* Cross-slice copy - triggers MEM_P analysis */
    for (int j = lo; j <= hi; ++j) {
        matrix[8][j] = matrix[7][j];
    }
}

/* Test 3: Char array with single element slice (count = 1) */
static void __attribute__((noinline))
test_single_char_slice(void) {
    char buffer[100][50];
    volatile int idx = 42;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            buffer[i][j] = i + j;
        }
    }
    
    /* Single element access in both contexts */
    buffer[30][lo] = 'X';           /* Store */
    char val = buffer[30][lo];      /* Load */
    use_int((int)val);
    
    /* Small range copy (2 elements) */
    if (lo < 49) {
        buffer[31][lo] = buffer[30][lo];
        buffer[31][lo+1] = buffer[30][lo+1];
    }
}

/* Test 4: VLA with constant size (affects MEM_P analysis) */
static void __attribute__((noinline))
test_vla_constant_slice(void) {
    const int n = 40;
    int vla[n][n];
    volatile int start = 15;
    volatile int end = 24;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Slice operations on VLA */
    int row = 20;
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; ++j) {
        vla[row][j] = (j - lo) * 100;
    }
    
    /* Read from VLA slice */
    int vla_sum = 0;
    for (int j = lo; j <= hi; ++j) {
        vla_sum += vla[row][j];
    }
    use_int(vla_sum);
    
    /* Copy between VLA slices */
    for (int j = lo; j <= hi; ++j) {
        vla[row+1][j] = vla[row][j];
    }
}

/* Test 5: Mixed types and slice sizes in one function */
static void __attribute__((noinline))
test_mixed_slices(void) {
    struct Mixed {
        int a[5][10];
        short b[8][12];
        char c[3][15];
    } m;
    
    /* Test int slice with count = 3 */
    {
        volatile int lo = 2;
        volatile int hi = 4;  /* count = 3 */
        int l = lo;
        int h = hi;
        
        /* Both store and load */
        for (int j = l; j <= h; ++j) {
            m.a[2][j] = j * 10;
        }
        
        int sum = 0;
        for (int j = l; j <= h; ++j) {
            sum += m.a[2][j];
        }
        use_int(sum);
    }
    
    /* Test short slice with count = 2 */
    {
        volatile int lo = 5;
        volatile int hi = 6;  /* count = 2 */
        int l = lo;
        int h = hi;
        
        m.b[4][l] = 100;
        m.b[4][h] = 200;
        
        short result = (short)(m.b[4][l] + m.b[4][h]);
        use_int((int)result);
    }
    
    /* Test char slice with count = 8 */
    {
        volatile int lo = 1;
        volatile int hi = 8;  /* count = 8 */
        int l = lo;
        int h = hi;
        
        for (int j = l; j <= h; ++j) {
            m.c[1][j] = (char)(j * 3);
        }
        
        char check = 0;
        for (int j = l; j <= h; ++j) {
            check += m.c[1][j];
        }
        use_int((int)check);
    }
}

/* Test 6: Complex index expressions that simplify to constants */
static void __attribute__((noinline))
test_constant_index_exprs(void) {
    int grid[50][30];
    
    /* These should become constants after optimization */
    volatile int base = 10;
    int lo = base + 2;      /* 12 */
    int hi = base + 5;      /* 15, count = 4 */
    
    /* Initialize */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 30; ++j) {
            grid[i][j] = i * 30 + j;
        }
    }
    
    /* Slice copy operation */
    int src_row = 20;
    int dst_row = 21;
    
    /* This should trigger the constant bounds logic */
    for (int j = lo; j <= hi; ++j) {
        grid[dst_row][j] = grid[src_row][j];
    }
    
    /* Verify */
    int verify = 0;
    for (int j = lo; j <= hi; ++j) {
        verify += grid[dst_row][j];
    }
    use_int(verify);
}

int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_medium_slice_double();
    checksum += 2;
    
    test_single_char_slice();
    checksum += 3;
    
    test_vla_constant_slice();
    checksum += 4;
    
    test_mixed_slices();
    checksum += 5;
    
    test_constant_index_exprs();
    checksum += 6;
    
    /* Final sink to prevent optimization */
    volatile int final_sink = checksum;
    printf("Test completed with checksum: %d\n", final_sink);
    
    return 0;
}

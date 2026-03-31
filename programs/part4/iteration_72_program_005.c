/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Dummy functions to prevent optimization */
static void __attribute__((noinline, noipa)) use_int(int x) {
    volatile static int sink;
    sink = x;
}

static void __attribute__((noinline, noipa)) use_double(double x) {
    volatile static double sink;
    sink = x;
}

static void __attribute__((noinline, noipa)) use_ptr(void *p) {
    volatile static void *sink;
    sink = p;
}

/* Test 1: Small count (<= 2) with int array */
static void __attribute__((noinline)) test_small_count_int(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int start = 5;
    volatile int end = 6;  /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice - rvalue context */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = arr[3][j];
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice - lvalue context */
    for (int j = lo; j <= hi; j++) {
        arr[3][j] = temp[j - lo] * 2;
    }
    
    /* Verify */
    for (int j = lo; j <= hi; j++) {
        use_int(arr[3][j]);
    }
}

/* Test 2: Larger count (> 2) with char array */
static void __attribute__((noinline)) test_large_count_char(void) {
    char arr[50][100];
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = (char)(i + j);
        }
    }
    
    /* Constant bounds - count = 10 */
    volatile int start = 20;
    volatile int end = 29;
    int lo = start;
    int hi = end;
    
    char buffer[10];
    
    /* Load slice - rvalue */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = arr[25][j];
        use_int(arr[25][j]);
    }
    
    /* Store slice - lvalue */
    for (int j = lo; j <= hi; j++) {
        arr[25][j] = buffer[j - lo] + 1;
    }
    
    /* Mixed access pattern */
    for (int j = lo; j <= hi; j += 2) {
        arr[25][j] = arr[25][j + 1];  /* Both lvalue and rvalue */
    }
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline)) test_double_array(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 25; j++) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Test count = 1 */
    volatile int start1 = 10;
    volatile int end1 = 10;  /* count = 1 */
    int lo1 = start1;
    int hi1 = end1;
    
    /* Single element access - both contexts */
    double val = matrix[5][lo1];  /* rvalue */
    use_double(val);
    matrix[5][lo1] = val * 2.0;   /* lvalue */
    
    /* Test count = 5 */
    volatile int start2 = 5;
    volatile int end2 = 9;  /* count = 5 */
    int lo2 = start2;
    int hi2 = end2;
    
    double temp[5];
    
    /* Block operations */
    for (int j = lo2; j <= hi2; j++) {
        temp[j - lo2] = matrix[10][j];
    }
    
    for (int j = lo2; j <= hi2; j++) {
        matrix[10][j] = temp[j - lo2] * 3.0;
        use_double(matrix[10][j]);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_constant_size(void) {
    const int n = 30;
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds through volatile */
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Access row slice */
    int row = 15;
    int buffer[10];
    
    /* Load from VLA slice */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = vla[row][j];
        use_int(vla[row][j]);
    }
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; j++) {
        vla[row][j] = buffer[j - lo] * 2;
    }
    
    /* Cross-slice copy */
    for (int j = lo; j <= hi; j++) {
        vla[row + 1][j] = vla[row][j];  /* rvalue and lvalue */
    }
}

/* Test 5: Mixed types and complex index calculations */
static void __attribute__((noinline)) test_mixed_types(void) {
    struct Mixed {
        char c;
        int i;
        double d;
    } arr[20][15];
    
    /* Initialize */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 15; j++) {
            arr[i][j].c = (char)(i + j);
            arr[i][j].i = i * 100 + j;
            arr[i][j].d = i * 2.5 + j * 0.5;
        }
    }
    
    /* Different count scenarios */
    volatile int bounds[][2] = {{1, 2}, {5, 14}};  /* count=2 and count=10 */
    
    for (int b = 0; b < 2; b++) {
        volatile int start = bounds[b][0];
        volatile int end = bounds[b][1];
        int lo = start;
        int hi = end;
        int count = hi - lo + 1;
        
        /* Access struct members in slice */
        for (int j = lo; j <= hi; j++) {
            /* Load operations */
            char c_val = arr[10][j].c;
            int i_val = arr[10][j].i;
            double d_val = arr[10][j].d;
            
            use_int(c_val);
            use_int(i_val);
            use_double(d_val);
            
            /* Store operations */
            arr[10][j].c = c_val + 1;
            arr[10][j].i = i_val * 2;
            arr[10][j].d = d_val * 1.5;
        }
    }
}

/* Test 6: Pointer-based access with constant offsets */
static void __attribute__((noinline)) test_pointer_arithmetic(void) {
    int grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            grid[i][j] = i * 50 + j;
        }
    }
    
    /* Get pointer to row */
    int *row_ptr = &grid[42][0];
    
    /* Constant bounds */
    volatile int low = 10;
    volatile int high = 25;  /* count = 16 */
    int lo = low;
    int hi = high;
    
    /* Access through pointer + constant index */
    int temp[16];
    
    /* Load slice */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = row_ptr[j];
        use_int(row_ptr[j]);
    }
    
    /* Store slice */
    for (int j = lo; j <= hi; j++) {
        row_ptr[j] = temp[j - lo] + 1000;
    }
    
    use_ptr(row_ptr);
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_count_int();
    checksum += 1;
    
    test_large_count_char();
    checksum += 2;
    
    test_double_array();
    checksum += 3;
    
    test_vla_constant_size();
    checksum += 4;
    
    test_mixed_types();
    checksum += 5;
    
    test_pointer_arithmetic();
    checksum += 6;
    
    printf("All tests completed. Checksum: %d\n", checksum);
    
    return 0;
}

/* Test program to trigger constant bounds checking logic in expr.cc */
#include <stdio.h>
#include <string.h>

/* Dummy opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile static int sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile static double sink;
    sink = val;
}

static void __attribute__((noinline, noipa))
use_ptr(void *ptr) {
    volatile static void *sink;
    sink = ptr;
}

/* Test 1: Multi-dimensional int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Write to slice (lvalue context) */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Read from slice (rvalue context) */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* Load from slice */
    }
    for (int j = lo; j <= hi; ++j) {
        arr[4][j] = temp[j - lo];  /* Store to different slice */
    }
}

/* Test 2: Multi-dimensional int array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int grid[100][50] = {0};
    
    /* Constant bounds */
    const int start = 10;
    const int end = 19;  /* count = 10 */
    
    /* Initialize slice */
    for (int j = start; j <= end; ++j) {
        grid[25][j] = j * 100;
    }
    
    /* Copy slice to another location */
    int buffer[10];
    for (int j = start; j <= end; ++j) {
        buffer[j - start] = grid[25][j];  /* Load */
    }
    
    /* Store slice back to different row */
    for (int j = start; j <= end; ++j) {
        grid[26][j] = buffer[j - start];  /* Store */
    }
    
    /* Use volatile to force analysis */
    volatile int check = 0;
    for (int j = start; j <= end; ++j) {
        check += grid[25][j] + grid[26][j];
    }
    use_int(check);
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    
    /* Different slice sizes */
    const int lo1 = 0;
    const int hi1 = 1;   /* count = 2 */
    const int lo2 = 5;
    const int hi2 = 14;  /* count = 10 */
    
    /* Small slice operations */
    for (int j = lo1; j <= hi1; ++j) {
        matrix[0][j] = j * 1.5;
    }
    
    /* Large slice operations */
    double temp[10];
    for (int j = lo2; j <= hi2; ++j) {
        matrix[1][j] = j * 2.5;           /* Store */
        temp[j - lo2] = matrix[1][j];     /* Load */
    }
    
    /* Cross-slice copy */
    for (int j = lo2; j <= hi2; ++j) {
        matrix[2][j] = temp[j - lo2];     /* Store from buffer */
    }
    
    volatile double sum = 0.0;
    for (int j = lo2; j <= hi2; ++j) {
        sum += matrix[1][j] + matrix[2][j];
    }
    use_double(sum);
}

/* Test 4: Char array with byte-sized elements */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[5][100];
    
    /* Constant bounds with volatile wrapper */
    volatile int vlo = 30;
    volatile int vhi = 49;
    const int lo = vlo;  /* Compiler knows these are 30 and 49 */
    const int hi = vhi;  /* count = 20 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        buffer[0][j] = (j % 26) + 'A';
    }
    
    /* Copy slice within array */
    for (int j = lo; j <= hi; ++j) {
        buffer[1][j] = buffer[0][j];  /* Both load and store */
    }
    
    /* Another slice with different bounds */
    const int lo2 = 60;
    const int hi2 = 61;  /* count = 2 */
    for (int j = lo2; j <= hi2; ++j) {
        buffer[2][j] = buffer[1][j + 10];
    }
    
    volatile char check = 0;
    for (int j = lo; j <= hi; ++j) {
        check ^= buffer[0][j] ^ buffer[1][j];
    }
    use_int((int)check);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 30;
    int vla[n][n];  /* VLA with constant size */
    
    /* Constant slice bounds */
    const int row = 10;
    const int lo = 5;
    const int hi = 24;  /* count = 20 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        vla[row][j] = row * 1000 + j;
    }
    
    /* Copy to another row */
    for (int j = lo; j <= hi; ++j) {
        vla[row + 1][j] = vla[row][j] + 1;
    }
    
    /* Small slice within VLA */
    const int lo2 = 0;
    const int hi2 = 1;  /* count = 2 */
    int small[2];
    for (int j = lo2; j <= hi2; ++j) {
        small[j - lo2] = vla[0][j];
        vla[0][j] = small[j - lo2] * 2;
    }
    
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[row][j];
    }
    use_int(sum);
}

/* Test 6: Mixed operations to trigger MEM_P checks */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x, y;
    } points[50][40];
    
    /* Various slice sizes */
    const int bounds[][2] = {
        {10, 10},  /* count = 1 */
        {20, 21},  /* count = 2 */
        {30, 39},  /* count = 10 */
    };
    
    for (int b = 0; b < 3; b++) {
        const int lo = bounds[b][0];
        const int hi = bounds[b][1];
        const int count = hi - lo + 1;
        
        /* Store operation */
        for (int j = lo; j <= hi; ++j) {
            points[b][j].x = j * 10;
            points[b][j].y = j * 20;
        }
        
        /* Load operation to temporary */
        struct Point temp[10];
        for (int j = lo; j <= hi; ++j) {
            temp[j - lo] = points[b][j];
        }
        
        /* Store back to different location */
        for (int j = lo; j <= hi; ++j) {
            points[b + 1][j] = temp[j - lo];
        }
    }
    
    volatile int check = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 40; j++) {
            check += points[i][j].x + points[i][j].y;
        }
    }
    use_int(check);
}

int main(void) {
    printf("Starting array slice tests...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_char_slice();
    test_vla_slice();
    test_mixed_operations();
    
    printf("Tests completed.\n");
    
    /* Force use of all test functions */
    volatile int dummy = 0;
    use_int(dummy);
    
    return 0;
}

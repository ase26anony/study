/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Dummy functions to prevent early optimization */
static void __attribute__((noinline, noipa)) 
use_int(int val) {
    volatile int sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa))
use_double(double val) {
    volatile double sink = val;
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
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - write to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - read from slice */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* Load */
        arr[3][j] = temp[j - lo] * 2;  /* Store */
    }
}

/* Test 2: Multi-dimensional int array with larger slice (count > 2) */
static void __attribute__((noinline))
test_large_slice_int(void) {
    int grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            grid[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds with volatile wrapper to force middle-end analysis */
    volatile int vlo = 10;
    volatile int vhi = 19;
    const int lo = vlo;  /* Compiler knows this is 10 */
    const int hi = vhi;  /* Compiler knows this is 19 */
    /* count = 10, TYPE_SIZE(int) * 10 = 40 bytes typically */
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = j * 1000;
    }
    
    /* Load from slice with volatile consumption */
    volatile int total = 0;
    for (int j = lo; j <= hi; ++j) {
        total += grid[25][j];
    }
    use_int(total);
    
    /* Copy slice to another array */
    int buffer[10];
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = grid[25][j];  /* Load */
    }
    
    /* Copy back modified slice */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = buffer[j - lo] * 3;  /* Store */
    }
}

/* Test 3: Double array with medium slice */
static void __attribute__((noinline))
test_double_slice(void) {
    double matrix[10][20];
    
    /* Constant bounds */
    const int lo = 2;
    const int hi = 5;  /* count = 4 */
    
    /* Initialize */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    /* Both load and store operations */
    double temp[4];
    
    /* Load slice */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = matrix[5][j];
    }
    
    /* Process and store back */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = temp[j - lo] * 2.0;
    }
    
    /* Volatile consumption */
    volatile double check = 0.0;
    for (int j = lo; j <= hi; ++j) {
        check += matrix[5][j];
    }
    use_double(check);
}

/* Test 4: Char array with single element slice (count = 1) */
static void __attribute__((noinline))
test_char_slice(void) {
    char buffer[50][100];
    
    /* Single element - count = 1 */
    const int lo = 42;
    const int hi = 42;
    
    /* Store single char */
    buffer[10][lo] = 'X';
    
    /* Load single char */
    volatile char c = buffer[10][hi];
    use_int((int)c);
    
    /* Two-element slice in another dimension */
    const int lo2 = 30;
    const int hi2 = 31;  /* count = 2 */
    
    /* Store two chars */
    buffer[20][lo2] = 'A';
    buffer[20][hi2] = 'B';
    
    /* Load two chars */
    volatile char c1 = buffer[20][lo2];
    volatile char c2 = buffer[20][hi2];
    use_int((int)c1 + (int)c2);
}

/* Test 5: VLA with constant size (affects MEM_P analysis) */
static void __attribute__((noinline))
test_vla_slice(void) {
    const int n = 30;  /* Constant size VLA */
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice */
    const int lo = 10;
    const int hi = 15;  /* count = 6 */
    
    /* Load slice into temporary */
    int slice[6];
    for (int j = lo; j <= hi; ++j) {
        slice[j - lo] = vla[15][j];
    }
    
    /* Modify and store back */
    for (int j = lo; j <= hi; ++j) {
        vla[15][j] = slice[j - lo] + 1000;
    }
    
    /* Verify with volatile */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[15][j];
    }
    use_int(sum);
}

/* Test 6: Mixed operations to trigger different paths */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Point {
        int x, y;
    } points[20][30];
    
    /* Different slice sizes in same function */
    
    /* Case 1: count = 2 */
    {
        const int lo = 5;
        const int hi = 6;
        
        /* Store */
        for (int j = lo; j <= hi; ++j) {
            points[10][j].x = j * 10;
            points[10][j].y = j * 20;
        }
        
        /* Load */
        volatile int sum_x = 0;
        for (int j = lo; j <= hi; ++j) {
            sum_x += points[10][j].x;
        }
        use_int(sum_x);
    }
    
    /* Case 2: count = 8 (larger than 2) */
    {
        const int lo = 10;
        const int hi = 17;  /* count = 8 */
        
        /* Initialize slice */
        for (int j = lo; j <= hi; ++j) {
            points[5][j].x = 100 + j;
            points[5][j].y = 200 + j;
        }
        
        /* Copy slice */
        struct Point temp[8];
        for (int j = lo; j <= hi; ++j) {
            temp[j - lo] = points[5][j];  /* Load */
        }
        
        /* Modify and store back */
        for (int j = lo; j <= hi; ++j) {
            points[5][j].x = temp[j - lo].y;  /* Store */
            points[5][j].y = temp[j - lo].x;
        }
    }
}

int main(void) {
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all test functions */
    test_small_slice_int();
    test_large_slice_int();
    test_double_slice();
    test_char_slice();
    test_vla_slice();
    test_mixed_operations();
    
    printf("All tests completed\n");
    
    /* Final volatile sink to prevent optimization */
    volatile int result = 42;
    return result;
}

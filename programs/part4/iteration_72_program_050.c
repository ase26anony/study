/* Test program for expr.cc lines 7691-7700 coverage */
#include <stdio.h>
#include <string.h>

/* Opaque functions to prevent early optimization */
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

/* Test 1: 2D int array with count <= 2 */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20] = {0};
    
    /* Force constant bounds through volatile */
    volatile int vlo = 5;
    volatile int vhi = 6;  /* count = 2 */
    int lo = vlo;
    int hi = vhi;
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Another case with count = 1 */
    volatile int vlo2 = 7;
    volatile int vhi2 = 7;  /* count = 1 */
    int lo2 = vlo2;
    int hi2 = vhi2;
    
    for (int j = lo2; j <= hi2; ++j) {
        arr[4][j] = arr[3][j] * 2;
    }
}

/* Test 2: Larger slice with count > 2 */
static void __attribute__((noinline))
test_large_slice_double(void) {
    double matrix[15][25] = {0.0};
    
    /* Constant bounds for larger slice */
    volatile int start = 8;
    volatile int end = 18;  /* count = 11 */
    int lo = start;
    int hi = end;
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[5][j] = (double)j * 1.5;
    }
    
    /* Copy slice to another row (both store and load contexts) */
    for (int j = lo; j <= hi; ++j) {
        matrix[6][j] = matrix[5][j] * 2.0;
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int j = lo; j <= hi; ++j) {
        checksum += matrix[6][j];
    }
    use_double(checksum);
}

/* Test 3: Char array with varying element size */
static void __attribute__((noinline))
test_char_array_slice(void) {
    char buffer[50][100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 100; ++j) {
            buffer[i][j] = (char)((i + j) & 0xFF);
        }
    }
    
    /* Test different slice sizes */
    volatile int bounds[][2] = {
        {10, 11},  /* count = 2 */
        {20, 29},  /* count = 10 */
        {30, 30},  /* count = 1 */
        {40, 55}   /* count = 16 */
    };
    
    for (int b = 0; b < 4; ++b) {
        int lo = bounds[b][0];
        int hi = bounds[b][1];
        
        /* Copy slice between rows */
        for (int j = lo; j <= hi; ++j) {
            buffer[25][j] = buffer[10][j];
        }
        
        /* Process slice */
        char sum = 0;
        for (int j = lo; j <= hi; ++j) {
            sum += buffer[25][j];
        }
        use_int((int)sum);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_bounds(void) {
    /* VLA declared with constant size */
    const int n = 30;
    int vla[n][n];
    
    /* Initialize */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds slice operations */
    volatile int vlo = 5;
    volatile int vhi = 15;  /* count = 11 */
    int lo = vlo;
    int hi = vhi;
    
    /* Store to slice */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = vla[0][j] * 3;
    }
    
    /* Load from slice */
    int total = 0;
    for (int j = lo; j <= hi; ++j) {
        total += vla[10][j];
    }
    use_int(total);
    
    /* Another slice with count = 3 */
    volatile int vlo2 = 20;
    volatile int vhi2 = 22;  /* count = 3 */
    int lo2 = vlo2;
    int hi2 = vhi2;
    
    for (int j = lo2; j <= hi2; ++j) {
        vla[15][j] = vla[10][j] + vla[5][j];
    }
}

/* Test 5: Mixed operations to trigger different paths */
static void __attribute__((noinline))
test_mixed_operations(void) {
    struct Mixed {
        int a[10][20];
        double b[5][15];
        char c[30][40];
    } data;
    
    /* Initialize */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            data.a[i][j] = (i * 20 + j) * 2;
        }
    }
    
    /* Test various constant-bound slices */
    
    /* Case 1: int slice, count = 4 */
    {
        volatile int lo = 3;
        volatile int hi = 6;
        for (int j = lo; j <= hi; ++j) {
            data.a[5][j] = data.a[1][j] + data.a[2][j];
        }
    }
    
    /* Case 2: char slice, count = 8 */
    {
        for (int i = 0; i < 30; ++i) {
            for (int j = 0; j < 40; ++j) {
                data.c[i][j] = (char)((i * j) & 0x7F);
            }
        }
        
        volatile int lo = 10;
        volatile int hi = 17;
        char checksum = 0;
        for (int j = lo; j <= hi; ++j) {
            checksum ^= data.c[15][j];
            data.c[16][j] = data.c[15][j] + 1;
        }
        use_int((int)checksum);
    }
}

/* Test 6: Pointer-based slice operations */
static void __attribute__((noinline))
test_pointer_slices(void) {
    int array[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Get pointer to middle of row - forces MEM_P analysis */
    volatile int offset = 20;
    int *slice_ptr = &array[50][offset];
    
    /* Constant bounds within pointer range */
    volatile int lo = 0;
    volatile int hi = 4;  /* count = 5 */
    
    /* Store through pointer */
    for (int j = lo; j <= hi; ++j) {
        slice_ptr[j] = j * 1000;
    }
    
    /* Load through pointer */
    int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += slice_ptr[j];
    }
    use_int(sum);
    use_ptr(slice_ptr);
}

int main(void) {
    printf("Starting array slice tests...\n");
    
    /* Run all tests */
    test_small_slice_int();
    test_large_slice_double();
    test_char_array_slice();
    test_vla_constant_bounds();
    test_mixed_operations();
    test_pointer_slices();
    
    printf("Tests completed.\n");
    
    /* Force materialization of results */
    volatile int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += i;
    }
    
    return dummy != 0;
}

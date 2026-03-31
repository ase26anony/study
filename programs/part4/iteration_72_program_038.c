/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Dummy opaque functions to prevent early optimization */
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

/* Test 1: Small count (count <= 2) with int array */
static void __attribute__((noinline))
test_small_count_int(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;
    }
    
    /* Load context (rvalue) - reading from slice */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];
    }
    use_int(sum);
    
    /* Mixed access pattern */
    arr[3][lo] = arr[3][hi] + 100;
    use_int(arr[3][lo]);
}

/* Test 2: Larger count with char array (small element size) */
static void __attribute__((noinline))
test_large_count_char(void) {
    char grid[100][50];
    
    /* Constant bounds */
    const int start = 10;
    const int end = 25;  /* count = 16 */
    
    /* Initialize slice */
    for (int j = start; j <= end; ++j) {
        grid[42][j] = (char)(j % 256);
    }
    
    /* Copy slice to another location */
    char buffer[50];
    for (int j = start; j <= end; ++j) {
        buffer[j - start] = grid[42][j];
    }
    
    /* Use volatile to force analysis */
    volatile char check = grid[42][start] + grid[42][end];
    use_int((int)check);
}

/* Test 3: Double array with varying element size */
static void __attribute__((noinline))
test_double_array(void) {
    double matrix[15][25];
    
    /* Constant bounds that create count > 2 */
    const int lo_idx = 3;
    const int hi_idx = 12;  /* count = 10 */
    
    /* Store to slice */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Load from slice with computation */
    volatile double acc = 0.0;
    for (int j = lo_idx; j <= hi_idx; ++j) {
        acc += matrix[7][j];
    }
    use_double(acc);
    
    /* Cross-slice copy */
    for (int j = lo_idx; j <= hi_idx; ++j) {
        matrix[8][j] = matrix[7][j] * 2.0;
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline))
test_single_element(void) {
    int table[30][40];
    
    const int idx = 17;  /* lo = hi, count = 1 */
    
    /* Both store and load on single element */
    table[12][idx] = 999;
    volatile int val = table[12][idx];
    use_int(val);
    
    /* Chain of single-element accesses */
    table[13][idx] = table[12][idx] + 1;
    use_int(table[13][idx]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    const int n = 30;
    int vla[n][n];  /* VLA with constant size */
    
    /* Constant bounds slice */
    const int slice_start = 5;
    const int slice_end = 14;  /* count = 10 */
    
    /* Initialize slice */
    for (int j = slice_start; j <= slice_end; ++j) {
        vla[10][j] = j * 3;
    }
    
    /* Copy slice within VLA */
    for (int j = slice_start; j <= slice_end; ++j) {
        vla[11][j] = vla[10][j] + 100;
    }
    
    /* Use volatile index calculation */
    volatile int mid = slice_start + (slice_end - slice_start) / 2;
    use_int(vla[10][mid]);
}

/* Test 6: Mixed types and bounds with volatile wrapper */
static void __attribute__((noinline))
test_mixed_types(void) {
    struct Mixed {
        char c;
        int i;
        double d;
    } data[20][15];
    
    /* Use volatile to wrap constants - forces middle-end analysis */
    volatile int vlo = 2;
    volatile int vhi = 4;  /* count = 3 */
    int lo = vlo;
    int hi = vhi;
    
    /* Access struct slice */
    for (int j = lo; j <= hi; ++j) {
        data[5][j].c = 'A' + j;
        data[5][j].i = j * 100;
        data[5][j].d = j * 0.5;
    }
    
    /* Copy struct slice */
    for (int j = lo; j <= hi; ++j) {
        data[6][j] = data[5][j];
    }
    
    use_ptr(&data[5][lo]);
}

/* Test 7: Exactly count = 2 case with float */
static void __attribute__((noinline))
test_exactly_two(void) {
    float values[50][60];
    
    /* Exactly two elements */
    const int a = 25;
    const int b = 26;  /* count = 2 */
    
    /* Store pair */
    values[30][a] = 3.14f;
    values[30][b] = 2.71f;
    
    /* Load and compute */
    volatile float prod = values[30][a] * values[30][b];
    use_double((double)prod);
    
    /* Swap elements */
    float temp = values[30][a];
    values[30][a] = values[30][b];
    values[30][b] = temp;
}

/* Test 8: Large count that triggers TYPE_SIZE calculation */
static void __attribute__((noinline))
test_large_block(void) {
    /* long long has larger TYPE_SIZE */
    long long big[10][100];
    
    const int block_start = 20;
    const int block_end = 49;  /* count = 30 */
    
    /* Fill block */
    for (int j = block_start; j <= block_end; ++j) {
        big[5][j] = (long long)j << 16;
    }
    
    /* Copy entire block */
    for (int j = block_start; j <= block_end; ++j) {
        big[6][j] = big[5][j] + 1;
    }
    
    /* Check boundaries */
    volatile long long first = big[5][block_start];
    volatile long long last = big[5][block_end];
    use_int((int)(first ^ last));
}

int main(void) {
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests to exercise different paths */
    test_single_element();      /* count = 1 */
    test_exactly_two();         /* count = 2 */
    test_small_count_int();     /* count = 2, different type */
    test_large_count_char();    /* count = 16, char type */
    test_double_array();        /* count = 10, double type */
    test_vla_constant_size();   /* VLA with constant bounds */
    test_mixed_types();         /* count = 3, struct type */
    test_large_block();         /* count = 30, long long type */
    
    printf("All tests completed.\n");
    
    /* Create a simple checksum to ensure execution */
    volatile int checksum = 0;
    int dummy[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            dummy[i][j] = i * 10 + j;
            checksum += dummy[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

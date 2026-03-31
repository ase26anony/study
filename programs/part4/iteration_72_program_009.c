/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

/* Dummy functions to prevent early optimization */
static void __attribute__((noinline, noipa)) use_int(int val) {
    volatile static int sink;
    sink = val;
}

static void __attribute__((noinline, noipa)) use_double(double val) {
    volatile static double sink;
    sink = val;
}

static void __attribute__((noinline, noipa)) use_ptr(void *ptr) {
    volatile static void *sink;
    sink = ptr;
}

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline)) test_small_slice(void) {
    int arr[10][20] = {0};
    
    /* Constant bounds known at compile time */
    const int lo = 5;
    const int hi = 6;  /* count = 2 */
    
    /* Store context (lvalue) - MEM_P(target) should be true */
    for (int j = lo; j <= hi; ++j) {
        arr[3][j] = j * 10;  /* Write to slice */
    }
    
    /* Load context (rvalue) - MEM_P(target) should be true */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += arr[3][j];    /* Read from slice */
    }
    use_int(sum);
    
    /* Mixed access pattern */
    int temp[2];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];  /* Load from array slice */
    }
    for (int j = lo; j <= hi; ++j) {
        arr[4][j] = temp[j - lo];  /* Store to array slice */
    }
}

/* Test 2: Larger slice with count > 2, small element size (char) */
static void __attribute__((noinline)) test_char_slice(void) {
    char grid[100][50];
    
    /* Constant bounds with volatile wrapper to force middle-end analysis */
    volatile int vlo = 10;
    volatile int vhi = 19;
    int lo = vlo;  /* Compiler knows this is 10 */
    int hi = vhi;  /* Compiler knows this is 19 */
    /* count = 10, TYPE_SIZE(char) = 1, total = 10 bytes */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        grid[25][j] = (char)(j % 256);
    }
    
    /* Copy slice to another location */
    char buffer[10];
    for (int j = lo; j <= hi; ++j) {
        buffer[j - lo] = grid[25][j];  /* Load from slice */
    }
    for (int j = lo; j <= hi; ++j) {
        grid[26][j] = buffer[j - lo];  /* Store to slice */
    }
    
    /* Use result to prevent elimination */
    volatile char checksum = 0;
    for (int j = lo; j <= hi; ++j) {
        checksum ^= grid[25][j] ^ grid[26][j];
    }
    use_int(checksum);
}

/* Test 3: Double array with medium slice, larger element size */
static void __attribute__((noinline)) test_double_slice(void) {
    double matrix[15][25];
    
    /* Constant bounds through arithmetic */
    const int base = 5;
    int lo = base + 0;  /* = 5 */
    int hi = base + 7;  /* = 12, count = 8 */
    /* TYPE_SIZE(double) = 8, total = 64 bytes */
    
    /* Write to slice (store context) */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 1.5;
    }
    
    /* Read from slice (load context) */
    volatile double acc = 0.0;
    for (int j = lo; j <= hi; ++j) {
        acc += matrix[7][j];
    }
    use_double(acc);
    
    /* Slice copy operation */
    double temp[8];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = matrix[7][j];  /* Load */
    }
    for (int j = lo; j <= hi; ++j) {
        matrix[8][j] = temp[j - lo] * 2.0;  /* Store with transformation */
    }
}

/* Test 4: Single element slice (count = 1) */
static void __attribute__((noinline)) test_single_element(void) {
    int data[30][40];
    
    /* Single element access - count = 1 */
    const int idx = 17;
    
    /* Both store and load on same element */
    data[10][idx] = 12345;      /* Store */
    int val = data[10][idx];    /* Load */
    data[11][idx] = val * 2;    /* Store using loaded value */
    
    use_int(data[11][idx]);
}

/* Test 5: VLA with constant size expression */
static void __attribute__((noinline)) test_vla_slice(void) {
    /* VLA with compile-time constant size */
    const int n = 30;
    int vla[n][n];
    
    /* Constant bounds */
    const int lo = 8;
    const int hi = 15;  /* count = 8 */
    
    /* Initialize slice */
    for (int j = lo; j <= hi; ++j) {
        vla[10][j] = j * 3;
    }
    
    /* Copy slice row to row */
    for (int j = lo; j <= hi; ++j) {
        vla[11][j] = vla[10][j] + 1;  /* Load and store in one expression */
    }
    
    /* Use pointer to slice to force MEM_P analysis */
    int *slice_ptr = &vla[10][lo];
    use_ptr(slice_ptr);
    
    /* Compute checksum */
    volatile int sum = 0;
    for (int j = lo; j <= hi; ++j) {
        sum += vla[10][j] + vla[11][j];
    }
    use_int(sum);
}

/* Test 6: Mixed slice sizes in same function */
static void __attribute__((noinline)) test_mixed_slices(void) {
    unsigned short table[50][60];
    
    /* Test count = 2 */
    {
        const int lo = 20;
        const int hi = 21;
        
        /* Store then load */
        table[30][lo] = 100;
        table[30][hi] = 200;
        unsigned short a = table[30][lo];
        unsigned short b = table[30][hi];
        table[31][lo] = a + b;
        table[31][hi] = a - b;
    }
    
    /* Test count = 10 (TYPE_SIZE(short)=2, total=20 bytes) */
    {
        const int lo = 40;
        const int hi = 49;
        
        for (int j = lo; j <= hi; ++j) {
            table[35][j] = j * 2;
        }
        
        /* Copy with offset */
        for (int j = lo; j <= hi; ++j) {
            table[36][j] = table[35][j] + 1000;
        }
    }
    
    /* Use results */
    volatile unsigned short check = 0;
    check ^= table[31][20] ^ table[31][21];
    check ^= table[36][45];
    use_int(check);
}

/* Test 7: Three-dimensional array slice */
static void __attribute__((noinline)) test_3d_slice(void) {
    char cube[10][20][30];
    
    /* Constant bounds for middle dimension */
    const int lo = 5;
    const int hi = 9;  /* count = 5 */
    
    /* Access slice in 3D array */
    for (int j = lo; j <= hi; ++j) {
        cube[3][j][7] = (char)(j + 'A');
    }
    
    /* Copy slice */
    char temp[5];
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = cube[3][j][7];  /* Load */
    }
    for (int j = lo; j <= hi; ++j) {
        cube[4][j][7] = temp[j - lo];  /* Store */
    }
    
    /* Verify */
    volatile char result = 0;
    for (int j = lo; j <= hi; ++j) {
        result += cube[3][j][7] - cube[4][j][7];
    }
    use_int(result);
}

int main(void) {
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all tests */
    test_small_slice();      /* count = 2 */
    test_char_slice();       /* count = 10, char type */
    test_double_slice();     /* count = 8, double type */
    test_single_element();   /* count = 1 */
    test_vla_slice();        /* VLA with constant size */
    test_mixed_slices();     /* Mixed count values */
    test_3d_slice();         /* 3D array */
    
    printf("All tests completed\n");
    
    /* Final checksum */
    volatile int final_check = 0;
    final_check += 1;  /* Dummy operation to prevent elimination */
    
    return 0;
}

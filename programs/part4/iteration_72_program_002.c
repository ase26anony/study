/* test_expr_coverage.c */
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

/* Test 1: 2D int array with small slice (count <= 2) */
static void __attribute__((noinline))
test_small_slice_int(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int start = 5;  /* Forces middle-end analysis */
    volatile int end = 6;    /* hi - lo + 1 = 2 */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice: rvalue context */
    for (int j = lo; j <= hi; ++j) {
        temp[j - lo] = arr[3][j];
        use_int(arr[3][j]);  /* Prevent elimination */
    }
    
    /* Store to slice: lvalue context */
    for (int j = lo; j <= hi; ++j) {
        arr[7][j] = temp[j - lo] * 2;
        use_int(arr[7][j]);  /* Prevent elimination */
    }
}

/* Test 2: 2D double array with medium slice (count > 2) */
static void __attribute__((noinline))
test_medium_slice_double(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 25; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds with count = 10 */
    volatile int start = 8;
    volatile int end = 17;
    int lo = start;  /* 8 */
    int hi = end;    /* 17, count = 10 */
    
    double buffer[10];
    
    /* Mixed load/store operations */
    for (int i = 5; i < 8; ++i) {
        /* Load slice: rvalue */
        for (int j = lo; j <= hi; ++j) {
            buffer[j - lo] = matrix[i][j];
            use_double(matrix[i][j]);
        }
        
        /* Store slice: lvalue */
        for (int j = lo; j <= hi; ++j) {
            matrix[i+3][j] = buffer[j - lo] * 1.1;
            use_double(matrix[i+3][j]);
        }
    }
}

/* Test 3: 3D char array with single element (count = 1) */
static void __attribute__((noinline))
test_single_element_char(void) {
    char cube[5][10][15];
    
    /* Initialize */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 15; ++k) {
                cube[i][j][k] = (i + j + k) & 0xFF;
            }
        }
    }
    
    /* Single element access - count = 1 */
    volatile int idx = 7;
    int lo = idx;
    int hi = idx;  /* hi - lo + 1 = 1 */
    
    /* Access same element in both contexts */
    char val = cube[2][3][lo];  /* rvalue */
    use_int((int)val);
    
    cube[2][3][hi] = val + 1;   /* lvalue */
    use_int((int)cube[2][3][hi]);
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_slice(void) {
    const int n = 30;  /* Constant size */
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            vla[i][j] = i * n + j;
        }
    }
    
    /* Constant bounds for slice */
    volatile int start = 10;
    volatile int end = 19;  /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Process a diagonal slice */
    for (int i = 0; i < 5; ++i) {
        int row = i * 3;
        
        /* Load from VLA slice */
        int sum = 0;
        for (int j = lo; j <= hi; ++j) {
            sum += vla[row][j];
        }
        use_int(sum);
        
        /* Store to VLA slice */
        for (int j = lo; j <= hi; ++j) {
            vla[row+1][j] = vla[row][j] * 2;
            use_int(vla[row+1][j]);
        }
    }
}

/* Test 5: Mixed types and access patterns */
static void __attribute__((noinline))
test_mixed_patterns(void) {
    struct Mixed {
        short s;
        int i;
        float f;
    } data[50][40];
    
    /* Initialize */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 40; ++j) {
            data[i][j].s = (short)(i + j);
            data[i][j].i = i * 100 + j;
            data[i][j].f = i * 0.5f + j * 0.1f;
        }
    }
    
    /* Test different slice sizes */
    volatile int bounds[][2] = {{1, 2}, {5, 14}, {20, 29}};
    
    for (int b = 0; b < 3; ++b) {
        int lo = bounds[b][0];
        int hi = bounds[b][1];
        int count = hi - lo + 1;  /* 2, 10, 10 respectively */
        
        /* Access struct members through slice */
        for (int i = 10; i < 15; ++i) {
            /* Load int members from slice */
            int sum = 0;
            for (int j = lo; j <= hi; ++j) {
                sum += data[i][j].i;
            }
            use_int(sum);
            
            /* Store to float members in slice */
            for (int j = lo; j <= hi; ++j) {
                data[i+5][j].f = data[i][j].f * 2.0f;
                use_double(data[i+5][j].f);
            }
        }
    }
}

/* Test 6: Array pointer manipulation with constant bounds */
static void __attribute__((noinline))
test_array_pointers(void) {
    int table[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            table[i][j] = i * 50 + j;
        }
    }
    
    /* Get pointer to a specific row slice */
    volatile int row = 42;
    volatile int start = 15;
    volatile int end = 24;  /* count = 10 */
    
    int *slice_ptr = &table[row][start];
    use_ptr(slice_ptr);
    
    /* Process through pointer with constant bounds */
    int lo = 0;
    int hi = end - start;  /* Constant: 9 */
    
    /* Copy slice to another row */
    for (int j = lo; j <= hi; ++j) {
        table[row+1][start + j] = slice_ptr[j] + 1000;
        use_int(table[row+1][start + j]);
    }
    
    /* Small slice copy (count = 2) */
    lo = 5;
    hi = 6;
    for (int j = lo; j <= hi; ++j) {
        table[row+2][start + j] = slice_ptr[j] * 3;
        use_int(table[row+2][start + j]);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Execute all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_medium_slice_double();
    checksum += 2;
    
    test_single_element_char();
    checksum += 3;
    
    test_vla_constant_slice();
    checksum += 4;
    
    test_mixed_patterns();
    checksum += 5;
    
    test_array_pointers();
    checksum += 6;
    
    /* Print checksum to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

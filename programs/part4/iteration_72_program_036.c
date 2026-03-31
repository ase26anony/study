/* Test program for expr.cc lines 7691-7700 - constant bounds array operations */
#include <stdio.h>
#include <stddef.h>

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

/* Test 1: Small count (count <= 2) with int array */
static void __attribute__((noinline))
test_small_count_int(void) {
    int arr[10][20];
    
    /* Initialize with some values */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 20; ++j) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int start = 5;  /* Forces middle-end analysis */
    volatile int end = 6;    /* count = 2 */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    for (int j = lo; j <= hi; ++j) {
        /* Store context */
        arr[3][j] = j * 10;
        
        /* Load context */
        int val = arr[3][j];
        use_int(val);
    }
    
    /* Another case with count = 1 */
    volatile int single = 8;
    int idx = single;
    
    /* Mixed operations */
    arr[5][idx] = arr[5][idx] + 1;
    use_int(arr[5][idx]);
}

/* Test 2: Larger count with double array */
static void __attribute__((noinline))
test_large_count_double(void) {
    double matrix[15][25];
    
    /* Initialize */
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 25; ++j) {
            matrix[i][j] = i * 1.5 + j * 0.3;
        }
    }
    
    /* Constant bounds: count = 10 > 2 */
    volatile int lo_idx = 3;
    volatile int hi_idx = 12;
    int lo = lo_idx;
    int hi = hi_idx;
    
    /* Store context - writing to slice */
    for (int j = lo; j <= hi; ++j) {
        matrix[7][j] = j * 2.5;
    }
    
    /* Load context - reading from slice */
    double sum = 0.0;
    for (int j = lo; j <= hi; ++j) {
        sum += matrix[7][j];
    }
    use_double(sum);
    
    /* Another pattern: copy between slices */
    for (int j = lo; j <= hi; ++j) {
        matrix[8][j] = matrix[7][j];
    }
}

/* Test 3: Char array with varying element size */
static void __attribute__((noinline))
test_char_array(void) {
    char buffer[50][100];
    
    /* Initialize */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 100; ++j) {
            buffer[i][j] = (i + j) % 256;
        }
    }
    
    /* Test different count values */
    
    /* Case 1: count = 2 */
    {
        volatile int c1_start = 10;
        volatile int c1_end = 11;
        int lo = c1_start;
        int hi = c1_end;
        
        for (int j = lo; j <= hi; ++j) {
            buffer[20][j] = 'A' + j;
            char c = buffer[20][j];
            use_int((int)c);
        }
    }
    
    /* Case 2: count = 15 > 2 */
    {
        volatile int c2_start = 30;
        volatile int c2_end = 44;
        int lo = c2_start;
        int hi = c2_end;
        
        /* Block copy operation */
        for (int j = lo; j <= hi; ++j) {
            buffer[21][j] = buffer[20][j - 20];
        }
        
        /* Verify */
        char check = 0;
        for (int j = lo; j <= hi; ++j) {
            check ^= buffer[21][j];
        }
        use_int((int)check);
    }
}

/* Test 4: VLA with constant size expression */
static void __attribute__((noinline))
test_vla_constant_size(void) {
    /* VLA with compile-time constant size */
    volatile int n = 30;  /* Constant propagated */
    int size = n;
    
    int vla[size][size];
    
    /* Initialize */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            vla[i][j] = i * size + j;
        }
    }
    
    /* Constant bounds slice */
    volatile int vla_lo = 5;
    volatile int vla_hi = 14;  /* count = 10 */
    int lo = vla_lo;
    int hi = vla_hi;
    
    /* Operations on VLA slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store */
        vla[10][j] = j * 100;
        
        /* Load and use */
        int val = vla[10][j];
        use_int(val);
    }
    
    /* Another slice with count = 3 */
    volatile int vla_lo2 = 20;
    volatile int vla_hi2 = 22;
    int lo2 = vla_lo2;
    int hi2 = vla_hi2;
    
    for (int j = lo2; j <= hi2; ++j) {
        vla[15][j] = vla[10][j - 15] + 1;
    }
}

/* Test 5: Mixed types and complex index calculations */
static void __attribute__((noinline))
test_mixed_types(void) {
    struct Mixed {
        int a;
        double b;
        char c[4];
    } data[20][15];
    
    /* Initialize */
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 15; ++j) {
            data[i][j].a = i * 10 + j;
            data[i][j].b = i * 0.5 + j * 0.1;
            for (int k = 0; k < 4; ++k) {
                data[i][j].c[k] = 'A' + ((i + j + k) % 26);
            }
        }
    }
    
    /* Constant bounds with struct elements */
    volatile int struct_lo = 2;
    volatile int struct_hi = 5;  /* count = 4 */
    int lo = struct_lo;
    int hi = struct_hi;
    
    /* Access struct members through slice */
    for (int j = lo; j <= hi; ++j) {
        /* Store to int member */
        data[8][j].a = j * 50;
        
        /* Load from double member */
        double d = data[8][j].b;
        use_double(d);
        
        /* Store to char array within struct */
        data[8][j].c[0] = 'Z' - j;
    }
}

/* Test 6: Multi-dimensional slice with constant bounds */
static void __attribute__((noinline))
test_multi_dim_slice(void) {
    int cube[5][10][15];
    
    /* Initialize 3D array */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 15; ++k) {
                cube[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Constant bounds in multiple dimensions */
    volatile int dim1_lo = 1;
    volatile int dim1_hi = 3;
    volatile int dim2_lo = 4;
    volatile int dim2_hi = 8;  /* count = 5 in second dimension */
    
    int lo1 = dim1_lo;
    int hi1 = dim1_hi;
    int lo2 = dim2_lo;
    int hi2 = dim2_hi;
    
    /* Process a 2D slice */
    for (int i = lo1; i <= hi1; ++i) {
        for (int j = lo2; j <= hi2; ++j) {
            /* Both store and load */
            cube[i][j][7] = cube[i][j][6] * 2;
            use_int(cube[i][j][7]);
        }
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations with constant bounds...\n");
    
    /* Run all tests */
    test_small_count_int();
    checksum += 1;
    
    test_large_count_double();
    checksum += 2;
    
    test_char_array();
    checksum += 3;
    
    test_vla_constant_size();
    checksum += 4;
    
    test_mixed_types();
    checksum += 5;
    
    test_multi_dim_slice();
    checksum += 6;
    
    /* Final sink to prevent optimization */
    volatile int final_sink = checksum;
    printf("Checksum: %d\n", final_sink);
    
    return final_sink != 21 ? 1 : 0;
}

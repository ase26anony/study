/* Test program for expr.cc lines 7691-7700 - constant bounds array slice operations */
#include <stdio.h>
#include <stddef.h>

/* Opaque functions to prevent early optimization */
static void __attribute__((noinline, noipa)) use_int(int val) {
    volatile int sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa)) use_double(double val) {
    volatile double sink = val;
    (void)sink;
}

static void __attribute__((noinline, noipa)) use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

/* Test 1: Small slice (count <= 2) with int array - should take first branch */
static void __attribute__((noinline)) test_small_slice_int(void) {
    int arr[10][20];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Constant bounds known at compile time */
    volatile int start = 5;  /* Forces middle-end analysis */
    volatile int end = 6;    /* count = 2 (hi - lo + 1 = 6 - 5 + 1 = 2) */
    int lo = start;
    int hi = end;
    
    /* Both store (lvalue) and load (rvalue) contexts */
    int temp[2];
    
    /* Load from slice - rvalue context */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = arr[3][j];
        use_int(arr[3][j]);
    }
    
    /* Store to slice - lvalue context */
    for (int j = lo; j <= hi; j++) {
        arr[7][j] = temp[j - lo] * 2;
        use_int(arr[7][j]);
    }
}

/* Test 2: Larger slice (count > 2) with char array - tests TYPE_SIZE calculation */
static void __attribute__((noinline)) test_large_slice_char(void) {
    char grid[100][50];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            grid[i][j] = (char)((i + j) % 256);
        }
    }
    
    /* Constant bounds: count = 10 elements */
    volatile int start = 10;
    volatile int end = 19;   /* count = 10 */
    int lo = start;
    int hi = end;
    
    /* Mixed contexts */
    char buffer[10];
    
    /* Load slice - rvalue */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = grid[25][j];
        use_int(grid[25][j]);
    }
    
    /* Store slice - lvalue */
    for (int j = lo; j <= hi; j++) {
        grid[75][j] = buffer[j - lo] ^ 0x55;
        use_int(grid[75][j]);
    }
}

/* Test 3: Double array with medium slice - tests different TYPE_SIZE */
static void __attribute__((noinline)) test_medium_slice_double(void) {
    double matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 1.5 + j * 0.1;
        }
    }
    
    /* Constant bounds: count = 5 elements */
    volatile int start = 8;
    volatile int end = 12;   /* count = 5 */
    int lo = start;
    int hi = end;
    
    double temp[5];
    
    /* Load operation */
    for (int j = lo; j <= hi; j++) {
        temp[j - lo] = matrix[5][j];
        use_double(matrix[5][j]);
    }
    
    /* Store operation */
    for (int j = lo; j <= hi; j++) {
        matrix[8][j] = temp[j - lo] * 3.14;
        use_double(matrix[8][j]);
    }
}

/* Test 4: Single element slice (count = 1) - edge case */
static void __attribute__((noinline)) test_single_element(void) {
    int data[15][25];
    
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 25; j++) {
            data[i][j] = i * j;
        }
    }
    
    /* Single element access */
    volatile int idx = 12;
    int lo = idx;
    int hi = idx;  /* count = 1 */
    
    /* Both contexts on same element */
    int val = data[7][lo];  /* Load - rvalue */
    use_int(val);
    
    data[9][lo] = val * 3;  /* Store - lvalue */
    use_int(data[9][lo]);
}

/* Test 5: VLA with constant size - affects MEM_P analysis */
static void __attribute__((noinline)) test_vla_constant_size(void) {
    const int n = 30;  /* Constant size VLA */
    int vla[n][n];
    
    /* Initialize VLA */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = (i << 8) | j;
        }
    }
    
    /* Constant bounds slice */
    volatile int start = 5;
    volatile int end = 14;   /* count = 10 */
    int lo = start;
    int hi = end;
    
    int buffer[10];
    
    /* Load from VLA slice */
    for (int j = lo; j <= hi; j++) {
        buffer[j - lo] = vla[10][j];
        use_int(vla[10][j]);
    }
    
    /* Store to VLA slice */
    for (int j = lo; j <= hi; j++) {
        vla[20][j] = buffer[j - lo] + 1000;
        use_int(vla[20][j]);
    }
}

/* Test 6: Mixed operations in same loop - complex pattern */
static void __attribute__((noinline)) test_mixed_operations(void) {
    struct Point {
        int x;
        int y;
        double z;
    } points[5][10];
    
    /* Initialize */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            points[i][j].x = i;
            points[i][j].y = j;
            points[i][j].z = i * 0.5 + j * 0.2;
        }
    }
    
    /* Constant bounds for slice */
    volatile int start = 2;
    volatile int end = 4;    /* count = 3 */
    int lo = start;
    int hi = end;
    
    /* Mixed load/store in same slice */
    for (int j = lo; j <= hi; j++) {
        /* Load from one row */
        int x_val = points[1][j].x;
        double z_val = points[1][j].z;
        
        /* Store to another row */
        points[3][j].x = x_val * 2;
        points[3][j].z = z_val * 1.5;
        
        use_int(points[3][j].x);
        use_double(points[3][j].z);
    }
}

int main(void) {
    volatile int checksum = 0;
    
    printf("Testing array slice operations for expr.cc coverage\n");
    
    /* Run all tests */
    test_small_slice_int();
    checksum += 1;
    
    test_large_slice_char();
    checksum += 2;
    
    test_medium_slice_double();
    checksum += 3;
    
    test_single_element();
    checksum += 4;
    
    test_vla_constant_size();
    checksum += 5;
    
    test_mixed_operations();
    checksum += 6;
    
    /* Use checksum to prevent optimization */
    volatile int final = checksum;
    printf("Test completed with checksum: %d\n", final);
    
    return final != 21 ? 1 : 0;
}

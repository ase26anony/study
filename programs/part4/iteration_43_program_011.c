/* Test for GCC expr.cc constant bounds checking */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 (triggers count <= 2 path) */
static int test_small_array_init(void) {
    int arr[5] = {0};
    
    /* Constant bounds: lo=0, hi=1, count=2 */
    int src[2] = {42, 43};
    
    /* This should trigger count <= 2 for MEM target */
    for (int i = 0; i < 2; i++) {
        arr[i] = src[i];
    }
    
    return arr[0] + arr[1];  /* 42 + 43 = 85 */
}

/* Test 2: MEM target with larger count but small total size 
   (triggers TYPE_SIZE * count calculation) */
static int test_char_array_init(void) {
    /* 10 chars = 10 bytes total, likely below threshold */
    char buffer[10];
    
    /* Constant bounds: lo=0, hi=9, count=10 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;  /* Sum of ASCII values A-J */
}

/* Test 3: Non-MEM target (register operation) 
   (triggers !MEM_P(target) path) */
static int test_bitfield_extract(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } p = {.a = 1, .b = 2, .c = 3, .d = 4};
    
    /* Extracting bitfields into registers (non-MEM targets) */
    unsigned int a = p.a;  /* Should be treated as register operation */
    unsigned int b = p.b;
    unsigned int c = p.c;
    unsigned int d = p.d;
    
    return a + b + c + d;  /* 1 + 2 + 3 + 4 = 10 */
}

/* Test 4: Array slice copy with constant bounds */
static int test_array_slice_copy(void) {
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10] = {0};
    
    /* Constant bounds: lo=2, hi=5, count=4 */
    for (int i = 2; i <= 5; i++) {
        dst[i] = src[i];
    }
    
    return dst[2] + dst[3] + dst[4] + dst[5];  /* 2 + 3 + 4 + 5 = 14 */
}

/* Test 5: Small structure copy (count=1 for struct type) */
static int test_struct_copy(void) {
    struct point {
        int x;
        int y;
    };
    
    struct point p1 = {.x = 100, .y = 200};
    struct point p2;
    
    /* This is a constant-sized memory copy of 2 ints */
    p2 = p1;
    
    return p2.x + p2.y;  /* 100 + 200 = 300 */
}

/* Test 6: Mixed types with constant loop bounds */
static int test_mixed_types(void) {
    short shorts[4];
    int ints[2];
    
    /* Initialize with constant bounds */
    for (int i = 0; i < 4; i++) {
        shorts[i] = (short)(i * 10);
    }
    
    for (int i = 0; i < 2; i++) {
        ints[i] = i * 100;
    }
    
    return shorts[0] + shorts[3] + ints[0] + ints[1];  /* 0 + 30 + 0 + 100 = 130 */
}

/* Test 7: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    int *ptr = array;
    
    /* Constant bounds through pointer arithmetic */
    int sum = 0;
    sum += ptr[0];  /* lo=0, hi=0, count=1 */
    sum += ptr[1];  /* lo=1, hi=1, count=1 */
    sum += ptr[2];  /* lo=2, hi=2, count=1 */
    
    return sum;  /* 10 + 20 + 30 = 60 */
}

/* Test 8: Boolean array initialization (small element size) */
static int test_bool_array(void) {
    _Bool flags[16];  /* Each is 1 byte typically */
    
    /* Constant bounds: lo=0, hi=15, count=16, but element size is small */
    for (int i = 0; i < 16; i++) {
        flags[i] = (i % 2) == 0;
    }
    
    int true_count = 0;
    for (int i = 0; i < 16; i++) {
        if (flags[i]) true_count++;
    }
    
    return true_count;  /* Should be 8 */
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* 85 */
    total += test_char_array_init();       /* 735 (sum of 'A' to 'J') */
    total += test_bitfield_extract();      /* 10 */
    total += test_array_slice_copy();      /* 14 */
    total += test_struct_copy();           /* 300 */
    total += test_mixed_types();           /* 130 */
    total += test_pointer_arithmetic();    /* 60 */
    total += test_bool_array();            /* 8 */
    
    /* Total should be: 85 + 735 + 10 + 14 + 300 + 130 + 60 + 8 = 1342 */
    printf("Result: %d\n", total);
    
    /* Verify expected result */
    if (total == 1342) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Unexpected result: %d\n", total);
        return 1;
    }
}

/* Test cases for expr.cc constant bounds checking */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    dst[2] = src[2];  /* lo=2, hi=2, count=1 */
    
    return dst[2];  /* Should return 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Constant bounds: copy two elements */
    dst[0] = src[0];  /* First copy */
    dst[1] = src[1];  /* Second copy - compiler may combine as count=2 operation */
    
    return dst[0] + dst[1];  /* Should return 30 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10] = {0};
    
    /* Constant bounds: copy all 10 chars */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];  /* lo=0, hi=9, count=10, but element size=1 byte */
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;  /* Should return 55 */
}

/* Test 4: MEM target with short array (small element size) */
static int test_mem_short_array(void) {
    short src[4] = {100, 200, 300, 400};
    short dst[4] = {0};
    
    /* Constant bounds: copy all 4 shorts */
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    
    return dst[0] + dst[1] + dst[2] + dst[3];  /* Should return 1000 */
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extract bit-fields into registers (non-MEM targets) */
    unsigned int a = s.a;  /* Extracted to register */
    unsigned int b = s.b;
    unsigned int c = s.c;
    unsigned int d = s.d;
    
    return a + b + c + d;  /* Should return 10 */
}

/* Test 6: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        int x;
        int y;
    };
    
    struct Point p1 = {42, 24};
    struct Point p2;
    
    /* Structure copy - constant size (2 ints) */
    p2 = p1;
    
    return p2.x + p2.y;  /* Should return 66 */
}

/* Test 7: Array slice with constant bounds */
static int test_array_slice(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int slice[3];
    
    /* Copy slice arr[2..4] - constant bounds */
    slice[0] = arr[2];
    slice[1] = arr[3];
    slice[2] = arr[4];
    
    return slice[0] + slice[1] + slice[2];  /* Should return 9 */
}

/* Test 8: Boolean array initialization */
static int test_bool_array(void) {
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += flags[i];  /* Each _Bool is 1 byte */
    }
    return sum;  /* Should return 4 */
}

/* Test 9: Pointer array with constant indices */
static int test_pointer_array(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int *ptrs[4] = {&a, &b, &c, &d};
    int *copy[4];
    
    /* Copy pointer array - constant bounds */
    for (int i = 0; i < 4; i++) {
        copy[i] = ptrs[i];
    }
    
    return *copy[0] + *copy[1] + *copy[2] + *copy[3];  /* Should return 10 */
}

/* Test 10: Mixed operations to ensure all paths are taken */
static int test_mixed(void) {
    /* Combination of different patterns */
    char small[2] = {100, 200};
    char small_copy[2];
    
    /* MEM target with count=2 */
    small_copy[0] = small[0];
    small_copy[1] = small[1];
    
    /* Non-MEM target extraction */
    unsigned char a = small_copy[0];
    unsigned char b = small_copy[1];
    
    return a + b;  /* Should return 300 */
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();        /* +3 */
    total += test_mem_count_2();        /* +30 */
    total += test_mem_small_total_size(); /* +55 */
    total += test_mem_short_array();    /* +1000 */
    total += test_non_mem_target();     /* +10 */
    total += test_struct_copy();        /* +66 */
    total += test_array_slice();        /* +9 */
    total += test_bool_array();         /* +4 */
    total += test_pointer_array();      /* +10 */
    total += test_mixed();              /* +300 */
    
    printf("Total: %d\n", total);  /* Should print 1487 */
    
    /* Verify expected total */
    if (total == 1487) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Test mismatch: expected 1487, got %d\n", total);
        return 1;
    }
}

/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_single_element_copy(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    dst[2] = src[2];  /* lo_index = 2, hi_index = 2, count = 1 */
    
    return dst[2];  /* Should return 3 */
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_two_element_copy(void) {
    short src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    short dst[10] = {0};
    
    /* Constant bounds: copy two elements */
    dst[3] = src[3];  /* First element */
    dst[4] = src[4];  /* Second element - compiler may combine */
    
    return dst[3] + dst[4];  /* Should return 3 + 4 = 7 */
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_small_char_array(void) {
    char src[10] = "abcdefghi";
    char dst[10] = {0};
    
    /* Constant bounds: copy 5 chars (count = 5, eltsize = 1, total = 5 bytes) */
    for (int i = 2; i < 7; i++) {  /* lo_index = 2, hi_index = 6, count = 5 */
        dst[i] = src[i];
    }
    
    return dst[2] + dst[3] + dst[4] + dst[5] + dst[6];  /* Sum of 'c' + 'd' + 'e' + 'f' + 'g' */
}

/* Test 4: MEM target with small struct copy (count = 1, but larger element) */
static int test_small_struct_copy(void) {
    struct small { char a; char b; char c; } src = {'x', 'y', 'z'};
    struct small dst;
    
    /* Constant bounds: copy entire struct (count = 1, size = 3 bytes) */
    dst = src;
    
    return dst.a + dst.b + dst.c;  /* Sum of 'x' + 'y' + 'z' */
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_bitfield_extract(void) {
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
    } bf = {5, 10, 15};
    
    /* Extracting bitfields into registers (non-MEM target) */
    unsigned int val1 = bf.a;  /* Should trigger !MEM_P(target) path */
    unsigned int val2 = bf.b;
    unsigned int val3 = bf.c;
    
    return val1 + val2 + val3;  /* 5 + 10 + 15 = 30 */
}

/* Test 6: Array initialization with compound literal (constant bounds) */
static int test_compound_literal(void) {
    int arr[5];
    
    /* Initialize slice with compound literal - constant bounds */
    int *slice = &arr[1];
    int init[3] = {100, 200, 300};  /* count = 3, but compiler may see constant initialization */
    
    /* Copy with constant bounds */
    for (int i = 0; i < 3; i++) {
        slice[i] = init[i];
    }
    
    return arr[1] + arr[2] + arr[3];  /* 100 + 200 + 300 = 600 */
}

/* Test 7: MEM target with bool array (small element size) */
static int test_bool_array(void) {
    _Bool src[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    _Bool dst[8] = {0};
    
    /* Copy 6 elements (count = 6, eltsize likely 1, total = 6 bytes) */
    for (int i = 1; i < 7; i++) {  /* lo_index = 1, hi_index = 6, count = 6 */
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 1; i < 7; i++) {
        sum += dst[i];
    }
    return sum;  /* Should return 3 (three 1's in positions 1-6) */
}

/* Test 8: Pointer arithmetic with constant bounds */
static int test_pointer_arithmetic(void) {
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int copy[10] = {0};
    
    int *src_ptr = &data[2];
    int *dst_ptr = &copy[2];
    
    /* Copy 4 elements using pointer arithmetic - constant bounds */
    for (int i = 0; i < 4; i++) {  /* count = 4 */
        dst_ptr[i] = src_ptr[i];
    }
    
    return copy[2] + copy[3] + copy[4] + copy[5];  /* 2 + 3 + 4 + 5 = 14 */
}

int main(void) {
    int total = 0;
    
    total += test_single_element_copy();    /* +3 = 3 */
    total += test_two_element_copy();       /* +7 = 10 */
    total += test_small_char_array();       /* +525 (sum of 'c'..'g') = 535 */
    total += test_small_struct_copy();      /* +363 (sum of 'x'..'z') = 898 */
    total += test_bitfield_extract();       /* +30 = 928 */
    total += test_compound_literal();       /* +600 = 1528 */
    total += test_bool_array();             /* +3 = 1531 */
    total += test_pointer_arithmetic();     /* +14 = 1545 */
    
    printf("Result: %d\n", total);
    
    /* Verify expected total */
    if (total == 1545) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Unexpected result!\n");
        return 1;
    }
}

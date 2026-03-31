/* Test cases for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {0xA, 0xBC, 0xD};
    
    /* Extract constant-sized bitfield - should trigger !MEM_P(target) path */
    unsigned int extracted = s.b;  /* 8-bit field extraction */
    return extracted;  /* 0xBC = 188 */
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    int arr[5] = {0};
    
    /* Initialize first 2 elements with constant values */
    int src[2] = {42, 17};
    
    /* This should be recognized as constant-bounded copy of 2 elements */
    for (int i = 0; i < 2; i++) {
        arr[i] = src[i];
    }
    
    return arr[0] + arr[1];  /* 42 + 17 = 59 */
}

/* Test 3: MEM target with count > 2 but small total size - char array */
static int test_char_array_copy(void) {
    char dest[10];
    char src[10] = "123456789";  /* 9 chars + null terminator */
    
    /* Copy 10 chars - total size = 10 bytes, should trigger TYPE_SIZE * count path */
    for (int i = 0; i < 10; i++) {
        dest[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 9; i++) {  /* Sum digits 1-9 */
        sum += dest[i] - '0';
    }
    return sum;  /* 1+2+3+4+5+6+7+8+9 = 45 */
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
    };
    
    struct Point p1 = {100, 200};
    struct Point p2;
    
    /* Structure copy - constant size of 4 bytes (2 shorts) */
    p2 = p1;
    
    return p2.x + p2.y;  /* 100 + 200 = 300 */
}

/* Test 5: Array slice with constant indices */
static int test_array_slice(void) {
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int slice[3];
    
    /* Copy elements 2-4 (inclusive) - count = 3, element size = 4 bytes */
    const int lo = 2;
    const int hi = 4;
    
    for (int i = lo; i <= hi; i++) {
        slice[i - lo] = data[i];
    }
    
    return slice[0] + slice[1] + slice[2];  /* 2 + 3 + 4 = 9 */
}

/* Test 6: Mixed types to test different element sizes */
static int test_mixed_types(void) {
    /* Array of shorts - 5 elements * 2 bytes = 10 bytes total */
    short shorts[5];
    const short src_shorts[5] = {10, 20, 30, 40, 50};
    
    for (int i = 0; i < 5; i++) {
        shorts[i] = src_shorts[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += shorts[i];
    }
    return sum;  /* 10+20+30+40+50 = 150 */
}

/* Test 7: Single element copy (count = 1) */
static int test_single_element(void) {
    int values[5] = {100, 200, 300, 400, 500};
    int single;
    
    /* Copy just one element - minimal case */
    const int idx = 3;
    single = values[idx];
    
    return single;  /* 400 */
}

/* Test 8: Boolean array - very small element size */
static int test_bool_array(void) {
    _Bool flags[8];
    const _Bool src_flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    /* 8 bools - total size implementation dependent but small */
    for (int i = 0; i < 8; i++) {
        flags[i] = src_flags[i];
    }
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (flags[i]) count++;
    }
    return count;  /* 4 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();  /* 188 */
    total += test_small_array_init();     /* 59 */
    total += test_char_array_copy();      /* 45 */
    total += test_struct_copy();          /* 300 */
    total += test_array_slice();          /* 9 */
    total += test_mixed_types();          /* 150 */
    total += test_single_element();       /* 400 */
    total += test_bool_array();           /* 4 */
    
    /* Expected total: 188 + 59 + 45 + 300 + 9 + 150 + 400 + 4 = 1155 */
    printf("Result: %d\n", total);
    
    return 0;
}

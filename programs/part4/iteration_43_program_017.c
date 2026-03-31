/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of array - count = 2 */
    int arr[10] = {1, 2};  /* Only first 2 initialized, rest zero */
    
    /* Force compiler to consider this as constant-bounded operation */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 3 */
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_char_array_copy(void) {
    /* 10 chars = 10 bytes total, small enough for scalar expansion */
    char src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char dst[10];
    
    /* Constant-bounded copy operation */
    for (int i = 0; i < 10; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dst[i];
    }
    return sum;  /* Should be 55 */
}

/* Test 3: Non-MEM target (register operation) */
static int test_bitfield_extract(void) {
    /* Extract constant-sized bitfield into register (non-MEM target) */
    unsigned int value = 0x12345678;
    
    /* Extract bits 8-15 (constant bounds) */
    unsigned int mask = 0xFF00;
    unsigned int extracted = (value & mask) >> 8;
    
    return extracted;  /* Should be 0x56 */
}

/* Test 4: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {1, 2, 3}, s2;
    
    /* Constant-sized structure copy (count = 3, element size = 1) */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;  /* Should be 6 */
}

/* Test 5: Array slice with constant indices */
static int test_array_slice(void) {
    int arr[20] = {0};
    
    /* Initialize slice with constant bounds */
    for (int i = 5; i < 8; i++) {  /* lo=5, hi=7, count=3 */
        arr[i] = i * 2;
    }
    
    int sum = 0;
    for (int i = 5; i < 8; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 30 (10+12+14) */
}

/* Test 6: Mixed types with small total size */
static int test_mixed_small(void) {
    /* Array of shorts - 4 elements = 8 bytes total */
    short data[4] = {100, 200, 300, 400};
    short copy[4];
    
    /* Constant-bounded copy */
    for (int i = 0; i < 4; i++) {
        copy[i] = data[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += copy[i];
    }
    return sum;  /* Should be 1000 */
}

/* Test 7: Boolean array initialization */
static int test_bool_array(void) {
    /* _Bool is typically 1 byte, 8 elements = 8 bytes */
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (flags[i]) count++;
    }
    return count;  /* Should be 4 */
}

/* Test 8: Pointer array with constant bounds */
static int test_pointer_array(void) {
    int a = 1, b = 2, c = 3;
    int* ptrs[3] = {&a, &b, &c};
    int* copy[3];
    
    /* Copy 3 pointers (each 8 bytes on 64-bit, 4 on 32-bit) */
    for (int i = 0; i < 3; i++) {
        copy[i] = ptrs[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += *copy[i];
    }
    return sum;  /* Should be 6 */
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* +3 */
    total += test_char_array_copy();       /* +55 */
    total += test_bitfield_extract();      /* +0x56 = 86 */
    total += test_struct_copy();           /* +6 */
    total += test_array_slice();           /* +30 */
    total += test_mixed_small();           /* +1000 */
    total += test_bool_array();            /* +4 */
    total += test_pointer_array();         /* +6 */
    
    printf("Total: %d\n", total);          /* Expected: 1190 */
    
    /* Verify expected total */
    if (total == 1190) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Unexpected total: %d\n", total);
        return 1;
    }
}

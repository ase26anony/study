/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of array - count = 2 */
    int arr1[10] = {1, 2};  /* Explicit initialization of 2 elements */
    
    /* Copy 2-element slice */
    int arr2[10];
    arr2[0] = arr1[0];
    arr2[1] = arr1[1];
    
    return arr1[0] + arr1[1] + arr2[0] + arr2[1];
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_char_array_ops(void) {
    /* 10 chars = 10 bytes total - should trigger TYPE_SIZE * count path */
    char buffer1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Copy entire 10-char array */
    char buffer2[10];
    for (int i = 0; i < 10; i++) {
        buffer2[i] = buffer1[i];
    }
    
    /* Initialize with compound literal */
    char buffer3[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer1[i] + buffer2[i] + buffer3[i];
    }
    return sum;
}

/* Test 3: Non-MEM target (register operations) */
static int test_bitfield_ops(void) {
    struct packed {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } src = {1, 2, 3, 4};
    
    /* Extract small bitfields into registers (!MEM_P target) */
    unsigned int reg1 = src.a;  /* 4-bit field extraction */
    unsigned int reg2 = src.b;
    unsigned int reg3 = src.c;
    unsigned int reg4 = src.d;
    
    /* Pack into a single value */
    unsigned int result = (reg4 << 12) | (reg3 << 8) | (reg2 << 4) | reg1;
    return (int)result;
}

/* Test 4: Mixed operations with constant bounds */
static int test_mixed_const_bounds(void) {
    /* Using const variables for bounds */
    const int start = 3;
    const int end = 6;  /* count = 4 */
    
    short src[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    short dst[10];
    
    /* Copy constant-bounded slice: 4 elements * sizeof(short) = 8 bytes */
    for (int i = start; i <= end; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'};
    
    /* Structure copy - constant size (3 bytes) */
    struct small s2 = s1;
    
    return s1.a + s1.b + s1.c + s2.a + s2.b + s2.c;
}

/* Test 6: Array initialization with enum bounds */
static int test_enum_bounds(void) {
    enum { ARRAY_SIZE = 5 };  /* Compile-time constant */
    
    int arr[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 7: Boolean array operations */
static int test_bool_array(void) {
    /* _Bool is 1 byte, 8 elements = 8 bytes total */
    _Bool flags[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    
    _Bool copy[8];
    for (int i = 0; i < 8; i++) {
        copy[i] = flags[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += copy[i];
    }
    return sum;
}

/* Test 8: Pointer array with constant bounds */
static int test_pointer_array(void) {
    const char *ptrs[3];
    ptrs[0] = "hello";
    ptrs[1] = "world";
    ptrs[2] = "test";
    
    const char *copy[3];
    for (int i = 0; i < 3; i++) {
        copy[i] = ptrs[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        if (copy[i]) sum += 1;
    }
    return sum;
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* MEM target, count <= 2 */
    total += test_char_array_ops();        /* MEM target, small total size */
    total += test_bitfield_ops();          /* Non-MEM target */
    total += test_mixed_const_bounds();    /* Mixed with const bounds */
    total += test_struct_copy();           /* Structure copy */
    total += test_enum_bounds();           /* Enum bounds */
    total += test_bool_array();            /* Boolean array */
    total += test_pointer_array();         /* Pointer array */
    
    printf("Result: %d\n", total);
    return 0;
}

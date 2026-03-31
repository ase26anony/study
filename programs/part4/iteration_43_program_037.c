/* Test case for expr.cc lines 7691-7700 - constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
/* This should trigger !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } s = {1, 2, 3, 4};
    
    /* Extracting multiple bitfields - compiler may use register operations */
    unsigned int result = (s.a << 12) | (s.b << 8) | (s.c << 4) | s.d;
    return result;
}

/* Test 2: MEM target with count <= 2 */
/* Initialize/copy 1-2 elements of an array */
static int test_small_array_copy(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    
    /* Copy exactly 2 elements - constant bounds */
    dst[0] = src[0];
    dst[1] = src[1];
    
    return dst[0] + dst[1];
}

/* Test 3: MEM target with count > 2 but small total size */
/* char array with 10 elements = 10 bytes total */
static int test_char_array_init(void) {
    char arr[10];
    
    /* Initialize with constant indices */
    for (int i = 0; i < 10; i++) {
        arr[i] = (char)(i + 'A');
    }
    
    /* Use the array to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 4: Structure copy with constant size */
/* Should be treated as constant-bounded memory operation */
static int test_struct_copy(void) {
    struct Point {
        short x;
        short y;
        short z;
    };
    
    struct Point p1 = {100, 200, 300};
    struct Point p2;
    
    /* Structure copy - constant size (6 bytes) */
    p2 = p1;
    
    return p2.x + p2.y + p2.z;
}

/* Test 5: Array slice with constant bounds */
static int test_array_slice(void) {
    int data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int buffer[8] = {0};
    
    /* Copy slice [2..4] - 3 elements, constant bounds */
    const int start = 2;
    const int end = 4;
    
    for (int i = start; i <= end; i++) {
        buffer[i] = data[i];
    }
    
    return buffer[2] + buffer[3] + buffer[4];
}

/* Test 6: Mixed types to test TYPE_SIZE calculations */
static int test_mixed_types(void) {
    /* Array of shorts - 5 elements = 10 bytes */
    short shorts[5];
    for (int i = 0; i < 5; i++) {
        shorts[i] = (short)(i * 100);
    }
    
    /* Array of chars - 8 elements = 8 bytes */
    unsigned char bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (unsigned char)(i * 10);
    }
    
    return shorts[2] + bytes[3];
}

/* Test 7: Nested constant loops that might unroll */
static int test_unrolled_loop(void) {
    int values[4] = {0};
    
    /* Small constant loop that might unroll */
    for (int i = 0; i < 4; i++) {
        values[i] = i * 2 + 1;
    }
    
    /* Another small loop */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += values[i];
    }
    
    return result;
}

/* Test 8: Using compile-time constants via enums */
static int test_enum_bounds(void) {
    enum { ARRAY_SIZE = 6 };
    
    int arr[ARRAY_SIZE];
    
    /* Initialize with enum-based bounds */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * i;
    }
    
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Test 9: Boolean array - small element size */
static int test_bool_array(void) {
    _Bool flags[12];  /* 12 bytes on most systems */
    
    /* Initialize with pattern */
    for (int i = 0; i < 12; i++) {
        flags[i] = (i % 3 == 0);
    }
    
    int count = 0;
    for (int i = 0; i < 12; i++) {
        if (flags[i]) count++;
    }
    
    return count;
}

/* Test 10: Pointer array with constant initialization */
static int test_pointer_array(void) {
    const char *strings[3];
    
    /* Initialize pointer array */
    strings[0] = "hello";
    strings[1] = "world";
    strings[2] = "test";
    
    /* Use pointers to prevent optimization */
    int total_len = 0;
    for (int i = 0; i < 3; i++) {
        total_len += (int)strlen(strings[i]);
    }
    
    return total_len;
}

int main(void) {
    int total = 0;
    
    /* Run all tests and accumulate results */
    total += test_bitfield_extraction();      /* Should trigger !MEM_P(target) */
    total += test_small_array_copy();         /* Should trigger count <= 2 for MEM */
    total += test_char_array_init();          /* Should trigger TYPE_SIZE * count calc */
    total += test_struct_copy();              /* Constant-sized structure copy */
    total += test_array_slice();              /* Array slice with constant bounds */
    total += test_mixed_types();              /* Different element types */
    total += test_unrolled_loop();            /* Potentially unrolled loops */
    total += test_enum_bounds();              /* Enum-based constant bounds */
    total += test_bool_array();               /* Small element type (_Bool) */
    total += test_pointer_array();            /* Pointer array */
    
    printf("Result: %d\n", total);
    
    /* Also test direct constant array initialization */
    int direct_array[4] = {100, 200, 300, 400};
    int direct_sum = 0;
    for (int i = 0; i < 4; i++) {
        direct_sum += direct_array[i];
    }
    printf("Direct array sum: %d\n", direct_sum);
    
    return 0;
}

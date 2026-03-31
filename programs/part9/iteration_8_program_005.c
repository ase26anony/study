/* Test program for GCC expr.cc array copy expansion logic */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[32] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
static char char_dst[32];
static int int_src[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static int int_dst[16];
static long long ll_src[8] = {100,200,300,400,500,600,700,800};
static long long ll_dst[8];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
    char d;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10, 'x'},
    {'b', 2, 20, 'y'},
    {'c', 3, 30, 'z'},
    {'d', 4, 40, 'w'}
};
static struct SmallStruct struct_dst[4];

/* Test 1: Constant small memcpy (count <= 2 or small total size) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 element - should trigger count <= 2 path */
    char_dst[0] = char_src[0];
    
    /* Copy 2 elements - should trigger count <= 2 path */
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    /* Copy 3 chars (3 bytes total) - small size, constant bounds */
    for (int i = 3; i <= 5; ++i) {
        char_dst[i] = char_src[i];
    }
    
    /* Copy 2 ints (8 bytes on 64-bit) - count <= 2 */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    
    /* Copy 3 chars using memcpy with constant size 3 */
    memcpy(&char_dst[6], &char_src[6], 3);
}

/* Test 2: Constant bounds but larger copy (should use library call) */
NOINLINE static void test_const_large_memcpy(void) {
    /* Copy 12 ints (48 bytes on 64-bit) - larger than inline threshold */
    for (int i = 0; i <= 11; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Copy 5 long longs (40 bytes) with memcpy */
    memcpy(&ll_dst[0], &ll_src[0], 5 * sizeof(long long));
}

/* Test 3: Register target (!MEM_P(target)) */
NOINLINE static void test_register_target(void) {
    /* Copy single element to register variable */
    int temp1 = int_src[3];
    int_dst[3] = temp1;
    
    /* Copy single char to register */
    char temp2 = char_src[10];
    char_dst[10] = temp2;
    
    /* Copy single long long to register */
    long long temp3 = ll_src[2];
    ll_dst[2] = temp3;
}

/* Test 4: Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct (small constant size) */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs - count <= 2 */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    /* Copy 3 structs using memcpy */
    memcpy(&struct_dst[0], &struct_src[0], 3 * sizeof(struct SmallStruct));
}

/* Test 5: Variable bounds (should use library call path) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - not constant at compile time */
    for (int i = start; i < end; ++i) {
        char_dst[i] = char_src[i];
    }
    
    /* Variable size memcpy */
    int size = end - start;
    if (size > 0) {
        memcpy(&int_dst[start], &int_src[start], size * sizeof(int));
    }
}

/* Test 6: Mixed element sizes and counts */
NOINLINE static void test_mixed_sizes(void) {
    /* Copy 3 ints (12 bytes) - constant bounds, count > 2 */
    for (int i = 10; i <= 12; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Copy 4 chars (4 bytes) - small total size */
    for (int i = 20; i <= 23; ++i) {
        char_dst[i] = char_src[i];
    }
    
    /* Copy 3 long longs (24 bytes) - constant bounds, count > 2 */
    for (int i = 3; i <= 5; ++i) {
        ll_dst[i] = ll_src[i];
    }
}

/* Test 7: Union copies */
NOINLINE static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u_src = {.i = 0x12345678};
    union Mixed u_dst;
    
    /* Copy entire union (constant size) */
    u_dst = u_src;
    
    /* Copy union array element */
    union Mixed u_arr_src[2] = {{.i = 1}, {.f = 2.0f}};
    union Mixed u_arr_dst[2];
    u_arr_dst[0] = u_arr_src[0];
    u_arr_dst[1] = u_arr_src[1];
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 32; i++) {
        sum += char_dst[i];
    }
    
    for (int i = 0; i < 16; i++) {
        sum += int_dst[i];
    }
    
    for (int i = 0; i < 8; i++) {
        sum += (int)(ll_dst[i] & 0xFFFFFFFF);
    }
    
    for (int i = 0; i < 4; i++) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c + struct_dst[i].d;
    }
    
    return sum;
}

int main(void) {
    /* Initialize destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(5, 10);  /* Variable bounds */
    test_mixed_sizes();
    test_union_copy();
    
    /* Compute and return checksum */
    return compute_checksum() & 0xFF;
}

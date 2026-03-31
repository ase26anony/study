/* Test program to trigger GCC's inline memory copy expansion logic */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test patterns */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global arrays to avoid aliasing issues */
static char src_char[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static char dst_char[16];
static int src_int[8] = {100,101,102,103,104,105,106,107};
static int dst_int[8];
static long long src_ll[4] = {1000LL, 1001LL, 1002LL, 1003LL};
static long long dst_ll[4];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    char c;
};

static struct SmallStruct src_struct[4] = {
    {1, 100, 'a'},
    {2, 200, 'b'},
    {3, 300, 'c'},
    {4, 400, 'd'}
};
static struct SmallStruct dst_struct[4];

/* 1. Constant bounds, small copy (count <= 2) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 element - should trigger !MEM_P(target) or count <= 2 */
    dst_int[0] = src_int[0];  /* Single element copy to memory */
    
    /* Copy 2 elements with constant bounds */
    for (int i = 2; i <= 3; ++i) {  /* count = 2 */
        dst_int[i] = src_int[i];
    }
    
    /* Copy 2 chars (2 bytes total) */
    dst_char[0] = src_char[0];
    dst_char[1] = src_char[1];
}

/* 2. Constant bounds, larger copy but still small byte size */
NOINLINE static void test_const_small_byte_copy(void) {
    /* Copy 3 chars = 3 bytes (small total size) */
    for (int i = 1; i <= 3; ++i) {  /* count = 3, but only 3 bytes */
        dst_char[i] = src_char[i];
    }
    
    /* Copy 3 ints = 12 bytes (may be above threshold on some targets) */
    for (int i = 0; i <= 2; ++i) {  /* count = 3, 12 bytes on 32-bit */
        dst_int[i] = src_int[i];
    }
}

/* 3. Register target (!MEM_P(target)) */
NOINLINE static void test_register_target(void) {
    /* Copy single element to register */
    int temp1 = src_int[3];  /* Should trigger !MEM_P(target) */
    volatile int sink1 = temp1;  /* Prevent elimination */
    
    /* Copy struct element to register */
    struct SmallStruct temp2 = src_struct[1];
    volatile char sink2 = temp2.a;  /* Prevent elimination */
    
    /* Copy via memcpy with constant size 1 (to register-like temp) */
    char temp3;
    memcpy(&temp3, &src_char[5], 1);
    volatile char sink3 = temp3;
}

/* 4. Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct (constant size, small) */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 struct elements */
    for (int i = 1; i <= 2; ++i) {  /* count = 2 structs */
        dst_struct[i] = src_struct[i];
    }
    
    /* Copy struct field by field (might be expanded differently) */
    dst_struct[3].a = src_struct[3].a;
    dst_struct[3].b = src_struct[3].b;
    dst_struct[3].c = src_struct[3].c;
}

/* 5. Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - should go to library call path */
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
}

/* 6. Mixed memcpy calls with constant sizes */
NOINLINE static void test_memcpy_calls(void) {
    /* memcpy with size 1 (should inline) */
    memcpy(&dst_char[10], &src_char[10], 1);
    
    /* memcpy with size 2 (should inline) */
    memcpy(&dst_char[11], &src_char[11], 2);
    
    /* memcpy with size 3 (small byte count) */
    memcpy(&dst_char[12], &src_char[12], 3);
    
    /* memcpy with size 16 (might use library call) */
    memcpy(&dst_char[0], &src_char[0], 16);
}

/* 7. Union test for different type sizes */
union MixedUnion {
    char c[8];
    int i[2];
    long long ll;
};

static union MixedUnion src_union = {.c = {1,2,3,4,5,6,7,8}};
static union MixedUnion dst_union;

NOINLINE static void test_union_copy(void) {
    /* Copy entire union */
    dst_union = src_union;
    
    /* Copy partial union with constant bounds */
    for (int i = 0; i <= 1; ++i) {  /* count = 2 chars */
        dst_union.c[i] = src_union.c[i];
    }
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Run all test patterns */
    test_const_small_memcpy();
    test_const_small_byte_copy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(0, 2);  /* Variable bounds */
    test_memcpy_calls();
    test_union_copy();
    
    /* Create checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += dst_char[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += dst_int[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum += (int)dst_ll[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

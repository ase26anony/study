/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {10, 20, 30, 40, 50, 60, 70, 80};
static int int_dst[8];
static long long ll_src[4] = {100, 200, 300, 400};
static long long ll_dst[4];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 2}, {'b', 3, 4}, {'c', 5, 6}, {'d', 7, 8}
};
static struct SmallStruct struct_dst[4];

/* 1. Constant small memcpy - should trigger count <= 2 or small size */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 char (count=1, size=1) - hits count <= 2 */
    char_dst[0] = char_src[0];
    
    /* Copy 2 chars (count=2, size=2) - hits count <= 2 */
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    /* Copy 3 chars with constant bounds loop - total size = 3 bytes */
    for (int i = 3; i <= 5; ++i) {  /* lo=3, hi=5, count=3 */
        char_dst[i] = char_src[i];
    }
    
    /* memcpy with constant size 2 - should use inline expansion */
    memcpy(&char_dst[6], &char_src[6], 2);
}

/* 2. Constant "large" copy - tests size threshold logic */
NOINLINE static void test_const_large_memcpy(void) {
    /* Copy 3 ints with constant bounds - total size = 12 bytes (assuming 4-byte int) */
    for (int i = 1; i <= 3; ++i) {  /* lo=1, hi=3, count=3 */
        int_dst[i] = int_src[i];
    }
    
    /* Copy 3 long longs - total size = 24 bytes (assuming 8-byte long long) */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        ll_dst[i] = ll_src[i];
    }
    
    /* memcpy with constant size 16 - may exceed inline threshold */
    memcpy(&int_dst[4], &int_src[4], 4 * sizeof(int));
}

/* 3. Register target - tests !MEM_P(target) path */
NOINLINE static void test_register_target(void) {
    /* Copy single element to register (scalar variable) */
    int temp1 = int_src[2];  /* Should trigger !MEM_P(target) */
    
    /* Copy multiple elements to separate registers */
    char temp2 = char_src[5];
    long long temp3 = ll_src[1];
    
    /* Use the values to prevent dead code elimination */
    int_dst[0] = temp1;
    char_dst[10] = temp2;
    ll_dst[3] = temp3;
}

/* 4. Struct copy - tests aggregate types (RECORD_TYPE) */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct - count=1, size=sizeof(struct SmallStruct) */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs - count=2 */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    /* Copy struct slice with constant bounds */
    for (int i = 2; i <= 3; ++i) {  /* lo=2, hi=3, count=2 */
        struct_dst[i] = struct_src[i];
    }
    
    /* memcpy with struct size */
    memcpy(&struct_dst[0], &struct_src[0], 2 * sizeof(struct SmallStruct));
}

/* 5. Variable bounds - should NOT trigger const_bounds_p path */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - const_bounds_p should be false */
    for (int i = start; i < end && i < 8; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* memcpy with variable size */
    int count = end - start;
    if (count > 0 && count < 8) {
        memcpy(&char_dst[start], &char_src[start], count);
    }
}

/* 6. Additional edge cases */
NOINLINE static void test_edge_cases(void) {
    /* Copy exactly 2 elements of various types */
    int_dst[5] = int_src[5];
    int_dst[6] = int_src[6];  /* Two separate assignments */
    
    /* Copy using pointer arithmetic with constant offset */
    *(int_dst + 7) = *(int_src + 7);
    
    /* Union test */
    union TestUnion {
        int i;
        float f;
        char c[4];
    } u_src = {0x12345678}, u_dst;
    
    u_dst = u_src;  /* Union copy assignment */
}

/* Main function that calls all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Execute all test functions */
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(1, 4);  /* Variable bounds copy */
    test_edge_cases();
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        checksum += char_dst[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += int_dst[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum += (int)(ll_dst[i] & 0xFF);
    }
    
    return checksum & 0xFF;  /* Return non-zero value */
}

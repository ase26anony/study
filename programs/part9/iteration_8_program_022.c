/* Test program for GCC expr.cc array copy expansion logic */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static int int_dst[8];
static long long ll_src[4] = {0x1122334455667788LL, 0x2233445566778899LL,
                               0x33445566778899AALL, 0x445566778899AABBLL};
static long long ll_dst[4];

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

/* Union for testing different type sizes */
union MixedUnion {
    char c[8];
    int i[2];
    long long ll;
};

static union MixedUnion union_src = {.ll = 0x123456789ABCDEF0LL};
static union MixedUnion union_dst;

/* 1. Constant bounds, small count (<=2), MEM_P target */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 chars (count <= 2, small size) */
    for (int i = 2; i <= 3; ++i) {  /* lo=2, hi=3, count=2 */
        char_dst[i] = char_src[i];
    }
    
    /* Copy exactly 1 int (count=1 <= 2) */
    int_dst[0] = int_src[0];
    
    /* Copy exactly 2 ints using memcpy with constant size */
    memcpy(&int_dst[1], &int_src[1], 2 * sizeof(int));
}

/* 2. Constant bounds, count > 2 but total size small */
NOINLINE static void test_const_small_total_size(void) {
    /* Copy 3 chars: count=3 > 2, but total size = 3 bytes (small) */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        char_dst[i + 4] = char_src[i + 4];
    }
    
    /* Copy 4 chars using memcpy: 4 bytes total */
    memcpy(&char_dst[8], &char_src[8], 4);
}

/* 3. Constant bounds, count > 2 and total size larger */
NOINLINE static void test_const_larger_size(void) {
    /* Copy 3 long longs: count=3 > 2, total size = 24 bytes (larger) */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        ll_dst[i] = ll_src[i];
    }
    
    /* Copy 3 ints: count=3 > 2, total size = 12 bytes */
    memcpy(&int_dst[3], &int_src[3], 3 * sizeof(int));
}

/* 4. Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* Copy single elements into registers (!MEM_P target) */
    int reg1 = int_src[2];          /* Single element to register */
    char reg2 = char_src[5];        /* Another single element */
    long long reg3 = ll_src[1];     /* Large type single element */
    
    /* Use the values to prevent dead code elimination */
    int_dst[2] = reg1;
    char_dst[5] = reg2;
    ll_dst[1] = reg3;
}

/* 5. Struct and union copies */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct (count=1 <= 2) */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs (count=2 <= 2) */
    for (int i = 1; i <= 2; ++i) {  /* lo=1, hi=2, count=2 */
        struct_dst[i] = struct_src[i];
    }
    
    /* Copy union */
    union_dst = union_src;
    
    /* Copy struct using memcpy with constant size */
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
}

/* 6. Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - compiler can't determine as constant */
    for (int i = start; i < end && i < 8; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Variable size memcpy */
    int count = end - start;
    if (count > 0 && count < 8) {
        memcpy(&char_dst[start], &char_src[start], count);
    }
}

/* 7. Mixed constant expressions */
NOINLINE static void test_mixed_const_expr(void) {
    /* Constant expressions in indices */
    const int idx1 = 1;
    const int idx2 = 3;
    
    /* Copy with constant indices but variable-looking expression */
    for (int i = idx1; i <= idx2; ++i) {
        char_dst[i + 8] = char_src[i + 8];
    }
    
    /* Compile-time constant size calculation */
    enum { COPY_SIZE = 2 };
    memcpy(&ll_dst[2], &ll_src[2], COPY_SIZE * sizeof(long long));
}

/* Main function that calls all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_larger_size();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  /* Variable bounds */
    test_mixed_const_expr();
    
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
    for (int i = 0; i < 4; i++) {
        checksum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    return checksum & 0xFF;  /* Return non-zero, non-constant value */
}

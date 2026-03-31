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

static union MixedUnion union_src = {.ll = 0x0123456789ABCDEFULL};
static union MixedUnion union_dst;

/* Test 1: Constant bounds, count <= 2, MEM_P target */
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

/* Test 2: Constant bounds, count > 2, but total size small */
NOINLINE static void test_const_small_total_size(void) {
    /* Copy 3 chars: TYPE_SIZE=1, count=3, total=3 bytes (small) */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        char_dst[i + 4] = char_src[i + 4];
    }
    
    /* Copy 4 chars using memcpy: 4 bytes total */
    memcpy(&char_dst[8], &char_src[8], 4);
}

/* Test 3: Constant bounds, count > 2, total size larger */
NOINLINE static void test_const_larger_total_size(void) {
    /* Copy 3 long longs: TYPE_SIZE=8, count=3, total=24 bytes */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        ll_dst[i] = ll_src[i];
    }
    
    /* Copy 4 ints: TYPE_SIZE=4, count=4, total=16 bytes */
    memcpy(&int_dst[4], &int_src[4], 4 * sizeof(int));
}

/* Test 4: Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* These should trigger !MEM_P(target) path */
    int temp1 = int_src[3];          /* Single element to register */
    long long temp2 = ll_src[2];     /* Another single element */
    char temp3 = char_src[5];        /* Char to register */
    
    /* Store back to memory to prevent elimination */
    int_dst[3] = temp1;
    ll_dst[2] = temp2;
    char_dst[5] = temp3;
}

/* Test 5: Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct (count=1 <= 2) */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs (count=2 <= 2) */
    for (int i = 0; i <= 1; ++i) {
        struct_dst[i + 1] = struct_src[i + 1];
    }
    
    /* Copy struct using memcpy with constant size */
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
}

/* Test 6: Union copies */
NOINLINE static void test_union_copy(void) {
    /* Whole union copy (count=1 <= 2) */
    union_dst = union_src;
    
    /* Copy union as bytes with constant bounds */
    for (int i = 0; i <= 3; ++i) {  /* lo=0, hi=3, count=4 */
        union_dst.c[i] = union_src.c[i];
    }
}

/* Test 7: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - compiler can't know these at compile time */
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
        }
    }
    
    /* Variable size memcpy */
    int count = end - start + 1;
    if (count > 0 && count <= 8) {
        memcpy(&char_dst[start], &char_src[start], count);
    }
}

/* Test 8: Mixed constant/variable patterns */
NOINLINE static void test_mixed_patterns(void) {
    /* Constant lower bound, variable upper bound */
    int end = 3;  /* Could be made variable via function param in real use */
    for (int i = 0; i <= end; ++i) {
        int_dst[i + 8] = int_src[i + 8];
    }
    
    /* Two separate constant copies (should be expanded inline) */
    ll_dst[4] = ll_src[4];
    ll_dst[5] = ll_src[5];
}

/* Main function that runs all tests and returns checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    memset(&union_dst, 0, sizeof(union_dst));
    
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_larger_total_size();
    test_register_target();
    test_struct_copy();
    test_union_copy();
    test_variable_bounds(2, 5);      /* Variable bounds */
    test_mixed_patterns();
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 32; i++) {
        checksum += char_dst[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += int_dst[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (int)(ll_dst[i] & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        checksum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    for (int i = 0; i < 8; i++) {
        checksum += union_dst.c[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero value */
}

/* Test program for GCC expr.cc array copy expansion logic */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[16] = "0123456789ABCDEF";
static char char_dst[16];
static int int_src[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static int int_dst[8];
static long long ll_src[4] = {0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL,
                               0x1122334455667788LL, 0x8877665544332211LL};
static long long ll_dst[4];

/* Small struct for aggregate type testing */
struct small_struct {
    char a;
    int b;
    short c;
};

static struct small_struct struct_src[4] = {
    {'a', 1, 100}, {'b', 2, 200}, {'c', 3, 300}, {'d', 4, 400}
};
static struct small_struct struct_dst[4];

/* Test 1: Constant small memcpy - should trigger count <= 2 or small size */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy 1 element - count <= 2 branch */
    char_dst[0] = char_src[0];
    
    /* Copy 2 elements - count <= 2 branch */
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    
    /* Copy 3 chars = 3 bytes - small size branch */
    for (int i = 3; i <= 5; ++i) {
        char_dst[i] = char_src[i];
    }
    
    /* Alternative using memcpy with constant size 2 */
    __builtin_memcpy(&char_dst[6], &char_src[6], 2);
    
    /* memcpy with constant size 3 (small size branch) */
    __builtin_memcpy(&char_dst[8], &char_src[8], 3);
}

/* Test 2: Constant larger memcpy - should exceed inline threshold */
NOINLINE static void test_const_large_memcpy(void) {
    /* Copy 3 long longs = 24 bytes (assuming 64-bit) - may exceed threshold */
    for (int i = 0; i <= 2; ++i) {
        ll_dst[i] = ll_src[i];
    }
    
    /* memcpy with constant size 16 bytes */
    __builtin_memcpy(&int_dst[0], &int_src[0], 16);
    
    /* Copy 4 ints = 16 bytes */
    for (int i = 1; i <= 4; ++i) {
        int_dst[i] = int_src[i];
    }
}

/* Test 3: Register target (!MEM_P(target)) - scalar assignment */
NOINLINE static void test_register_target(void) {
    /* These should create register targets */
    int reg1 = int_src[0];
    int reg2 = int_src[1];
    long long reg3 = ll_src[0];
    char reg4 = char_src[0];
    
    /* Store back to memory */
    int_dst[7] = reg1 + reg2;
    ll_dst[3] = reg3;
    char_dst[15] = reg4;
}

/* Test 4: Struct copies - aggregate types */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct - count = 1 */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs - count <= 2 */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    /* Copy using memcpy with constant size (sizeof struct) */
    __builtin_memcpy(&struct_dst[3], &struct_src[3], sizeof(struct small_struct));
    
    /* Copy partial struct members with constant bounds */
    for (int i = 0; i < 2; ++i) {
        struct_dst[i].a = struct_src[i].a;
        struct_dst[i].b = struct_src[i].b;
    }
}

/* Test 5: Variable bounds - should NOT trigger const_bounds_p */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - compiler can't determine as constant */
    for (int i = start; i < end && i < 8; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Another variable bounds example */
    int n = 3;
    for (int i = 0; i < n; ++i) {
        char_dst[i + 10] = char_src[i + 10];
    }
}

/* Test 6: Union copies */
NOINLINE static void test_union_copy(void) {
    union test_union {
        int i;
        float f;
        char c[4];
    } u_src = {0x12345678}, u_dst;
    
    /* Copy whole union - count = 1 */
    u_dst = u_src;
    
    /* Copy partial with constant bounds */
    for (int i = 0; i < 2; ++i) {
        u_dst.c[i] = u_src.c[i];
    }
}

/* Test 7: Mixed types and sizes */
NOINLINE static void test_mixed_types(void) {
    /* Copy exactly 2 elements of different types */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];  /* count = 2 */
    
    /* Copy 1 element of long long */
    ll_dst[0] = ll_src[0];    /* count = 1 */
    
    /* Copy 3 chars (3 bytes) - small size branch */
    char temp[3];
    for (int i = 0; i <= 2; ++i) {
        temp[i] = char_src[i + 5];
    }
    __builtin_memcpy(&char_dst[5], temp, 3);
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 16; ++i) {
        sum += char_dst[i];
    }
    
    for (int i = 0; i < 8; ++i) {
        sum += int_dst[i];
    }
    
    for (int i = 0; i < 4; ++i) {
        sum += (int)(ll_dst[i] & 0xFFFFFFFF) + (int)(ll_dst[i] >> 32);
    }
    
    for (int i = 0; i < 4; ++i) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c;
    }
    
    return sum;
}

int main(void) {
    /* Initialize destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Execute all test cases */
    test_const_small_memcpy();
    test_const_large_memcpy();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  /* Variable bounds */
    test_union_copy();
    test_mixed_types();
    
    /* Return checksum to prevent optimization */
    return compute_checksum() & 0xFF;
}

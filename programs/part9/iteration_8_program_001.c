/* test_expr_coverage.c - Coverage test for GCC expr.cc array copy expansion */

#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test code */
#define KEEP(expr) do { \
    volatile void *__ptr = (volatile void*)&(expr); \
    (void)__ptr; \
} while(0)

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
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30},
    {'d', 4, 40}
};
static struct SmallStruct struct_dst[4];

/* Union for testing */
union MixedUnion {
    int i;
    float f;
    char c[4];
};

static union MixedUnion union_src[4];
static union MixedUnion union_dst[4];

/* Test 1: Constant small memcpy (should trigger inline expansion) */
__attribute__((noinline))
static void test_const_small_memcpy(void) {
    /* Copy 1 element - count <= 2 path */
    memcpy(&char_dst[0], &char_src[0], 1);
    
    /* Copy 2 elements - count <= 2 path */
    memcpy(&char_dst[1], &char_src[1], 2);
    
    /* Copy 3 chars - total size = 3 bytes, small enough to inline */
    memcpy(&char_dst[3], &char_src[3], 3);
    
    /* Copy 2 ints - count <= 2 */
    memcpy(&int_dst[0], &int_src[0], 2 * sizeof(int));
    
    /* Copy 3 ints - larger size, might still inline depending on threshold */
    memcpy(&int_dst[2], &int_src[2], 3 * sizeof(int));
    
    KEEP(char_dst);
    KEEP(int_dst);
}

/* Test 2: Constant bounds with loop (explicit constant indices) */
__attribute__((noinline))
static void test_const_bounds_loop(void) {
    /* Loop with constant bounds - should be unrolled */
    for (int i = 2; i <= 4; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Another constant loop with char */
    for (int i = 0; i <= 1; ++i) {
        char_dst[i + 10] = char_src[i + 10];
    }
    
    /* Loop copying 3 long longs - larger total size */
    for (int i = 0; i <= 2; ++i) {
        ll_dst[i] = ll_src[i];
    }
    
    KEEP(int_dst);
    KEEP(char_dst);
    KEEP(ll_dst);
}

/* Test 3: Register target (!MEM_P(target)) */
__attribute__((noinline))
static void test_register_target(void) {
    /* Copy single element to register */
    int reg1 = int_src[3];
    char reg2 = char_src[5];
    long long reg3 = ll_src[2];
    
    /* Copy back to memory */
    int_dst[3] = reg1;
    char_dst[5] = reg2;
    ll_dst[2] = reg3;
    
    KEEP(reg1);
    KEEP(reg2);
    KEEP(reg3);
}

/* Test 4: Struct and union copies */
__attribute__((noinline))
static void test_struct_copy(void) {
    /* Copy entire small struct - constant size */
    struct_dst[0] = struct_src[0];
    
    /* Copy 2 structs - count <= 2 */
    memcpy(&struct_dst[1], &struct_src[1], 2 * sizeof(struct SmallStruct));
    
    /* Initialize union */
    union_src[0].i = 0x12345678;
    union_src[1].f = 3.14f;
    
    /* Copy union - should be treated as aggregate */
    union_dst[0] = union_src[0];
    union_dst[1] = union_src[1];
    
    /* Copy union array slice with constant bounds */
    for (int i = 0; i <= 1; ++i) {
        union_dst[i + 2] = union_src[i + 2];
    }
    
    KEEP(struct_dst);
    KEEP(union_dst);
}

/* Test 5: Variable bounds (should NOT trigger const_bounds_p) */
__attribute__((noinline))
static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - should go to library call path */
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
        }
    }
    
    /* Variable size memcpy */
    int size = end - start;
    if (size > 0 && size <= 16) {
        memcpy(&char_dst[start], &char_src[start], size);
    }
    
    KEEP(int_dst);
    KEEP(char_dst);
}

/* Test 6: Mixed scenarios to hit different branches */
__attribute__((noinline))
static void test_mixed_scenarios(void) {
    /* Direct assignment of 2 elements - count <= 2 */
    int_dst[10] = int_src[10];
    int_dst[11] = int_src[11];
    
    /* 3 chars via memcpy - small total size */
    memcpy(&char_dst[20], &char_src[20], 3);
    
    /* 3 long longs via memcpy - larger total size */
    memcpy(&ll_dst[3], &ll_src[3], 3 * sizeof(long long));
    
    /* Single struct element to register */
    struct SmallStruct temp = struct_src[2];
    struct_dst[2] = temp;
    
    KEEP(int_dst);
    KEEP(char_dst);
    KEEP(ll_dst);
    KEEP(struct_dst);
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize union array */
    for (int i = 0; i < 4; i++) {
        union_src[i].i = i * 1000;
    }
    
    /* Run all test functions */
    test_const_small_memcpy();
    test_const_bounds_loop();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(2, 5);  /* Variable bounds */
    test_mixed_scenarios();
    
    /* Create checksum to prevent dead code elimination */
    for (int i = 0; i < 32; i++) {
        checksum += char_dst[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += int_dst[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (int)(ll_dst[i] & 0xFF);
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

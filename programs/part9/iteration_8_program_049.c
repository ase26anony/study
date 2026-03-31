/* test_expr_coverage.c
 * Designed to trigger GCC's inline memory copy expansion logic
 * Compile with: gcc -std=gnu11 -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -fprofile-arcs -ftest-coverage
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "ABCDEFGHIJKLMNOPQRST";
static char dst_char[20] = {0};
static long long src_ll[5] = {100, 200, 300, 400, 500};
static long long dst_ll[5] = {0};

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[3] = {
    {'x', 42, 100},
    {'y', 43, 101},
    {'z', 44, 102}
};
static struct SmallStruct dst_struct[3] = {0};

/* 1. Constant bounds, count <= 2, MEM_P(target) = true */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 ints - should trigger count <= 2 branch */
    for (int i = 2; i <= 3; ++i) {  /* lo=2, hi=3, count=2 */
        dst_int[i] = src_int[i];
    }
    
    /* Copy 1 char - count=1 <= 2 */
    dst_char[5] = src_char[5];
    
    /* Copy 2 chars using memcpy with constant size */
    memcpy(&dst_char[10], &src_char[10], 2 * sizeof(char));
}

/* 2. Constant bounds, count > 2, small total size */
NOINLINE static void test_const_small_total_size(void) {
    /* Copy 3 chars = 3 bytes total - small enough to inline */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        dst_char[i] = src_char[i];
    }
    
    /* Copy 4 chars using memcpy - 4 bytes */
    memcpy(&dst_char[15], &src_char[15], 4);
}

/* 3. Constant bounds, count > 2, large total size */
NOINLINE static void test_const_large_total_size(void) {
    /* Copy 3 long longs = 24 bytes (on 64-bit) - may exceed threshold */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        dst_ll[i] = src_ll[i];
    }
    
    /* Copy 5 ints = 20 bytes */
    memcpy(&dst_int[5], &src_int[5], 5 * sizeof(int));
}

/* 4. Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* Single element copy to register - !MEM_P(target) */
    int temp1 = src_int[3];  /* Register target */
    volatile int sink1 = temp1;  /* Prevent elimination */
    
    /* Two elements to separate registers */
    int temp2 = src_int[4];
    int temp3 = src_int[5];
    volatile int sink2 = temp2 + temp3;
    
    /* Struct element to register */
    struct SmallStruct temp4 = src_struct[1];
    volatile char sink3 = temp4.a;
}

/* 5. Struct copies - aggregate types */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct - constant size */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 structs using memcpy */
    memcpy(&dst_struct[1], &src_struct[1], 2 * sizeof(struct SmallStruct));
    
    /* Copy struct field by field (creates multiple small copies) */
    dst_struct[2].a = src_struct[2].a;
    dst_struct[2].b = src_struct[2].b;
    dst_struct[2].c = src_struct[2].c;
}

/* 6. Variable bounds - should NOT trigger const_bounds_p */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - forces library call path */
    for (int i = start; i <= end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Variable size memcpy */
    int size = (end - start + 1) * sizeof(int);
    if (size > 0) {
        memcpy(&dst_char[start], &src_char[start], size);
    }
}

/* 7. Mixed constant/variable patterns */
NOINLINE static void test_mixed_patterns(void) {
    /* Constant inner loop, variable outer */
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i <= 1; ++i) {  /* Constant bounds */
            dst_int[j * 2 + i] = src_int[j * 2 + i];
        }
    }
    
    /* Constant offset, variable count */
    int count = 2;
    memcpy(dst_char + 5, src_char + 5, count);
}

/* 8. Union testing */
NOINLINE static void test_union_copy(void) {
    union Mixed {
        int i;
        float f;
        char c[4];
    } u1 = {.i = 0x12345678}, u2;
    
    /* Copy entire union - constant size */
    u2 = u1;
    
    /* Copy union array element */
    union Mixed uarr[2] = {{.i = 1}, {.f = 2.0f}};
    uarr[1] = uarr[0];
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        sum += dst_int[i];
        if (i < 5) sum += dst_ll[i] % 256;
    }
    
    for (int i = 0; i < 20; i++) {
        sum += dst_char[i];
    }
    
    for (int i = 0; i < 3; i++) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    return sum;
}

int main(void) {
    /* Clear destinations */
    memset(dst_int, 0, sizeof(dst_int));
    memset(dst_char, 0, sizeof(dst_char));
    memset(dst_ll, 0, sizeof(dst_ll));
    memset(dst_struct, 0, sizeof(dst_struct));
    
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_large_total_size();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(1, 3);  /* Variable bounds */
    test_mixed_patterns();
    test_union_copy();
    
    /* Use results to prevent optimization */
    int checksum = compute_checksum();
    
    /* Return checksum mod 256 to avoid overflow issues */
    return checksum & 0xFF;
}

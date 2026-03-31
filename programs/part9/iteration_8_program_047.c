/* test_expr_coverage.c
 * Designed to trigger GCC's inline memory copy expansion logic
 * Compile with: gcc -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -std=gnu11
 */

#include <stdio.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789ABCDEFGHI";
static char dst_char[20] = {0};
static long long src_ll[5] = {100, 200, 300, 400, 500};
static long long dst_ll[5] = {0};

/* Small struct for aggregate type testing */
struct SmallStruct {
    char a;
    int b;
    short c;
};

static struct SmallStruct src_struct[5] = {
    {'a', 1, 10}, {'b', 2, 20}, {'c', 3, 30}, {'d', 4, 40}, {'e', 5, 50}
};
static struct SmallStruct dst_struct[5] = {0};

/* Union for testing different type sizes */
union MixedUnion {
    char c[8];
    int i[2];
    long long ll;
};

static union MixedUnion src_union[3] = {0};
static union MixedUnion dst_union[3] = {0};

/* 1. Constant small memcpy - should trigger count <= 2 path */
NOINLINE static int test_const_small_memcpy(void) {
    int checksum = 0;
    
    /* Copy exactly 2 ints - count <= 2 */
    memcpy(&dst_int[0], &src_int[0], 2 * sizeof(int));
    checksum += dst_int[0] + dst_int[1];
    
    /* Copy exactly 1 int - count <= 2 */
    memcpy(&dst_int[5], &src_int[5], 1 * sizeof(int));
    checksum += dst_int[5];
    
    /* Copy 3 chars - total size small (3 bytes) */
    memcpy(&dst_char[0], &src_char[0], 3);
    checksum += dst_char[0] + dst_char[1] + dst_char[2];
    
    /* Copy 2 long longs - count <= 2 */
    memcpy(&dst_ll[0], &src_ll[0], 2 * sizeof(long long));
    checksum += (int)(dst_ll[0] % 100) + (int)(dst_ll[1] % 100);
    
    return checksum;
}

/* 2. Constant large memcpy - should trigger size-based logic */
NOINLINE static int test_const_large_memcpy(void) {
    int checksum = 0;
    
    /* Copy 3 long longs - 24 bytes on 64-bit, tests size threshold */
    memcpy(&dst_ll[1], &src_ll[1], 3 * sizeof(long long));
    checksum += (int)(dst_ll[1] % 100) + (int)(dst_ll[2] % 100) + (int)(dst_ll[3] % 100);
    
    /* Copy 5 ints - 20 bytes, tests size threshold */
    memcpy(&dst_int[2], &src_int[2], 5 * sizeof(int));
    for (int i = 2; i <= 6; i++) {
        checksum += dst_int[i];
    }
    
    return checksum;
}

/* 3. Register target - should trigger !MEM_P(target) path */
NOINLINE static int test_register_target(void) {
    int checksum = 0;
    
    /* Copy to register (scalar variable) */
    int reg1 = src_int[3];  /* Single element copy to register */
    checksum += reg1;
    
    char reg2 = src_char[5]; /* Single char to register */
    checksum += reg2;
    
    long long reg3 = src_ll[2]; /* Single long long to register */
    checksum += (int)(reg3 % 100);
    
    /* Multiple register copies */
    int reg4 = src_int[0];
    int reg5 = src_int[1];
    checksum += reg4 + reg5;
    
    return checksum;
}

/* 4. Struct copy - tests aggregate types */
NOINLINE static int test_struct_copy(void) {
    int checksum = 0;
    
    /* Copy single struct - count <= 2 */
    dst_struct[0] = src_struct[0];
    checksum += src_struct[0].b + src_struct[0].c;
    
    /* Copy 2 structs - count <= 2 */
    memcpy(&dst_struct[1], &src_struct[1], 2 * sizeof(struct SmallStruct));
    checksum += dst_struct[1].b + dst_struct[2].b;
    
    /* Copy 3 structs - tests size threshold */
    memcpy(&dst_struct[2], &src_struct[2], 3 * sizeof(struct SmallStruct));
    checksum += dst_struct[2].c + dst_struct[3].c + dst_struct[4].c;
    
    /* Union copy */
    src_union[0].ll = 0x123456789ABCDEF0LL;
    dst_union[0] = src_union[0];  /* Single union copy */
    checksum += (int)(dst_union[0].ll & 0xFF);
    
    return checksum;
}

/* 5. Variable bounds - should NOT trigger const_bounds_p path */
NOINLINE static int test_variable_bounds(int start, int end) {
    int checksum = 0;
    
    /* Loop with variable bounds - const_bounds_p should be false */
    for (int i = start; i < end && i < 10; i++) {
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    /* Variable-sized memcpy */
    int count = end - start;
    if (count > 0 && count <= 5) {
        memcpy(&dst_char[start], &src_char[start], count);
        for (int i = start; i < start + count && i < 20; i++) {
            checksum += dst_char[i];
        }
    }
    
    return checksum;
}

/* 6. Additional tests for constant loop bounds */
NOINLINE static int test_constant_loop_bounds(void) {
    int checksum = 0;
    
    /* Loop with compile-time constant bounds */
    for (int i = 2; i <= 4; ++i) {  /* Constant bounds: 2 to 4 inclusive */
        dst_int[i] = src_int[i];
        checksum += dst_int[i];
    }
    
    /* Another constant loop with different bounds */
    for (int i = 0; i < 3; ++i) {  /* 0, 1, 2 */
        dst_char[i + 10] = src_char[i + 10];
        checksum += dst_char[i + 10];
    }
    
    /* Nested constant loops */
    for (int i = 1; i <= 2; ++i) {
        for (int j = 1; j <= 2; ++j) {
            int idx = i * 3 + j;
            if (idx < 10) {
                dst_int[idx] = src_int[idx];
                checksum += dst_int[idx];
            }
        }
    }
    
    return checksum;
}

/* 7. Mixed small and large element types */
NOINLINE static int test_mixed_types(void) {
    int checksum = 0;
    
    /* Copy 4 chars (4 bytes) - small size */
    char small_src[4] = {1, 2, 3, 4};
    char small_dst[4];
    memcpy(small_dst, small_src, 4);
    for (int i = 0; i < 4; i++) checksum += small_dst[i];
    
    /* Copy 4 doubles (32 bytes on most systems) - larger size */
    double large_src[4] = {1.1, 2.2, 3.3, 4.4};
    double large_dst[4];
    memcpy(large_dst, large_src, 4 * sizeof(double));
    for (int i = 0; i < 4; i++) checksum += (int)large_dst[i];
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    /* Initialize source arrays */
    for (int i = 0; i < 10; i++) {
        src_int[i] = i * 10;
    }
    
    for (int i = 0; i < 20; i++) {
        src_char[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 5; i++) {
        src_ll[i] = 1000LL * (i + 1);
    }
    
    /* Run all test cases */
    total_checksum += test_const_small_memcpy();
    total_checksum += test_const_large_memcpy();
    total_checksum += test_register_target();
    total_checksum += test_struct_copy();
    total_checksum += test_variable_bounds(1, 4);  /* Variable bounds */
    total_checksum += test_constant_loop_bounds();
    total_checksum += test_mixed_types();
    
    /* Verify some copies worked */
    printf("Checksum: %d\n", total_checksum);
    printf("Sample verification:\n");
    printf("  dst_int[0]=%d, dst_int[1]=%d\n", dst_int[0], dst_int[1]);
    printf("  dst_char[0-2]=%c%c%c\n", dst_char[0], dst_char[1], dst_char[2]);
    
    return total_checksum != 0 ? 0 : 1;
}

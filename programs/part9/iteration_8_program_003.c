/* test_expr_coverage.c
 * Designed to trigger GCC's inline memory copy expansion logic
 * Compile with: gcc -std=gnu11 -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -fprofile-arcs -ftest-coverage
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to maintain function boundaries for coverage */
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

static struct SmallStruct src_struct[3] = {
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30}
};
static struct SmallStruct dst_struct[3] = {0};

/* Union for testing different type sizes */
union MixedUnion {
    char bytes[8];
    int ints[2];
    long long ll;
};

static union MixedUnion src_union = {.ll = 0x0123456789ABCDEFULL};
static union MixedUnion dst_union = {0};

/* Test 1: Constant bounds, count <= 2, memory target */
NOINLINE static void test_const_small_memcpy(void) {
    /* Case 1: Copy 1 element - should trigger count <= 2 path */
    dst_int[0] = src_int[0];  /* Single assignment */
    
    /* Case 2: Copy 2 elements - explicit assignments */
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    
    /* Case 3: Copy 2 elements via loop with constant bounds */
    for (int i = 3; i <= 4; ++i) {  /* i=3 to i=4 inclusive = 2 elements */
        dst_int[i] = src_int[i];
    }
    
    /* Case 4: memcpy with constant size 2*sizeof(int) */
    memcpy(&dst_int[5], &src_int[5], 2 * sizeof(int));
}

/* Test 2: Constant bounds, count > 2 but small total size */
NOINLINE static void test_const_small_total_size(void) {
    /* Copy 3 chars = 3 bytes total (small) */
    for (int i = 0; i <= 2; ++i) {  /* 3 elements */
        dst_char[i] = src_char[i];
    }
    
    /* Copy 4 chars via memcpy with constant size */
    memcpy(&dst_char[4], &src_char[4], 4 * sizeof(char));
    
    /* Copy 3 ints = 12 bytes (might be small depending on threshold) */
    for (int i = 6; i <= 8; ++i) {  /* 3 elements */
        dst_int[i] = src_int[i];
    }
}

/* Test 3: Constant bounds, larger total size */
NOINLINE static void test_const_large_total_size(void) {
    /* Copy 3 long longs = 24 bytes (likely above inline threshold on 64-bit) */
    for (int i = 0; i <= 2; ++i) {  /* 3 elements */
        dst_ll[i] = src_ll[i];
    }
    
    /* memcpy with constant size 24 bytes */
    memcpy(&dst_ll[2], &src_ll[2], 3 * sizeof(long long));
}

/* Test 4: Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* Single element copy to register variable */
    int reg1 = src_int[0];      /* Should trigger !MEM_P(target) */
    char reg2 = src_char[0];    /* Another register target */
    long long reg3 = src_ll[0]; /* Large register target */
    
    /* Use the values to prevent elimination */
    dst_int[0] = reg1;
    dst_char[0] = reg2;
    dst_ll[0] = reg3;
    
    /* Two-element copy to separate registers */
    int reg4 = src_int[1];
    int reg5 = src_int[2];
    dst_int[1] = reg4;
    dst_int[2] = reg5;
}

/* Test 5: Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct (constant size) */
    dst_struct[0] = src_struct[0];  /* Single struct assignment */
    
    /* Copy two structs via loop */
    for (int i = 1; i <= 2; ++i) {  /* 2 elements */
        dst_struct[i] = src_struct[i];
    }
    
    /* Copy struct via memcpy */
    memcpy(&dst_struct[2], &src_struct[2], sizeof(struct SmallStruct));
    
    /* Union copy */
    dst_union = src_union;
}

/* Test 6: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - should go to library call path */
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
        }
    }
    
    /* memcpy with variable size */
    int count = end - start + 1;
    if (count > 0 && count <= 10) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
    }
}

/* Test 7: Mixed scenarios with different element types */
NOINLINE static void test_mixed_scenarios(void) {
    /* Copy 1 element of different types */
    dst_int[9] = src_int[9];
    dst_char[19] = src_char[19];
    dst_ll[4] = src_ll[4];
    
    /* Copy exactly 2 bytes (two chars) */
    dst_char[10] = src_char[10];
    dst_char[11] = src_char[11];
    
    /* Copy exactly 2 ints (8 bytes on 32-bit, 8 bytes on 64-bit) */
    memcpy(&dst_int[7], &src_int[7], 2 * sizeof(int));
    
    /* Copy 3 bytes via three char assignments */
    dst_char[12] = src_char[12];
    dst_char[13] = src_char[13];
    dst_char[14] = src_char[14];
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        sum += dst_int[i];
    }
    
    for (int i = 0; i < 20; i++) {
        sum += dst_char[i];
    }
    
    for (int i = 0; i < 5; i++) {
        sum += (int)(dst_ll[i] % 1000);
    }
    
    for (int i = 0; i < 3; i++) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    sum += (int)(dst_union.ll % 1000);
    
    return sum;
}

int main(void) {
    /* Execute all test cases */
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_large_total_size();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(0, 2);  /* Variable bounds */
    test_variable_bounds(3, 5);  /* Another variable bounds case */
    test_mixed_scenarios();
    
    /* Compute and return checksum */
    return compute_checksum() % 256;
}

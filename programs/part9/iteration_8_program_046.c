/* test_expr_coverage.c
 * Designed to trigger GCC's inline copy expansion logic in expr.cc
 * Compile with: gcc -std=gnu11 -O2 -fno-builtin-memcpy -fno-tree-loop-distribute-patterns -fprofile-arcs -ftest-coverage
 */

#include <stdint.h>
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

static struct SmallStruct src_struct[3] = {
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30}
};
static struct SmallStruct dst_struct[3] = {0};

/* Union for testing different type sizes */
union MixedUnion {
    char c[8];
    int i[2];
    long long ll;
};

static union MixedUnion src_union = {.ll = 0x0123456789ABCDEFULL};
static union MixedUnion dst_union = {0};

/* Test 1: Constant small memcpy - should trigger count <= 2 or small size branch */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 ints - count <= 2 branch */
    for (int i = 2; i <= 3; ++i) {  // Constant bounds: i=2 to i=3 inclusive
        dst_int[i] = src_int[i];
    }
    
    /* Copy 3 chars - small total size (3 bytes) */
    for (int i = 0; i <= 2; ++i) {  // Constant bounds
        dst_char[i] = src_char[i];
    }
    
    /* Alternative using memcpy with constant size 2 */
    memcpy(&dst_int[5], &src_int[5], 2 * sizeof(int));  // 8 bytes on 64-bit
}

/* Test 2: Constant larger copy - should NOT use inline expansion if size > threshold */
NOINLINE static void test_const_larger_copy(void) {
    /* Copy 3 long longs = 24 bytes on 64-bit - might exceed threshold */
    for (int i = 0; i <= 2; ++i) {  // Constant bounds
        dst_ll[i] = src_ll[i];
    }
    
    /* Copy 5 ints = 20 bytes */
    for (int i = 1; i <= 5; ++i) {  // Constant bounds: i=1 to i=5
        dst_int[i] = src_int[i];
    }
}

/* Test 3: Register target (!MEM_P(target)) - copy to scalar variable */
NOINLINE static void test_register_target(void) {
    /* Single element copy to register */
    int reg1 = src_int[3];  // Should trigger !MEM_P(target) path
    
    /* Multiple register copies */
    char reg2 = src_char[5];
    long long reg3 = src_ll[2];
    
    /* Use the values to prevent elimination */
    dst_int[0] = reg1;
    dst_char[0] = reg2;
    dst_ll[0] = reg3;
}

/* Test 4: Struct copies - aggregate types */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct - small aggregate */
    dst_struct[0] = src_struct[0];  // Should use inline copy
    
    /* Copy 2 structs - count <= 2 */
    for (int i = 0; i <= 1; ++i) {  // Constant bounds
        dst_struct[i] = src_struct[i];
    }
    
    /* Union copy */
    dst_union = src_union;
}

/* Test 5: Variable bounds - should NOT trigger const_bounds_p path */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - forces library call path */
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
        }
    }
    
    /* Variable-sized memcpy */
    int count = end - start + 1;
    if (count > 0 && count <= 10) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
    }
}

/* Test 6: Mixed scenarios with different element sizes */
NOINLINE static void test_mixed_scenarios(void) {
    /* Copy 1 element - trivial case */
    dst_int[9] = src_int[9];
    
    /* Copy 2 elements with pointer arithmetic */
    int *src_ptr = &src_int[4];
    int *dst_ptr = &dst_int[4];
    for (int i = 0; i <= 1; ++i) {
        dst_ptr[i] = src_ptr[i];
    }
    
    /* Copy partial array with constant offset */
    memcpy(dst_char + 5, src_char + 5, 3);  // 3 bytes
}

/* Test 7: Edge cases with different types */
NOINLINE static void test_edge_cases(void) {
    /* Copy single byte */
    dst_char[10] = src_char[10];
    
    /* Copy two bytes */
    dst_char[11] = src_char[11];
    dst_char[12] = src_char[12];
    
    /* Copy with different integer types */
    int16_t src_16[4] = {10, 20, 30, 40};
    int16_t dst_16[4] = {0};
    
    for (int i = 0; i <= 2; ++i) {  // Copy 3 elements = 6 bytes
        dst_16[i] = src_16[i];
    }
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        sum += dst_int[i];
        if (i < 20) sum += dst_char[i];
        if (i < 5) sum += (int)dst_ll[i];
        if (i < 3) {
            sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
        }
    }
    
    sum += dst_union.c[0] + dst_union.i[0];
    return sum;
}

int main(void) {
    /* Run all test cases */
    test_const_small_memcpy();
    test_const_larger_copy();
    test_register_target();
    test_struct_copy();
    
    /* Variable bounds tests with different ranges */
    test_variable_bounds(2, 4);    // Constant range but passed as args
    test_variable_bounds(0, 1);    // Small range
    test_variable_bounds(3, 7);    // Larger range
    
    test_mixed_scenarios();
    test_edge_cases();
    
    /* Return checksum to ensure all code executes */
    return compute_checksum() > 0 ? 0 : 1;
}

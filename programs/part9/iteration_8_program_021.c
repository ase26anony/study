/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test patterns */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0,1,2,3,4,5,6,7,8,9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789ABCDEFGHI";
static char dst_char[20] = {0};
static long long src_ll[5] = {100,200,300,400,500};
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

/* Union for testing */
union TestUnion {
    int i;
    float f;
    char c[4];
};

static union TestUnion src_union = {.i = 0x12345678};
static union TestUnion dst_union;

/* Test 1: Constant bounds, count <= 2, MEM_P target */
NOINLINE static void test_const_small_memcpy(void) {
    /* These should trigger count <= 2 path */
    
    /* Copy 1 element - count == 1 */
    dst_int[0] = src_int[0];
    
    /* Copy 2 elements - count == 2 */
    dst_int[1] = src_int[1];
    dst_int[2] = src_int[2];
    
    /* memcpy with constant size 2*sizeof(int) = 8 bytes (assuming 32-bit) */
    memcpy(&dst_int[3], &src_int[3], 2 * sizeof(int));
    
    /* Copy 2 chars - small total size */
    dst_char[0] = src_char[0];
    dst_char[1] = src_char[1];
}

/* Test 2: Constant bounds, count > 2 but small total size */
NOINLINE static void test_const_small_total_size(void) {
    /* Copy 3 chars = 3 bytes total - should trigger size-based inline */
    for (int i = 2; i <= 4; ++i) {  /* Constant bounds: i=2 to i=4 */
        dst_char[i] = src_char[i];
    }
    
    /* Copy 4 chars = 4 bytes */
    memcpy(&dst_char[5], &src_char[5], 4 * sizeof(char));
    
    /* Copy small struct (size likely 12 bytes with padding) */
    dst_struct[0] = src_struct[0];
    dst_struct[1] = src_struct[1];
}

/* Test 3: Constant bounds, larger total size (should use library call) */
NOINLINE static void test_const_large_total_size(void) {
    /* Copy 3 long longs = 24 bytes (on 64-bit) - likely exceeds threshold */
    for (int i = 0; i <= 2; ++i) {  /* Constant bounds */
        dst_ll[i] = src_ll[i];
    }
    
    /* memcpy with size 24 bytes */
    memcpy(&dst_ll[3], &src_ll[3], 2 * sizeof(long long));
}

/* Test 4: Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* These should trigger !MEM_P(target) path */
    int temp1 = src_int[3];      /* Load into register */
    int temp2 = src_int[4];
    dst_int[5] = temp1;          /* Store from register */
    dst_int[6] = temp2;
    
    /* Multiple register loads */
    char c1 = src_char[10];
    char c2 = src_char[11];
    char c3 = src_char[12];
    dst_char[10] = c1;
    dst_char[11] = c2;
    dst_char[12] = c3;
}

/* Test 5: Struct and union copies */
NOINLINE static void test_struct_union_copy(void) {
    /* Struct assignment - aggregate type */
    struct SmallStruct local_struct = src_struct[0];
    dst_struct[2] = local_struct;
    
    /* Union copy */
    dst_union = src_union;
    
    /* Array of structs with constant bounds */
    for (int i = 0; i <= 1; ++i) {
        dst_struct[i].a = src_struct[i].a;
        dst_struct[i].b = src_struct[i].b;
        dst_struct[i].c = src_struct[i].c;
    }
}

/* Test 6: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Variable bounds - should go to library call path */
    for (int i = start; i < end; ++i) {
        dst_int[i] = src_int[i];
    }
    
    /* Variable size memcpy */
    int count = end - start;
    if (count > 0) {
        memcpy(&dst_char[start], &src_char[start], count * sizeof(char));
    }
}

/* Test 7: Mixed constant/variable patterns */
NOINLINE static void test_mixed_patterns(void) {
    /* Constant slice in middle of array */
    memcpy(&dst_int[2], &src_int[2], 3 * sizeof(int));
    
    /* Multiple small constant copies */
    dst_char[15] = src_char[15];
    dst_char[16] = src_char[16];
    dst_char[17] = src_char[17];
    
    /* Small struct with padding */
    memcpy(&dst_struct[0], &src_struct[0], sizeof(struct SmallStruct));
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
        sum += (int)(dst_ll[i] % 256);
    }
    
    for (int i = 0; i < 3; i++) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
    }
    
    sum += dst_union.i % 256;
    
    return sum;
}

int main(void) {
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_small_total_size();
    test_const_large_total_size();
    test_register_target();
    test_struct_union_copy();
    test_variable_bounds(1, 4);  /* Variable bounds */
    test_mixed_patterns();
    
    /* Additional variable bounds test */
    test_variable_bounds(0, 3);
    
    /* Return checksum to ensure all code executes */
    return compute_checksum() > 0 ? 0 : 1;
}

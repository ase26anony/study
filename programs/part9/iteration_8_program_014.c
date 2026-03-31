/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to keep function boundaries clear for coverage */
#define NOINLINE __attribute__((noinline))

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
    {'a', 1, 10},
    {'b', 2, 20},
    {'c', 3, 30}
};
static struct SmallStruct dst_struct[3] = {0};

/* Union for testing */
union TestUnion {
    int i;
    float f;
    char arr[4];
};

static union TestUnion src_union = {.i = 0x12345678};
static union TestUnion dst_union;

/* Test 1: Constant bounds, count <= 2, memory target */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 ints - should trigger count <= 2 branch */
    for (int i = 2; i <= 3; ++i) {  /* lo=2, hi=3, count=2 */
        dst_int[i] = src_int[i];
    }
    
    /* Copy 1 char - count=1 */
    dst_char[0] = src_char[0];
    
    /* Copy 2 chars via memcpy with constant size */
    memcpy(&dst_char[5], &src_char[5], 2 * sizeof(char));
}

/* Test 2: Constant bounds, small total byte size */
NOINLINE static void test_const_small_byte_size(void) {
    /* Copy 3 chars = 3 bytes total - small size branch */
    for (int i = 1; i <= 3; ++i) {  /* lo=1, hi=3, count=3 */
        dst_char[i] = src_char[i];
    }
    
    /* Copy 3 ints on 32-bit = 12 bytes, might be threshold */
    for (int i = 0; i <= 2; ++i) {
        dst_int[i] = src_int[i];
    }
}

/* Test 3: Constant bounds, larger byte size */
NOINLINE static void test_const_large_byte_size(void) {
    /* Copy 3 long longs = 24 bytes on 64-bit - likely exceeds threshold */
    for (int i = 0; i <= 2; ++i) {  /* lo=0, hi=2, count=3 */
        dst_ll[i] = src_ll[i];
    }
    
    /* memcpy with constant size that's larger */
    memcpy(&dst_int[3], &src_int[3], 4 * sizeof(int));  /* 16 bytes on 32-bit */
}

/* Test 4: Non-MEM_P target (register) */
NOINLINE static void test_register_target(void) {
    /* Single element copy to register variable */
    int reg1 = src_int[3];  /* Should trigger !MEM_P(target) */
    
    /* Two elements to separate registers */
    int reg2 = src_int[4];
    int reg3 = src_int[5];
    
    /* Use the values to prevent elimination */
    dst_int[0] = reg1 + reg2 + reg3;
}

/* Test 5: Struct copies - aggregate types */
NOINLINE static void test_struct_copy(void) {
    /* Copy single struct - count=1 */
    dst_struct[0] = src_struct[0];
    
    /* Copy 2 structs - count=2 */
    for (int i = 0; i <= 1; ++i) {
        dst_struct[i] = src_struct[i];
    }
    
    /* Union copy */
    dst_union = src_union;
}

/* Test 6: Variable bounds - should NOT trigger const_bounds_p */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - const_bounds_p should be false */
    for (int i = start; i <= end; ++i) {
        if (i >= 0 && i < 10) {
            dst_int[i] = src_int[i];
        }
    }
    
    /* memcpy with variable size */
    int count = end - start + 1;
    if (count > 0 && count <= 10) {
        memcpy(&dst_char[10], &src_char[10], count * sizeof(char));
    }
}

/* Test 7: Mixed patterns to hit various conditions */
NOINLINE static void test_mixed_patterns(void) {
    /* Pattern 1: Direct assignment of 2 elements */
    dst_int[5] = src_int[5];
    dst_int[6] = src_int[6];  /* Two separate assignments */
    
    /* Pattern 2: Small memcpy in loop with constant bounds */
    for (int i = 7; i <= 9; ++i) {  /* lo=7, hi=9, count=3 */
        dst_int[i] = src_int[i];
    }
    
    /* Pattern 3: Array slice with pointer arithmetic */
    int *src_ptr = &src_int[2];
    int *dst_ptr = &dst_int[2];
    for (int i = 0; i < 2; ++i) {  /* count=2 */
        dst_ptr[i] = src_ptr[i];
    }
}

/* Checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    
    for (int i = 0; i < 10; ++i) {
        sum += dst_int[i];
        sum += src_int[i];
    }
    
    for (int i = 0; i < 20; ++i) {
        sum += dst_char[i];
        sum += src_char[i];
    }
    
    for (int i = 0; i < 5; ++i) {
        sum += (int)(dst_ll[i] % 256);
        sum += (int)(src_ll[i] % 256);
    }
    
    for (int i = 0; i < 3; ++i) {
        sum += dst_struct[i].a + dst_struct[i].b + dst_struct[i].c;
        sum += src_struct[i].a + src_struct[i].b + src_struct[i].c;
    }
    
    sum += dst_union.arr[0] + src_union.arr[0];
    
    return sum;
}

int main(void) {
    /* Clear destination arrays */
    memset(dst_int, 0, sizeof(dst_int));
    memset(dst_char, 0, sizeof(dst_char));
    memset(dst_ll, 0, sizeof(dst_ll));
    memset(dst_struct, 0, sizeof(dst_struct));
    
    /* Execute all test patterns */
    test_const_small_memcpy();
    test_const_small_byte_size();
    test_const_large_byte_size();
    test_register_target();
    test_struct_copy();
    test_variable_bounds(1, 3);  /* Variable bounds */
    test_mixed_patterns();
    
    /* Compute and return checksum */
    return compute_checksum() % 256;
}

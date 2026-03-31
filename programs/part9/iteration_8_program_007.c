/* test_expr_coverage.c - Target GCC expr.cc lines 7691-7700 */
#include <stdio.h>
#include <string.h>

/* Global arrays to avoid aliasing issues */
static int src_int[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int dst_int[10] = {0};
static char src_char[20] = "0123456789ABCDEFGHI";
static char dst_char[20] = {0};
static long long src_ll[5] = {100LL, 200LL, 300LL, 400LL, 500LL};
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

/* Test 1: Constant small memcpy - should trigger count <= 2 or small size */
__attribute__((noinline))
static int test_const_small_memcpy(void) {
    int sum = 0;
    
    /* Copy 1 element - count == 1 */
    memcpy(&dst_int[0], &src_int[0], sizeof(int));
    sum += dst_int[0];
    
    /* Copy 2 elements - count == 2 */
    memcpy(&dst_int[1], &src_int[1], 2 * sizeof(int));
    sum += dst_int[1] + dst_int[2];
    
    /* Copy 3 chars - small total size (3 bytes) */
    memcpy(&dst_char[0], &src_char[0], 3);
    sum += dst_char[0] + dst_char[1] + dst_char[2];
    
    /* Copy 3 ints - larger total size (12 bytes on 32-bit, 12 bytes on 64-bit) */
    memcpy(&dst_int[3], &src_int[3], 3 * sizeof(int));
    sum += dst_int[3] + dst_int[4] + dst_int[5];
    
    return sum;
}

/* Test 2: Constant large memcpy - should exceed inline threshold */
__attribute__((noinline))
static int test_const_large_memcpy(void) {
    int sum = 0;
    
    /* Copy 5 long longs - 40 bytes on 64-bit, likely exceeds threshold */
    memcpy(dst_ll, src_ll, 5 * sizeof(long long));
    for (int i = 0; i < 5; i++) {
        sum += (int)(dst_ll[i] % 256);
    }
    
    /* Copy 10 chars using array indices with constant bounds */
    int lo = 0, hi = 9;  /* These are constants at compile time */
    memcpy(&dst_char[lo], &src_char[lo], (hi - lo + 1) * sizeof(char));
    for (int i = lo; i <= hi; i++) {
        sum += dst_char[i];
    }
    
    return sum;
}

/* Test 3: Register target (!MEM_P(target)) - scalar assignment */
__attribute__((noinline))
static int test_register_target(void) {
    int sum = 0;
    
    /* Copy single element to register variable */
    int reg1 = src_int[3];  /* Should trigger !MEM_P(target) */
    sum += reg1;
    
    /* Copy two elements to separate registers */
    int reg2 = src_int[4];
    int reg3 = src_int[5];
    sum += reg2 + reg3;
    
    /* Copy struct member to register */
    short reg4 = src_struct[0].c;
    sum += reg4;
    
    return sum;
}

/* Test 4: Struct and union copies */
__attribute__((noinline))
static int test_struct_copy(void) {
    int sum = 0;
    
    /* Copy entire struct - constant size aggregate */
    dst_struct[0] = src_struct[0];
    sum += dst_struct[0].b;
    
    /* Copy 2 structs using memcpy with constant size */
    memcpy(&dst_struct[1], &src_struct[1], 2 * sizeof(struct SmallStruct));
    sum += dst_struct[1].b + dst_struct[2].b;
    
    /* Copy union */
    dst_union = src_union;
    sum += dst_union.arr[0];
    
    /* Copy struct array slice with constant bounds */
    for (int i = 0; i <= 1; i++) {  /* Constant bounds: i=0 to i=1 */
        dst_struct[i].a = src_struct[i].a;
        sum += dst_struct[i].a;
    }
    
    return sum;
}

/* Test 5: Variable bounds - should NOT trigger const_bounds_p */
__attribute__((noinline))
static int test_variable_bounds(int start, int end) {
    int sum = 0;
    
    /* Copy with variable bounds - forces library call path */
    if (start >= 0 && end < 10 && start <= end) {
        for (int i = start; i <= end; i++) {
            dst_int[i] = src_int[i];
            sum += dst_int[i];
        }
    }
    
    /* Variable-sized memcpy */
    int count = end - start + 1;
    if (count > 0 && count <= 5) {
        memcpy(&dst_char[start], &src_char[start], count);
        for (int i = 0; i < count; i++) {
            sum += dst_char[start + i];
        }
    }
    
    return sum;
}

/* Test 6: Mixed constant-bound array copies with different element sizes */
__attribute__((noinline))
static int test_mixed_const_copies(void) {
    int sum = 0;
    
    /* Copy 3 chars using explicit loop with constant bounds */
    for (int i = 2; i <= 4; ++i) {  /* i=2,3,4 -> count=3 */
        dst_char[i] = src_char[i];
        sum += dst_char[i];
    }
    
    /* Copy 2 ints using explicit loop with constant bounds */
    for (int i = 5; i <= 6; ++i) {  /* i=5,6 -> count=2 */
        dst_int[i] = src_int[i];
        sum += dst_int[i];
    }
    
    /* Copy 3 long longs - large total size */
    for (int i = 1; i <= 3; ++i) {  /* i=1,2,3 -> count=3 */
        dst_ll[i] = src_ll[i];
        sum += (int)(dst_ll[i] % 256);
    }
    
    return sum;
}

int main(void) {
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test_const_small_memcpy();
    total_sum += test_const_large_memcpy();
    total_sum += test_register_target();
    total_sum += test_struct_copy();
    total_sum += test_variable_bounds(2, 4);  /* Variable bounds */
    total_sum += test_mixed_const_copies();
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", total_sum);
    
    return total_sum == 0 ? 1 : 0;  /* Return non-zero if all tests passed */
}

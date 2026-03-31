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
    char d;
};

static struct SmallStruct struct_src[4] = {
    {'a', 1, 10, 'x'},
    {'b', 2, 20, 'y'},
    {'c', 3, 30, 'z'},
    {'d', 4, 40, 'w'}
};
static struct SmallStruct struct_dst[4];

/* Test 1: Constant small memcpy (count <= 2 or small total size) */
NOINLINE static void test_const_small_memcpy(void) {
    /* Copy exactly 2 chars (count <= 2, small size) */
    char_dst[0] = char_src[0];
    char_dst[1] = char_src[1];
    
    /* Copy 3 chars via memcpy (count > 2 but total size = 3 bytes) */
    memcpy(&char_dst[2], &char_src[2], 3);
    
    /* Copy 2 ints (count <= 2) */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    
    /* Copy 3 chars using loop with constant bounds */
    for (int i = 5; i <= 7; ++i) {
        char_dst[i] = char_src[i];
    }
}

/* Test 2: Constant bounds but larger copy size */
NOINLINE static void test_const_larger_memcpy(void) {
    /* Copy 3 long longs (24 bytes on 64-bit) - tests size threshold */
    for (int i = 0; i <= 2; ++i) {
        ll_dst[i] = ll_src[i];
    }
    
    /* Copy 4 ints (16 bytes) using memcpy with constant size */
    memcpy(&int_dst[4], &int_src[4], 4 * sizeof(int));
    
    /* Copy 8 chars (8 bytes) - still relatively small */
    for (int i = 8; i <= 15; ++i) {
        char_dst[i] = char_src[i];
    }
}

/* Test 3: Register target (!MEM_P(target)) */
NOINLINE static void test_register_target(void) {
    /* Copy single elements into register variables */
    int reg1 = int_src[3];           /* Single element to register */
    long long reg2 = ll_src[2];      /* Another to register */
    char reg3 = char_src[10];        /* Char to register */
    
    /* Use the values to prevent elimination */
    int_dst[3] = reg1;
    ll_dst[2] = reg2;
    char_dst[10] = reg3;
    
    /* Copy two elements to separate registers */
    int reg4 = int_src[5];
    int reg5 = int_src[6];
    int_dst[5] = reg4 + reg5;
}

/* Test 4: Struct copies (aggregate types) */
NOINLINE static void test_struct_copy(void) {
    /* Copy entire struct (constant size, small) */
    struct_dst[0] = struct_src[0];
    
    /* Copy two structs (count = 2) */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    
    /* Copy struct via memcpy with constant size */
    memcpy(&struct_dst[3], &struct_src[3], sizeof(struct SmallStruct));
    
    /* Copy struct field by field (creates multiple small copies) */
    struct_dst[0].a = struct_src[0].a;
    struct_dst[0].b = struct_src[0].b;
    struct_dst[0].c = struct_src[0].c;
    struct_dst[0].d = struct_src[0].d;
}

/* Test 5: Variable bounds (should NOT trigger const_bounds_p) */
NOINLINE static void test_variable_bounds(int start, int end) {
    /* Loop with variable bounds - forces library call path */
    for (int i = start; i < end && i < 16; ++i) {
        int_dst[i] = int_src[i];
    }
    
    /* Variable-sized memcpy */
    int count = end - start;
    if (count > 0 && count < 8) {
        memcpy(&char_dst[start], &char_src[start], count);
    }
}

/* Test 6: Mixed constant/variable patterns */
NOINLINE static void test_mixed_patterns(void) {
    /* Constant slice in the middle of array */
    for (int i = 10; i <= 12; ++i) {
        ll_dst[i-10] = ll_src[i-10];  /* Constant bounds 0-2 */
    }
    
    /* Multiple small constant copies */
    char_dst[20] = char_src[20];
    char_dst[21] = char_src[21];
    char_dst[22] = char_src[22];  /* Three separate assignments */
    
    /* Small memcpy with constant size 4 */
    memcpy(&int_dst[8], &int_src[8], 4);
}

/* Compute checksum to prevent dead code elimination */
NOINLINE static int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 32; ++i) sum += char_dst[i];
    for (int i = 0; i < 16; ++i) sum += int_dst[i];
    for (int i = 0; i < 8; ++i) sum += (int)ll_dst[i];
    for (int i = 0; i < 4; ++i) {
        sum += struct_dst[i].a + struct_dst[i].b + struct_dst[i].c + struct_dst[i].d;
    }
    return sum;
}

int main(void) {
    /* Clear destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Execute all test patterns */
    test_const_small_memcpy();      /* Hits count <= 2 and small size paths */
    test_const_larger_memcpy();     /* Tests size threshold logic */
    test_register_target();         /* Tests !MEM_P(target) path */
    test_struct_copy();             /* Tests aggregate types */
    test_variable_bounds(2, 5);     /* Forces non-const path */
    test_mixed_patterns();          /* Additional constant patterns */
    
    /* Use results to prevent optimization */
    return compute_checksum() & 0xFF;
}

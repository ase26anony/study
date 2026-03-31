/* Test program to cover GCC's expression expansion logic for array/memory copies */
#include <stdint.h>
#include <string.h>

#define NOINLINE __attribute__((noinline))

/* Global arrays to avoid aliasing issues */
static char char_src[32] = "abcdefghijklmnopqrstuvwxyz012345";
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

/* 1. Constant small memcpy - should trigger count <= 2 or small size branch */
NOINLINE static int test_const_small_memcpy(void) {
    int sum = 0;
    
    /* Copy 1 element - count <= 2 branch */
    char_dst[0] = char_src[0];
    sum += char_dst[0];
    
    /* Copy 2 elements - count <= 2 branch */
    char_dst[1] = char_src[1];
    char_dst[2] = char_src[2];
    sum += char_dst[1] + char_dst[2];
    
    /* Copy 3 chars = 3 bytes - small total size branch */
    for (int i = 3; i <= 5; ++i) {  /* Constant bounds: i=3 to i=5 */
        char_dst[i] = char_src[i];
        sum += char_dst[i];
    }
    
    /* Copy 2 ints - count <= 2 branch */
    int_dst[0] = int_src[0];
    int_dst[1] = int_src[1];
    sum += int_dst[0] + int_dst[1];
    
    return sum;
}

/* 2. Constant "large" memcpy - total size may exceed threshold */
NOINLINE static int test_const_large_memcpy(void) {
    int sum = 0;
    
    /* Copy 3 long longs = 24 bytes (may exceed threshold) */
    for (int i = 0; i <= 2; ++i) {  /* Constant bounds: i=0 to i=2 */
        ll_dst[i] = ll_src[i];
        sum += (int)(ll_dst[i] % 256);
    }
    
    /* Copy 5 ints = 20 bytes */
    for (int i = 3; i <= 7; ++i) {  /* Constant bounds: i=3 to i=7 */
        int_dst[i] = int_src[i];
        sum += int_dst[i];
    }
    
    return sum;
}

/* 3. Register target - !MEM_P(target) branch */
NOINLINE static int test_register_target(void) {
    int sum = 0;
    
    /* Copy to register variables */
    int reg1 = int_src[0];          /* Single element to register */
    char reg2 = char_src[1];        /* Another to register */
    long long reg3 = ll_src[2];     /* Large type to register */
    
    sum += reg1 + reg2 + (int)(reg3 % 256);
    
    /* Copy from array to multiple registers */
    int temp1 = int_src[10];
    int temp2 = int_src[11];
    sum += temp1 + temp2;
    
    return sum;
}

/* 4. Struct copy - aggregate types */
NOINLINE static int test_struct_copy(void) {
    int sum = 0;
    
    /* Copy single struct (constant size ~12 bytes with padding) */
    struct_dst[0] = struct_src[0];
    sum += struct_dst[0].b + struct_dst[0].c;
    
    /* Copy 2 structs - count <= 2 branch */
    struct_dst[1] = struct_src[1];
    struct_dst[2] = struct_src[2];
    sum += struct_dst[1].b + struct_dst[2].b;
    
    /* Copy struct slice with constant bounds */
    for (int i = 1; i <= 3; ++i) {  /* Constant bounds: i=1 to i=3 */
        struct_dst[i].a = struct_src[i].a;
        struct_dst[i].d = struct_src[i].d;
        sum += struct_dst[i].a + struct_dst[i].d;
    }
    
    return sum;
}

/* 5. Variable bounds - should NOT trigger const_bounds_p */
NOINLINE static int test_variable_bounds(int start, int end) {
    int sum = 0;
    
    /* Variable bounds - compiler can't know these at compile time */
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < 16) {
            int_dst[i] = int_src[i];
            sum += int_dst[i];
        }
    }
    
    /* Another variable copy */
    int n = end - start;
    if (n > 0 && n <= 8) {
        for (int i = 0; i < n; ++i) {
            char_dst[i] = char_src[i + start];
            sum += char_dst[i];
        }
    }
    
    return sum;
}

/* 6. Additional tests for edge cases */
NOINLINE static int test_edge_cases(void) {
    int sum = 0;
    
    /* Exactly 2 elements copy using memcpy with constant size */
    memcpy(&int_dst[8], &int_src[8], 2 * sizeof(int));
    sum += int_dst[8] + int_dst[9];
    
    /* Single element memcpy */
    memcpy(&ll_dst[3], &ll_src[3], sizeof(long long));
    sum += (int)(ll_dst[3] % 256);
    
    /* Small union copy */
    union {
        int i;
        float f;
        char c[4];
    } u_src = {0x12345678}, u_dst;
    
    u_dst = u_src;  /* Whole union copy */
    sum += u_dst.i & 0xFF;
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Clear destination arrays */
    memset(char_dst, 0, sizeof(char_dst));
    memset(int_dst, 0, sizeof(int_dst));
    memset(ll_dst, 0, sizeof(ll_dst));
    memset(struct_dst, 0, sizeof(struct_dst));
    
    /* Run all tests */
    checksum += test_const_small_memcpy();
    checksum += test_const_large_memcpy();
    checksum += test_register_target();
    checksum += test_struct_copy();
    checksum += test_variable_bounds(2, 5);  /* Variable bounds */
    checksum += test_edge_cases();
    
    /* Use results to prevent dead code elimination */
    volatile int result = checksum;
    
    return result != 0 ? 0 : 1;
}

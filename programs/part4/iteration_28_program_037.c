/* expr_cc_coverage.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc (lines 7691-7700) by creating various initialization
 * patterns with constant array/struct bounds.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers expr_cc_coverage.c -o expr_cc_coverage
 *   gcc -O2 -ftree-vectorize -std=gnu11 expr_cc_coverage.c -o expr_cc_coverage_opt
 */

#include <stdio.h>
#include <string.h>

/* ==================== 1. CONSTANT BOUNDS DEFINITIONS ==================== */
enum { 
    L = 2, 
    H = 5,
    BIG_START = 10,
    BIG_END = 90,
    SMALL_COUNT = 2
};

const int C_LOW = 0;
const int C_HIGH = 3;

/* ==================== 2. ELEMENT TYPES WITH CONSTANT SIZES ==================== */
/* Basic types with constant sizes */
typedef char small_type;
typedef int medium_type;
typedef long large_type;

/* Packed struct with constant bitfield size */
struct packed_bitfield {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 16;
} __attribute__((packed));

/* Struct containing array */
struct with_array {
    int header;
    int data[8];
    char footer;
};

/* ==================== 3. TARGET CONDITION VARIATIONS ==================== */

/* Case A: count <= 2 (path taken regardless of MEM_P(target)) */
static void test_count_le_2(void) {
    printf("=== Testing count <= 2 ===\n");
    
    /* Exactly 2 elements, constant bounds */
    int arr1[10] = {[C_LOW ... C_LOW+1] = 42};  /* count = 2 */
    printf("arr1[0]=%d, arr1[1]=%d\n", arr1[0], arr1[1]);
    
    /* Single element, constant bound */
    int arr2[100] = {[50] = 99};  /* count = 1 */
    printf("arr2[50]=%d\n", arr2[50]);
    
    /* With enum constants */
    int arr3[] = {[L ... L+1] = 7};  /* count = 2 */
    printf("arr3[2]=%d, arr3[3]=%d\n", arr3[2], arr3[3]);
}

/* Case B: !MEM_P(target) - register target */
static void test_register_target(void) {
    printf("\n=== Testing !MEM_P(target) (register) ===\n");
    
    /* Use 'register' keyword to encourage register allocation */
    register int reg_target = ({ 
        int temp[] = {[0 ... 1] = 123};  /* count = 2, constant bounds */
        temp[0] + temp[1];
    });
    printf("reg_target sum: %d\n", reg_target);
    
    /* Small struct that might go in register */
    register struct { int x; int y; } reg_struct = {
        .x = ({ int t[] = {[0] = 5}; t[0]; }),
        .y = ({ int t[] = {[0 ... 1] = 3}; t[0] + t[1]; })
    };
    printf("reg_struct: x=%d, y=%d\n", reg_struct.x, reg_struct.y);
}

/* Case C: MEM_P(target) && count > 2 && constant element size */
static void test_mem_target_large_count(void) {
    printf("\n=== Testing MEM_P(target) with count > 2 ===\n");
    
    /* Large static array with wide constant range */
    static int big_array[100] = {[BIG_START ... BIG_END] = 0xABCD};
    printf("big_array[%d]=0x%X, big_array[%d]=0x%X\n", 
           BIG_START, big_array[BIG_START], 
           BIG_END, big_array[BIG_END]);
    
    /* Different element type with constant size */
    static char char_array[1000] = {[100 ... 500] = 'X'};
    printf("char_array[100]='%c', char_array[500]='%c'\n", 
           char_array[100], char_array[500]);
    
    /* Packed struct array with constant range */
    static struct packed_bitfield bit_array[50] = {
        [10 ... 20] = { .a = 0x7F, .b = 0x1FF, .c = 0xFFFF }
    };
    printf("bit_array[10].a=0x%X\n", bit_array[10].a);
}

/* Case D: Mixed conditions with volatile (ensures MEM_P) */
static void test_volatile_target(void) {
    printf("\n=== Testing volatile target (definitely MEM_P) ===\n");
    
    volatile int vol_array[10] = {[1 ... 4] = 999};  /* count = 4 > 2 */
    printf("vol_array[1]=%d, vol_array[4]=%d\n", vol_array[1], vol_array[4]);
    
    volatile struct with_array vol_struct = {
        .data = {[2 ... 5] = 777}  /* count = 4 > 2 */
    };
    printf("vol_struct.data[2]=%d, vol_struct.data[5]=%d\n", 
           vol_struct.data[2], vol_struct.data[5]);
}

/* ==================== 4. MULTI-DIMENSIONAL AND NESTED ==================== */
static void test_multi_dimensional(void) {
    printf("\n=== Testing multi-dimensional arrays ===\n");
    
    /* 2D array with constant range in both dimensions */
    int matrix[5][5] = {[0 ... 2][1 ... 3] = 88};
    printf("matrix[0][1]=%d, matrix[2][3]=%d\n", matrix[0][1], matrix[2][3]);
    
    /* 3D array with constant ranges */
    int cube[3][3][3] = {[0 ... 1][0 ... 1][0 ... 1] = 42};
    printf("cube[0][0][0]=%d, cube[1][1][1]=%d\n", cube[0][0][0], cube[1][1][1]);
    
    /* Nested struct with array initialization */
    struct outer {
        int id;
        struct with_array inner;
    } nested = {
        .id = 1,
        .inner.data = {[0 ... 7] = 111}  /* count = 8 > 2 */
    };
    printf("nested.inner.data[0]=%d, nested.inner.data[7]=%d\n", 
           nested.inner.data[0], nested.inner.data[7]);
}

/* ==================== 5. COMPOUND LITERALS AND ATTRIBUTES ==================== */
static void test_compound_literals(void) {
    printf("\n=== Testing compound literals ===\n");
    
    /* Compound literal assignment - creates initialization context */
    struct with_array *ptr = &(struct with_array){
        .data = {[1 ... 4] = 222}  /* count = 4 > 2 */
    };
    printf("compound literal data[1]=%d\n", ptr->data[1]);
    
    /* Aligned array might affect MEM_P classification */
    int aligned_array[8] __attribute__((aligned(64))) = {[0 ... 3] = 333};
    printf("aligned_array[0]=%d (alignment 64)\n", aligned_array[0]);
}

/* ==================== 6. CONTROL FLOW VARIATIONS ==================== */
static void test_control_flow(void) {
    printf("\n=== Testing control flow contexts ===\n");
    
    /* Initialization inside conditional with constant condition */
    if (1) {  /* Always true, but creates a block context */
        int block_array[10] = {[2 ... 5] = 444};  /* count = 4 > 2 */
        printf("block_array[2]=%d\n", block_array[2]);
    }
    
    /* Switch with constant case */
    switch (3) {
        case 3: {
            int switch_array[5] = {[0 ... 2] = 555};  /* count = 3 > 2 */
            printf("switch_array[0]=%d\n", switch_array[0]);
            break;
        }
    }
    
    /* Loop with constant bound */
    for (int i = 0; i < 1; i++) {
        int loop_array[6] = {[1 ... 4] = 666};  /* count = 4 > 2 */
        printf("loop_array[1]=%d\n", loop_array[1]);
    }
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    printf("Starting constant bounds initialization tests...\n\n");
    
    /* Execute all test cases in sequence to cover different paths */
    test_count_le_2();               /* Triggers count <= 2 path */
    test_register_target();          /* Explores !MEM_P(target) */
    test_mem_target_large_count();   /* MEM_P && count > 2 && const size */
    test_volatile_target();          /* Forces MEM_P classification */
    test_multi_dimensional();        /* Nested constant bounds */
    test_compound_literals();        /* Compound literal contexts */
    test_control_flow();             /* Various control flow contexts */
    
    /* Final computation using initialized values to prevent dead code elimination */
    int final_check = 0;
    
    /* Access all test arrays to ensure they're used */
    {
        extern int big_array[100];  /* from static declaration */
        final_check += big_array[BIG_START];
    }
    
    printf("\nFinal check sum: %d\n", final_check);
    printf("All constant bounds initialization tests completed.\n");
    
    return 0;
}

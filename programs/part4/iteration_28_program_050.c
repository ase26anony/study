/* test_expr_constant_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically the uncovered block that handles
 * array/aggregate initialization with constant bounds.
 *
 * The code creates various initialization scenarios to cover different
 * paths in the condition:
 *   (!MEM_P(target) || count <= 2 || (tree_fits_uhwi_p(TYPE_SIZE(elttype)) && ...))
 */

#include <stdio.h>
#include <stdint.h>

/* ==================== 1. CONSTANT BOUNDS DEFINITIONS ==================== */
/* Use enum and const to ensure compile-time constant folding */
enum ConstantBounds {
    L1 = 0,
    H1 = 1,      /* count = 2 */
    L2 = 10,
    H2 = 90,     /* count = 81 > 2 */
    L3 = 5,
    H3 = 5,      /* count = 1 */
    L4 = 0,
    H4 = 2       /* count = 3 > 2 */
};

const int C_LO = 2;
const int C_HI = 5;  /* count = 4 > 2 */

/* ==================== 2. AGGREGATE TYPES WITH CONSTANT SIZES ==================== */
/* Basic types with constant sizes */
typedef char small_type;
typedef int medium_type;
typedef long large_type;

/* Packed struct with constant bitfield size */
struct PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
} __attribute__((packed));

/* Struct containing an array */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* Multi-dimensional array type */
typedef int md_array_t[4][6];

/* ==================== 3. INITIALIZATION SCENARIOS ==================== */

/* Scenario A: Register target with count <= 2 (!MEM_P(target) path) */
static void test_register_target(void) {
    /* Use 'register' keyword to encourage register allocation */
    register struct PackedStruct reg_target = {
        .a = 1,
        .b = 2,
        .c = 3
    };
    /* Designated initializer with constant range (count = 2) */
    register int reg_arr[10] = { [L1 ... H1] = 42 };
    
    /* Use values to prevent elimination */
    printf("Register target: %u %u %u\n", 
           reg_target.a, reg_target.b, reg_target.c);
    printf("Register array[0]=%d, [1]=%d\n", reg_arr[0], reg_arr[1]);
}

/* Scenario B: Memory target with count <= 2 (count <= 2 path) */
static int small_range_array[10] = { [L3] = 100 };  /* count = 1, static storage */

/* Scenario C: Memory target with count > 2 and constant element size */
static large_type big_array[100] = { 
    [L2 ... H2] = 0x123456789ABCDEFLL  /* count = 81 > 2, MEM_P true */
};

/* Scenario D: Automatic variable with constant bounds */
static void test_automatic_vars(void) {
    /* Automatic array - could be register or memory depending on optimization */
    int auto_array[] = { [C_LO ... C_HI] = 999 };  /* count = 4 > 2 */
    
    /* Volatile ensures MEM_P(target) == true */
    volatile int volatile_array[20] = { [L4 ... H4] = 777 };  /* count = 3 > 2 */
    
    /* Packed struct array with constant range */
    struct PackedStruct ps_array[10] = { [2 ... 4] = { .a = 5, .b = 10, .c = 2 } };
    
    printf("Auto array[%d]=%d\n", C_LO, auto_array[C_LO]);
    printf("Volatile array[%d]=%d\n", L4, volatile_array[L4]);
    printf("Packed struct array[3].b=%u\n", ps_array[3].b);
}

/* Scenario E: Multi-dimensional array with nested constant ranges */
static void test_multi_dimensional(void) {
    /* GCC extension: multi-dimensional designated initializers */
    md_array_t md = { [0 ... 1][2 ... 3] = 55 };  /* Creates multiple constant bounds */
    
    /* Struct with array member initialization */
    struct WithArray sa = { 
        .header = 1,
        .data = { [1 ... 3] = 88 },  /* count = 3 > 2 */
        .ps = { .a = 2, .b = 4, .c = 1 }
    };
    
    printf("MD[0][2]=%d, MD[1][3]=%d\n", md[0][2], md[1][3]);
    printf("Struct array data[2]=%d\n", sa.data[2]);
}

/* Scenario F: Mixed initializers with constant expressions */
static void test_mixed_initializers(void) {
    /* Complex constant expression as bound */
    int mixed[(H1 - L1 + 1) * 5] = { 
        [0 ... (H1 - L1 + 1) * 2 - 1] = 1234  /* count = 4 > 2 */
    };
    
    /* Compound literal assignment (creates initialization context) */
    struct PackedStruct *ptr = &(struct PackedStruct){
        .a = 6, .b = 20, .c = 0
    };
    
    printf("Mixed[0]=%d, Mixed[3]=%d\n", mixed[0], mixed[3]);
    printf("Compound literal .b=%u\n", ptr->b);
}

/* Scenario G: Conditional initialization with constant bounds */
static void test_conditional_init(int selector) {
    /* Use constant condition to ensure initialization is parsed */
    if (selector > 0) {
        /* This initialization should still be processed */
        int cond_array[15] = { [5 ... 10] = selector * 2 };  /* count = 6 > 2 */
        printf("Conditional array[7]=%d\n", cond_array[7]);
    } else {
        /* Different range */
        int cond_array[15] = { [0 ... 2] = 111 };  /* count = 3 > 2 */
        printf("Conditional array[1]=%d\n", cond_array[1]);
    }
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    printf("=== Testing constant bounds initialization paths ===\n\n");
    
    /* 1. Register target scenario (!MEM_P(target) path) */
    printf("1. Testing register target (hopefully !MEM_P):\n");
    test_register_target();
    printf("\n");
    
    /* 2. Static small range (count <= 2 path) */
    printf("2. Testing static small range (count <= 2):\n");
    printf("small_range_array[%d] = %d\n", L3, small_range_array[L3]);
    printf("\n");
    
    /* 3. Large static array (count > 2, MEM_P, constant size) */
    printf("3. Testing large static array (count > 2, MEM_P):\n");
    printf("big_array[%d] = %ld\n", L2, big_array[L2]);
    printf("big_array[%d] = %ld\n", H2, big_array[H2]);
    printf("\n");
    
    /* 4. Automatic and volatile variables */
    printf("4. Testing automatic and volatile variables:\n");
    test_automatic_vars();
    printf("\n");
    
    /* 5. Multi-dimensional and nested aggregates */
    printf("5. Testing multi-dimensional arrays:\n");
    test_multi_dimensional();
    printf("\n");
    
    /* 6. Mixed initializers */
    printf("6. Testing mixed initializers:\n");
    test_mixed_initializers();
    printf("\n");
    
    /* 7. Conditional initialization */
    printf("7. Testing conditional initialization:\n");
    test_conditional_init(1);
    test_conditional_init(0);
    printf("\n");
    
    /* Use all initialized values to prevent dead code elimination */
    int sum = small_range_array[L3] + big_array[L2] % 100;
    printf("Final checksum: %d\n", sum);
    
    return 0;
}

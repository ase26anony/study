/* test_expr_coverage.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc (lines 7691-7700) by creating various initializations
 * with constant array/aggregate bounds.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_coverage.c -o test_expr_coverage
 *   gcc -O2 -ftree-vectorize -std=gnu11 test_expr_coverage.c -o test_expr_coverage_opt
 */

#include <stdio.h>
#include <stdint.h>

/* ==================== 1. CONSTANT BOUNDS DEFINITIONS ==================== */
enum {
    L = 2,
    H = 5,
    BIG_START = 10,
    BIG_END = 90,
    SMALL_COUNT = 2
};

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

/* Struct containing an array */
struct with_array {
    int header;
    int data[8];
    struct packed_bitfield bits;
};

/* ==================== 3. TARGET CONDITION VARIATIONS ==================== */

/* Scenario A: Register target with !MEM_P(target) */
static void test_register_target(void) {
    /* Use register storage class to encourage register allocation */
    register struct packed_bitfield reg_target = {
        .a = 1,
        .b = 2,
        .c = 3
    };
    
    /* Designated initializer with constant range (count=1) */
    register int reg_arr[4] = { [L] = 42 };
    
    /* Force usage to prevent optimization */
    printf("Register target: %u %u %u\n", 
           reg_target.a, reg_target.b, reg_target.c);
    printf("Register array[%d] = %d\n", L, reg_arr[L]);
}

/* Scenario B: count <= 2 with memory target */
static void test_small_count_memory(void) {
    /* Exactly 2 elements in range */
    int small_range[10] = { [0 ... 1] = 99 };  /* count = 2 */
    
    /* Single element with constant index */
    static int single_elem[100] = { [H] = 77 };  /* count = 1 */
    
    printf("Small range[0]=%d, [1]=%d\n", small_range[0], small_range[1]);
    printf("Single elem[%d]=%d\n", H, single_elem[H]);
}

/* Scenario C: count > 2 with memory target and constant element size */
static void test_large_count_memory(void) {
    /* Large constant array with wide range */
    static int big_array[100] = { 
        [BIG_START ... BIG_END] = 123 
        /* count = 81, > 2, MEM_P(target)=true */
    };
    
    /* Multi-dimensional with constant range */
    int md_array[5][10] = {
        [0 ... 2][3 ... 7] = 456  /* 3*5 = 15 elements > 2 */
    };
    
    /* Packed struct array with constant range */
    struct packed_bitfield bit_array[20] = {
        [5 ... 15] = { .a = 1, .b = 2, .c = 3 }  /* count = 11 > 2 */
    };
    
    printf("Big array[%d]=%d, [%d]=%d\n", 
           BIG_START, big_array[BIG_START], 
           BIG_END, big_array[BIG_END]);
    printf("MD array[0][3]=%d, [2][7]=%d\n", 
           md_array[0][3], md_array[2][7]);
    printf("Bit array[5].c=%u, [15].c=%u\n", 
           bit_array[5].c, bit_array[15].c);
}

/* Scenario D: Nested aggregates with constant bounds */
static void test_nested_aggregates(void) {
    /* Struct containing array with designated range */
    struct with_array nested = {
        .header = 999,
        .data = { [1 ... 4] = 888 },  /* count = 4 > 2 */
        .bits = { .a = 5, .b = 6, .c = 7 }
    };
    
    /* Array of structs with constant range */
    struct with_array struct_array[10] = {
        [2 ... 5] = {  /* count = 4 > 2 */
            .header = 111,
            .data = { [0 ... 3] = 222 },
            .bits = { .a = 8, .b = 9, .c = 10 }
        }
    };
    
    printf("Nested.data[1]=%d, [4]=%d\n", 
           nested.data[1], nested.data[4]);
    printf("Struct_array[2].header=%d, [5].bits.c=%u\n",
           struct_array[2].header, struct_array[5].bits.c);
}

/* Scenario E: Volatile memory target */
static void test_volatile_target(void) {
    /* Volatile ensures MEM_P(target) = true */
    volatile int volatile_array[50] = {
        [10 ... 20] = 333  /* count = 11 > 2 */
    };
    
    volatile struct packed_bitfield volatile_struct = {
        .a = 11, .b = 12, .c = 13
    };
    
    printf("Volatile array[10]=%d, [20]=%d\n",
           volatile_array[10], volatile_array[20]);
    printf("Volatile struct.c=%u\n", volatile_struct.c);
}

/* Scenario F: Compound literals as targets */
static void test_compound_literals(void) {
    /* Compound literal assignment - creates initialization context */
    struct with_array *ptr = &(struct with_array){
        .data = { [0 ... 7] = 444 }  /* count = 8 > 2 */
    };
    
    int *arr_ptr = (int[30]){ [5 ... 15] = 555 };  /* count = 11 > 2 */
    
    printf("Compound literal ptr->data[0]=%d, [7]=%d\n",
           ptr->data[0], ptr->data[7]);
    printf("Compound literal arr_ptr[5]=%d, [15]=%d\n",
           arr_ptr[5], arr_ptr[15]);
}

/* ==================== 4. CONTROL FLOW VARIATIONS ==================== */

static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Automatic variable inside conditional block */
        int conditional_array[20] = {
            [3 ... 8] = selector  /* count = 6 > 2 */
        };
        printf("Conditional array[3]=%d, [8]=%d\n",
               conditional_array[3], conditional_array[8]);
    }
    
    switch (selector) {
        case 1: {
            /* Block scope with constant range */
            static int switch_array[10] = {
                [0 ... 9] = 666  /* count = 10 > 2 */
            };
            printf("Switch array[0]=%d\n", switch_array[0]);
            break;
        }
        default:
            break;
    }
}

/* ==================== 5. MAIN EXECUTION FLOW ==================== */

int main(void) {
    printf("=== Testing expr.cc constant bounds checking ===\n\n");
    
    /* 1. Register target (exploring !MEM_P(target)) */
    printf("1. Testing register target:\n");
    test_register_target();
    printf("\n");
    
    /* 2. Small count (count <= 2) */
    printf("2. Testing small count memory target:\n");
    test_small_count_memory();
    printf("\n");
    
    /* 3. Large count with memory target */
    printf("3. Testing large count memory target:\n");
    test_large_count_memory();
    printf("\n");
    
    /* 4. Nested aggregates */
    printf("4. Testing nested aggregates:\n");
    test_nested_aggregates();
    printf("\n");
    
    /* 5. Volatile target */
    printf("5. Testing volatile target:\n");
    test_volatile_target();
    printf("\n");
    
    /* 6. Compound literals */
    printf("6. Testing compound literals:\n");
    test_compound_literals();
    printf("\n");
    
    /* 7. Conditional initialization */
    printf("7. Testing conditional initialization:\n");
    test_conditional_init(1);
    printf("\n");
    
    /* Additional mixed test in main */
    {
        /* Local block with mixed initializers */
        int mixed[][4] = {
            [0 ... 2] = { [0 ... 3] = 777 },  /* 3*4 = 12 elements > 2 */
            [3] = { 1, 2, 3, 4 }
        };
        printf("Mixed array[0][0]=%d, [2][3]=%d\n",
               mixed[0][0], mixed[2][3]);
    }
    
    printf("=== All tests completed ===\n");
    return 0;
}

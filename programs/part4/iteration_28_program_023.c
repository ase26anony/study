/* This program is designed to trigger the uncovered constant bounds checking
   logic in GCC's expr.cc, specifically lines 7691-7700. It creates various
   initialization scenarios with constant array bounds to exercise different
   paths in the condition:
   (!MEM_P(target) || count <= 2 || (tree_fits_uhwi_p(TYPE_SIZE(elttype)) && ...))
*/

#include <stdio.h>
#include <stdint.h>

/* Use enum to define constant bounds that will be folded by the front-end */
enum { L = 2, H = 5, BIG_START = 10, BIG_END = 90 };

/* Packed struct with constant bitfield sizes to ensure TYPE_SIZE is constant */
struct __attribute__((packed)) PackedBitfield {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
};

/* Struct containing an array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct PackedBitfield bits;
};

/* Global/static initializations (MEM_P(target) is true here) */
static int static_arr[100] = { [BIG_START ... BIG_END] = 99 }; /* count > 2, constant size */

/* Multi-dimensional array with constant range */
static int md_arr[4][5] = { [0 ... 2][1 ... 3] = 7 };

/* Function to force register-target initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Small struct that may be initialized in a register */
    register struct PackedBitfield reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* Use designated initializer with constant range of 2 elements (count <= 2) */
    register int reg_arr[10] = { [L ... H] = 42 }; /* L=2, H=5 => count=4, but target may be register */
    
    /* Force computation to prevent elimination */
    printf("Register target: %u,%u,%u\n", reg_target.a, reg_target.b, reg_target.c);
    printf("Register array[%d]=%d\n", L, reg_arr[L]);
}

/* Function to test automatic variables with various conditions */
static void test_automatic_vars(void) {
    /* Automatic array with constant range (count > 2, MEM_P likely true) */
    int auto_arr[20] = { [3 ... 15] = 123 };
    
    /* Volatile ensures MEM_P(target) is true */
    volatile int volatile_arr[10] = { [0 ... 9] = 999 };
    
    /* Small range (count <= 2) with automatic storage */
    int small_range[10] = { [8] = 1, [9] = 2 }; /* Two elements */
    
    /* Nested struct with array initialization using constant range */
    struct WithArray nested = {
        .header = 0xABCD,
        .data = { [1 ... 4] = 88 },
        .bits = { .a = 5, .b = 10, .c = 2 }
    };
    
    /* Multi-dimensional automatic array */
    int local_md[3][4] = { [0 ... 1][2 ... 3] = 55 };
    
    /* Use values to prevent dead code elimination */
    printf("Auto_arr[5]=%d\n", auto_arr[5]);
    printf("Volatile_arr[0]=%d\n", volatile_arr[0]);
    printf("Small_range[8]=%d\n", small_range[8]);
    printf("Nested.data[2]=%d\n", nested.data[2]);
    printf("Local_md[0][2]=%d\n", local_md[0][2]);
}

/* Function with conditional constant initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector == 0) {
        int cond_arr[10] = { [0 ... 4] = 111 };
        printf("Cond_arr[2]=%d\n", cond_arr[2]);
    } else {
        /* Different range */
        int cond_arr2[10] = { [5 ... 9] = 222 };
        printf("Cond_arr2[7]=%d\n", cond_arr2[7]);
    }
}

/* Compound literal assignment (creates initialization context) */
static void test_compound_literal(void) {
    struct WithArray *ptr = &(struct WithArray){
        .data = { [2 ... 6] = 333 }
    };
    printf("Compound literal data[3]=%d\n", ptr->data[3]);
}

int main(void) {
    printf("Testing constant bounds initialization paths:\n");
    
    /* 1. Register target with constant range (explores !MEM_P(target)) */
    test_register_target();
    
    /* 2. Static initialization with large range (count > 2, MEM_P true) */
    printf("Static_arr[%d]=%d\n", BIG_START, static_arr[BIG_START]);
    printf("Static_arr[%d]=%d\n", BIG_END, static_arr[BIG_END]);
    
    /* 3. Multi-dimensional static */
    printf("MD_arr[1][2]=%d\n", md_arr[1][2]);
    
    /* 4. Automatic variables with various conditions */
    test_automatic_vars();
    
    /* 5. Conditional initialization */
    test_conditional_init(0);
    test_conditional_init(1);
    
    /* 6. Compound literal */
    test_compound_literal();
    
    return 0;
}

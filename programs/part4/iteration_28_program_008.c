/* test_expr_cc.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It creates various initialization patterns with constant bounds
 * to exercise different paths in the condition:
 *   (!MEM_P(target) || count <= 2 || 
 *    (tree_fits_uhwi_p(TYPE_SIZE(elttype)) && ...))
 */

#include <stdio.h>
#include <stdint.h>

/* ==================== Helper Definitions ==================== */

/* Use enum to define constant bounds that will be folded by the front-end */
enum { 
    L1 = 2, 
    H1 = 5,           /* count = 4 */
    L2 = 0, 
    H2 = 1,           /* count = 2 */
    L3 = 10, 
    H3 = 90,          /* count = 81 */
    L4 = 0, 
    H4 = 0,           /* count = 1 */
    L5 = 0,
    H5 = 2            /* count = 3 */
};

/* Small packed struct with constant bitfield sizes */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* Struct containing an array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct Packed7_9 p;
};

/* ==================== Test Cases ==================== */

/* 1. Static large array with wide constant range (count > 2, MEM_P(target) true) */
static int big_array[100] = { [L3 ... H3] = 99 };  /* count = 81, memory target */

/* 2. Static struct with array member and constant range */
static struct WithArray s1 = { 
    .header = 1,
    .data = { [2 ... 5] = 42 },  /* count = 4 */
    .p = { .a = 0x7F, .b = 0x1FF }
};

/* 3. Global with exactly 2-element range (count <= 2) */
static int two_elem[10] = { [L2 ... H2] = 7 };  /* count = 2 */

/* 4. Single element range (count = 1) */
static int single_elem[5] = { [L4] = 123 };

/* 5. Multi-dimensional array with nested constant range */
static int md_array[4][5] = { [0 ... 2][1 ... 3] = 9 };  /* 3 rows, 3 cols each */

/* Function to force register-target initialization */
static void test_register_target(void) {
    /* Use 'register' keyword to hint at register storage */
    register int reg_target = { [0] = 5 };  /* scalar init with designator, count=1 */
    
    /* Small struct that might go into register */
    register struct { int x; int y; } reg_struct = { .x = 10, .y = 20 };
    
    /* Use the values to prevent elimination */
    printf("Register targets: %d, {%d,%d}\n", 
           reg_target, reg_struct.x, reg_struct.y);
}

/* Function with automatic variables in different scopes */
static void test_automatic_vars(void) {
    /* Automatic array with constant range - stack memory target */
    int auto_array[20] = { [L1 ... H1] = 33 };  /* count = 4 */
    
    /* Volatile ensures MEM_P(target) is true */
    volatile int volatile_array[10] = { [1 ... 3] = 99 };  /* count = 3 */
    
    /* Nested block with another initialization */
    {
        /* This might be considered for register if small enough */
        struct Packed7_9 local_packed = { .a = 0x3F, .b = 0xFF };
        
        /* Multi-dimensional automatic */
        int local_md[3][4] = { [0 ... 1][2 ... 3] = 77 };
        
        printf("Local packed: %u,%u\n", local_packed.a, local_packed.b);
        printf("Local md[0][2] = %d\n", local_md[0][2]);
    }
    
    printf("Auto_array[%d] = %d\n", L1, auto_array[L1]);
    printf("Volatile_array[2] = %d\n", volatile_array[2]);
}

/* Function with conditional constant initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with constant range inside conditional block */
        int cond_array[15] = { [5 ... 9] = selector };  /* count = 5 */
        printf("Cond_array[6] = %d\n", cond_array[6]);
    } else {
        /* Different range when selector <= 0 */
        int alt_array[8] = { [0 ... 2] = -selector };  /* count = 3 */
        printf("Alt_array[1] = %d\n", alt_array[1]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            struct WithArray local_s = { 
                .data = { [0 ... 7] = 1 }  /* count = 8 */
            };
            printf("Case 1 init: %d\n", local_s.data[3]);
            break;
        }
        case 2: {
            /* Range with count = 1 */
            int single[10] = { [5] = 2 };
            printf("Case 2 init: %d\n", single[5]);
            break;
        }
        default: {
            /* Empty range? Actually single element with designator */
            int def[3] = { [2] = 3 };
            printf("Default init: %d\n", def[2]);
        }
    }
}

/* Test compound literals (create initialization contexts) */
static void test_compound_literals(void) {
    /* Compound literal assignment - creates initialization with target */
    struct WithArray *ptr = &s1;
    *ptr = (struct WithArray){ 
        .header = 100,
        .data = { [L5 ... H5] = 255 }  /* count = 3 */
    };
    
    /* Array compound literal */
    int *arr_ptr = (int[6]){ [1 ... 4] = 888 };  /* count = 4 */
    printf("Compound literal: %d\n", arr_ptr[2]);
}

/* ==================== Main ==================== */
int main(void) {
    printf("Testing constant-bounded initializations...\n");
    
    /* 1. Register target with count <= 2 */
    test_register_target();
    
    /* 2. Large static array (count > 2, MEM_P target) */
    printf("Big_array[%d] = %d\n", L3, big_array[L3]);
    printf("Big_array[%d] = %d\n", H3, big_array[H3]);
    
    /* 3. Static struct with array range */
    printf("s1.data[%d] = %d\n", L1, s1.data[L1]);
    
    /* 4. Two-element range global */
    printf("two_elem[%d] = %d\n", L2, two_elem[L2]);
    
    /* 5. Single element range */
    printf("single_elem[%d] = %d\n", L4, single_elem[L4]);
    
    /* 6. Multi-dimensional array */
    printf("md_array[1][2] = %d\n", md_array[1][2]);
    
    /* 7. Automatic variables in function */
    test_automatic_vars();
    
    /* 8. Conditional initialization */
    test_conditional_init(1);
    test_conditional_init(0);
    test_conditional_init(2);
    
    /* 9. Compound literals */
    test_compound_literals();
    
    /* Use all initialized values to prevent dead code elimination */
    int sum = big_array[L3] + s1.data[L1] + two_elem[L2] + 
              single_elem[L4] + md_array[1][2];
    printf("Final sum: %d\n", sum);
    
    return 0;
}

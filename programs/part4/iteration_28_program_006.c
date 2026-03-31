/* test_expr_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700, by creating various
 * initialization patterns with constant array/struct bounds.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_bounds.c -o test_expr_bounds
 *   gcc -O2 -ftree-vectorize -std=gnu11 test_expr_bounds.c -o test_expr_bounds_opt
 */

#include <stdio.h>
#include <string.h>

/* Use enum to define constant bounds that will be folded by the front-end */
enum { L = 2, H = 5 };
enum { START = 10, END = 90 };
enum { SMALL_COUNT = 2 };

/* Packed struct with constant bitfield sizes to ensure TYPE_SIZE is constant */
struct PackedBits {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
} __attribute__((packed));

/* Struct containing an array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct PackedBits bits;
};

/* Global/static initializations (MEM_P(target) likely true) */

/* 1. Large static array with wide constant range (count > 2, MEM_P true) */
static int big_array[100] = { [START ... END] = 99, [0] = -1, [99] = -1 };

/* 2. Static struct with array member initialized with constant range */
static struct WithArray global_struct = {
    .header = 0xABCD,
    .data = { [1 ... 4] = 123 },
    .bits = { .a = 127, .b = 511, .c = 7 }
};

/* 3. Multi-dimensional array with nested constant range */
static int md_array[5][6] = { [0 ... 2][3 ... 5] = 42 };

/* Function to force register-target initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Use 'register' keyword to hint at register storage */
    register int reg_target = ({ 
        int temp = 0;
        /* Compound literal with constant range (count <= 2) */
        struct { int a; int b; } s = { [0 ... 1] = 17 };
        temp = s.a + s.b;
        temp;
    });
    printf("Register target result: %d\n", reg_target);
    
    /* Small struct that may be initialized in registers */
    register struct PackedBits reg_bits = { .a = 64, .b = 256, .c = 3 };
    printf("Register bits: a=%u b=%u c=%u\n", 
           reg_bits.a, reg_bits.b, reg_bits.c);
}

/* Function to test automatic variables with various initializations */
static void test_automatic_vars(void) {
    /* Automatic array with constant range (count <= 2) */
    int small[5] = { [3 ... 4] = 9 };  /* count = 2 */
    printf("Small array[3]=%d [4]=%d\n", small[3], small[4]);
    
    /* Automatic array with larger constant range (count > 2) */
    int auto_array[20] = { [5 ... 15] = 77 };  /* count = 11 */
    printf("Auto array[5]=%d [15]=%d\n", auto_array[5], auto_array[15]);
    
    /* Volatile array to ensure MEM_P(target) is true */
    volatile int volatile_array[10] = { [0 ... 9] = 0xAA };
    printf("Volatile array[0]=%d\n", volatile_array[0]);
    
    /* Nested block with constant range initialization */
    {
        /* This may be considered for register initialization */
        int inner = ({ 
            struct { short x; short y; } p = { [0 ... 1] = 100 };
            p.x + p.y;
        });
        printf("Inner block result: %d\n", inner);
    }
    
    /* Multi-dimensional automatic array with constant range */
    int auto_md[3][4] = { [0 ... 1][2 ... 3] = 55 };
    printf("Auto MD[0][2]=%d [1][3]=%d\n", auto_md[0][2], auto_md[1][3]);
}

/* Function to test conditional initialization with constant bounds */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with enum-defined constant bounds */
        int arr[10] = { [L ... H] = selector };  /* L=2, H=5, count=4 */
        printf("Conditional arr[%d]=%d\n", L, arr[L]);
    } else {
        /* Different constant range when selector <= 0 */
        int arr[10] = { [0 ... SMALL_COUNT-1] = -selector };  /* count=2 */
        printf("Conditional arr[0]=%d\n", arr[0]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            /* Struct with designated array range */
            struct WithArray local = { .data = { [2 ... 6] = 888 } };
            printf("Switch case 1: data[2]=%d\n", local.data[2]);
            break;
        }
        case 2: {
            /* Array with single element range (count = 1) */
            int single[5] = { [4] = 999 };
            printf("Switch case 2: single[4]=%d\n", single[4]);
            break;
        }
        default: {
            /* Two-element range (count = 2) */
            int def[3] = { [0 ... 1] = 111 };
            printf("Switch default: def[0]=%d\n", def[0]);
            break;
        }
    }
}

/* Main function that orchestrates all test cases */
int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* 1. Register target initialization (!MEM_P(target)) */
    printf("1. Testing register target initializations:\n");
    test_register_target();
    printf("\n");
    
    /* 2. Automatic variables with various constant ranges */
    printf("2. Testing automatic variable initializations:\n");
    test_automatic_vars();
    printf("\n");
    
    /* 3. Conditional initializations with constant bounds */
    printf("3. Testing conditional initializations:\n");
    test_conditional_init(1);
    test_conditional_init(0);
    test_conditional_init(2);
    printf("\n");
    
    /* 4. Use static/global initializations (MEM_P(target) true) */
    printf("4. Testing static/global initializations:\n");
    printf("   big_array[%d]=%d (should be 99)\n", START, big_array[START]);
    printf("   big_array[%d]=%d (should be -1)\n", 0, big_array[0]);
    printf("   Global struct data[1]=%d (should be 123)\n", global_struct.data[1]);
    printf("   MD array[0][3]=%d (should be 42)\n", md_array[0][3]);
    printf("   Packed bits: a=%u b=%u c=%u\n", 
           global_struct.bits.a, global_struct.bits.b, global_struct.bits.c);
    
    /* 5. Compound literal assignment (creates initialization context) */
    printf("\n5. Testing compound literals:\n");
    struct WithArray *ptr = &global_struct;
    *ptr = (struct WithArray){ 
        .header = 0x1234,
        .data = { [0 ... 7] = 255 },  /* count = 8 */
        .bits = { .a = 1, .b = 2, .c = 3 }
    };
    printf("   After compound literal: header=0x%x data[7]=%d\n", 
           ptr->header, ptr->data[7]);
    
    /* 6. Ensure no dead code elimination by using all values */
    int checksum = big_array[START] + global_struct.data[1] + md_array[0][3];
    printf("\n6. Final checksum: %d\n", checksum);
    
    return 0;
}

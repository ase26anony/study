/* test_expr_coverage.c
 * Designed to cover constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_coverage.c -o test
 * Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test_expr_coverage.c -o test_opt
 */

#include <stdio.h>
#include <stdint.h>

/* 1. Constant bounds via enum */
enum { L = 2, H = 5, BIG_START = 10, BIG_END = 90 };

/* 2. Small packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:16;
};

/* 3. Struct containing array for nested initialization */
struct Container {
    int id;
    int data[8];
    struct PackedStruct ps;
};

/* 4. Static large array - will be MEM_P(target) with count > 2 */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* 5. Multi-dimensional array with constant bounds */
static int md_array[4][5] = { [0 ... 2][1 ... 3] = 7 };

/* Function to force register target (!MEM_P(target)) with count <= 2 */
static void test_register_target(void) {
    /* Use register keyword to encourage register allocation */
    register struct PackedStruct reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* Designated initializer with constant range of 2 elements */
    register int reg_arr[10] = { [L ... H] = 42 };  /* count = 4, but target might be register */
    
    /* Force usage to prevent elimination */
    printf("Register target: a=%d, b=%d, c=%d\n", reg_target.a, reg_target.b, reg_target.c);
    printf("Register array[%d]=%d\n", L, reg_arr[L]);
}

/* Function to test count <= 2 path */
static void test_small_count(void) {
    /* Exactly 1 element range - count = 1 */
    int single[100] = { [42] = 123 };
    
    /* Exactly 2 element range - count = 2 */
    int double_range[50] = { [10 ... 11] = 456 };
    
    /* Mixed: 2 explicit elements */
    int mixed[20] = { [0] = 1, [1] = 2 };  /* Not a range but 2 initializers */
    
    /* Range of 2 elements */
    int range_two[30] = { [5 ... 6] = 789 };
    
    printf("Small count test: %d %d %d %d\n", 
           single[42], double_range[10], mixed[0], range_two[5]);
}

/* Function to test MEM_P(target) with count > 2 and constant element size */
static void test_large_memory_target(void) {
    /* Automatic array with large range - MEM_P(target) true, count > 2 */
    int auto_large[200] = { [20 ... 80] = 333 };
    
    /* Volatile ensures MEM_P(target) */
    volatile int vol_array[100] = { [10 ... 50] = 777 };
    
    /* Packed struct array with constant size elements */
    struct PackedStruct ps_array[10] = { [1 ... 8] = { .a = 1, .b = 2, .c = 3 } };
    
    printf("Large memory: %d %d %d\n", 
           auto_large[50], vol_array[30], ps_array[2].b);
}

/* Function with nested blocks for different contexts */
static void test_nested_contexts(void) {
    /* Outer block - struct with array member initialization */
    struct Container c1 = { 
        .id = 1,
        .data = { [1 ... 4] = 111 },  /* count = 4 */
        .ps = { .a = 5, .b = 6, .c = 7 }
    };
    
    /* Nested block 1 */
    {
        /* Compound literal assignment - creates initialization context */
        struct Container *c2 = &(struct Container){
            .data = { [2 ... 5] = 222 }  /* count = 4 */
        };
        printf("Nested block 1: %d %d\n", c1.data[2], c2->data[3]);
    }
    
    /* Nested block 2 with conditional constant */
    if (1) {  /* Always true constant condition */
        /* Multi-dimensional with constant bounds */
        int local_md[3][4] = { [0 ... 1][2 ... 3] = 555 };
        printf("Nested block 2: %d\n", local_md[0][2]);
    }
    
    /* Switch with constant case */
    switch (3) {
        case 3: {
            /* Array with enum bounds */
            int switch_arr[20] = { [L ... H] = 888 };
            printf("Switch case: %d\n", switch_arr[3]);
            break;
        }
    }
}

/* Test constant bounds with different element types */
static void test_element_types(void) {
    /* char - small constant size */
    char char_array[100] = { [10 ... 20] = 'A' };
    
    /* short */
    short short_array[50] = { [5 ... 15] = 1234 };
    
    /* long long */
    long long ll_array[30] = { [2 ... 10] = 0x123456789ABCDEFLL };
    
    /* Bitfield struct array */
    struct BitfieldStruct {
        unsigned int a:3;
        unsigned int b:5;
        unsigned int c:8;
    } __attribute__((packed)) bf_array[20] = { [5 ... 15] = { 1, 2, 3 } };
    
    printf("Element types: %c %d %lld %u\n",
           char_array[15], short_array[10], ll_array[5], bf_array[10].b);
}

/* Main function orchestrating all tests */
int main(void) {
    printf("=== Testing GCC expr.cc constant bounds coverage ===\n");
    
    /* 1. First: register target with count <= 2 */
    test_register_target();
    
    /* 2. Small count paths (count <= 2) */
    test_small_count();
    
    /* 3. Static initialization (already done) - MEM_P with count > 2 */
    printf("Static big_array[%d] = %d\n", BIG_START, big_array[BIG_START]);
    printf("Static md_array[0][1] = %d\n", md_array[0][1]);
    
    /* 4. Large memory target with count > 2 */
    test_large_memory_target();
    
    /* 5. Nested contexts */
    test_nested_contexts();
    
    /* 6. Different element types */
    test_element_types();
    
    /* Additional complex case: nested struct with array of packed structs */
    struct {
        int header;
        struct PackedStruct items[8];
    } complex = {
        .header = 999,
        .items = { [1 ... 6] = { .a = 4, .b = 5, .c = 6 } }  /* count = 6 */
    };
    printf("Complex nested: %d %d\n", complex.header, complex.items[3].b);
    
    /* Force use of all arrays to prevent dead code elimination */
    int sum = big_array[10] + md_array[0][0];
    printf("Final sum: %d\n", sum);
    
    return 0;
}

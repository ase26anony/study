/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>

/* Use GNU C extensions for designated initializer ranges */
#pragma GCC diagnostic ignored "-Wpedantic"

/* 1. Register target with count <= 2 */
static void test_register_target(void) {
    /* Small struct that fits in registers */
    struct Small {
        char a;
        char b;
    };
    
    /* Force register storage with count=2 initialization */
    register struct Small reg_target = { .a = 1, .b = 2 };
    /* Alternative: designated range with exactly 2 elements */
    int reg_arr[10] = { [0] = 100, [1] = 200 }; /* count=2 for the range */
    
    printf("Register target: %d %d\n", reg_target.a, reg_arr[0]);
}

/* 2. Memory target with count <= 2 */
static void test_small_memory_target(void) {
    /* Static storage - definitely MEM_P */
    static int small_mem[5] = { [3] = 42 }; /* count=1 */
    
    /* Volatile ensures memory operand */
    volatile int vol_small[3] = { [0 ... 1] = 99 }; /* count=2 */
    
    printf("Small memory: %d %d\n", small_mem[3], vol_small[0]);
}

/* 3. Large memory target with count > 2 and constant element size */
static void test_large_memory_target(void) {
    /* Large array with wide constant range - triggers third condition */
    static int big_array[100] = { [10 ... 90] = 0xABCD }; /* count=81 > 2 */
    
    /* Packed struct with constant odd size */
    struct __attribute__((packed)) Packed {
        unsigned int a : 7;
        unsigned int b : 9;
        unsigned int c : 13;
    };
    
    /* Array of packed structs with constant range */
    static struct Packed packed_arr[50] = { [5 ... 25] = {1, 2, 3} };
    
    printf("Large memory: %d %d\n", big_array[50], packed_arr[10].a);
}

/* 4. Multi-dimensional array with constant ranges */
static void test_multi_dimensional(void) {
    /* 2D array with nested constant ranges */
    int md[5][6] = { [0 ... 2][1 ... 3] = 777 };
    
    /* 3D array with complex constant bounds */
    int three_d[3][4][5] = { 
        [0 ... 1][2 ... 3][1 ... 2] = 888 
    };
    
    printf("Multi-dim: %d %d\n", md[1][2], three_d[0][2][1]);
}

/* 5. Struct containing array with constant range */
static void test_nested_aggregate(void) {
    struct Container {
        int header;
        int data[10];
        int footer;
    };
    
    struct Container c = { 
        .header = 1,
        .data = { [2 ... 7] = 333 }, /* count=6 > 2 */
        .footer = 2
    };
    
    printf("Nested: %d %d\n", c.header, c.data[5]);
}

/* 6. Using enum constants for bounds */
static void test_enum_bounds(void) {
    enum { LOWER = 5, UPPER = 15 };
    
    int enum_arr[20] = { [LOWER ... UPPER] = 999 }; /* count=11 > 2 */
    
    printf("Enum bounds: %d\n", enum_arr[10]);
}

/* 7. Compound literal with constant range */
static void test_compound_literal(void) {
    /* Compound literal creates initialization context */
    int *ptr = (int[8]){ [1 ... 6] = 444 }; /* count=6 > 2 */
    
    printf("Compound: %d\n", ptr[3]);
}

/* 8. Mixed constant expressions */
static void test_mixed_constants(void) {
    /* Using const variables that fold to constants */
    const int start = 2;
    const int end = 8;
    
    int mixed[20] = { [start ... end] = 555 }; /* count=7 > 2 */
    
    /* Arithmetic in bounds */
    int arith[30] = { [2*3 ... 4+5] = 666 }; /* [6...9] count=4 > 2 */
    
    printf("Mixed: %d %d\n", mixed[5], arith[7]);
}

/* 9. Test with different element types */
static void test_different_types(void) {
    /* Different constant-sized types */
    char char_arr[50] = { [10 ... 40] = 'A' }; /* count=31 > 2 */
    
    short short_arr[30] = { [5 ... 20] = 1234 }; /* count=16 > 2 */
    
    long long_arr[20] = { [3 ... 15] = 999999L }; /* count=13 > 2 */
    
    printf("Types: %c %d %ld\n", char_arr[25], short_arr[10], long_arr[10]);
}

/* 10. Conditional initialization with constant condition */
static void test_conditional_init(void) {
    int cond_arr1[10];
    int cond_arr2[10];
    
    /* Constant condition ensures initialization is parsed */
    if (1) { /* Always true */
        cond_arr1[0 ... 4] = 111; /* count=5 > 2 */
    }
    
    switch (1) { /* Constant switch */
        case 1:
            cond_arr2[2 ... 7] = 222; /* count=6 > 2 */
            break;
    }
    
    printf("Conditional: %d %d\n", cond_arr1[2], cond_arr2[5]);
}

int main(void) {
    printf("Testing constant bounds initialization paths:\n\n");
    
    /* 1. Register target scenario (!MEM_P(target)) */
    test_register_target();
    
    /* 2. Small count scenario (count <= 2) with MEM_P */
    test_small_memory_target();
    
    /* 3. Large count with constant element size */
    test_large_memory_target();
    
    /* 4. Multi-dimensional arrays */
    test_multi_dimensional();
    
    /* 5. Nested aggregates */
    test_nested_aggregate();
    
    /* 6. Enum constants as bounds */
    test_enum_bounds();
    
    /* 7. Compound literals */
    test_compound_literal();
    
    /* 8. Mixed constant expressions */
    test_mixed_constants();
    
    /* 9. Different element types */
    test_different_types();
    
    /* 10. Conditional contexts */
    test_conditional_init();
    
    printf("\nAll tests completed.\n");
    
    return 0;
}

/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>

/* GNU C extensions required for designated range initializers */
#pragma GCC diagnostic ignored "-Wpedantic"

/* 1. Register target with count <= 2 */
static void test_register_target(void) {
    /* Small struct that fits in register */
    struct Small {
        int a;
        int b;
    } __attribute__((packed));
    
    /* Use register storage class to encourage register target */
    register struct Small reg_target = { .a = 1, .b = 2 };
    /* Constant bounds with exactly 2 elements */
    int arr1[10] = { [0] = 1, [1] = 2 };  /* count = 2 */
    
    printf("Register target: %d %d\n", reg_target.a, reg_target.b);
    printf("Array[0..1]: %d %d\n", arr1[0], arr1[1]);
}

/* 2. Memory target with count > 2 and constant element size */
static void test_memory_target_large_count(void) {
    /* Static storage ensures MEM_P(target) == true */
    static int big_array[100] = { 
        [10 ... 90] = 99  /* count = 81 > 2, constant bounds */
    };
    
    /* Packed struct with constant odd size */
    struct Packed {
        int a:7;
        int b:9;
        int c:3;
    } __attribute__((packed));
    
    static struct Packed packed_array[50] = {
        [5 ... 45] = { .a = 1, .b = 2, .c = 3 }  /* count = 41 > 2 */
    };
    
    printf("Big array[10]=%d, [90]=%d\n", big_array[10], big_array[90]);
    printf("Packed[5].a=%d\n", packed_array[5].a);
}

/* 3. Automatic variable with potential register target */
static void test_automatic_variable(void) {
    /* Inside function, small array might be considered for register */
    int auto_array[5] = { [0 ... 4] = 42 };  /* count = 5 > 2 */
    
    /* Volatile forces memory operand */
    volatile int volatile_array[10] = { [2 ... 7] = 99 };  /* count = 6 > 2 */
    
    printf("Auto array[0]=%d\n", auto_array[0]);
    printf("Volatile array[2]=%d\n", volatile_array[2]);
}

/* 4. Multi-dimensional array with nested constant ranges */
static void test_multi_dimensional(void) {
    int md[3][4] = { 
        [0 ... 1][2 ... 3] = 5  /* Nested constant ranges */
    };
    
    /* Struct containing array with constant range */
    struct Container {
        int id;
        int data[8];
    };
    
    struct Container cont = { 
        .id = 1,
        .data = { [1 ... 6] = 42 }  /* count = 6 > 2 */
    };
    
    printf("MD[0][2]=%d, MD[1][3]=%d\n", md[0][2], md[1][3]);
    printf("Container.data[1]=%d\n", cont.data[1]);
}

/* 5. Using enum constants for bounds */
static void test_enum_bounds(void) {
    enum { 
        LOWER = 3,
        UPPER = 8
    };
    
    int enum_array[20] = { [LOWER ... UPPER] = 77 };  /* count = 6 > 2 */
    
    /* const variables that fold to constants */
    const int c_low = 1;
    const int c_high = 4;
    int const_bounds_array[10] = { [c_low ... c_high] = 88 };  /* count = 4 > 2 */
    
    printf("Enum array[3]=%d, [8]=%d\n", enum_array[3], enum_array[8]);
    printf("Const bounds array[1]=%d\n", const_bounds_array[1]);
}

/* 6. Compound literals for initialization */
static void test_compound_literals(void) {
    struct Point {
        int x;
        int y;
    };
    
    /* Compound literal assignment - creates initialization context */
    struct Point p = (struct Point){ .x = 10, .y = 20 };
    
    /* Array compound literal with range */
    int *arr_ptr = (int[10]){ [2 ... 5] = 100 };  /* count = 4 > 2 */
    
    printf("Point: %d,%d\n", p.x, p.y);
    printf("Compound array[2]=%d\n", arr_ptr[2]);
}

/* 7. Conditional initialization with constant conditions */
static void test_conditional_init(void) {
    int flag = 1;
    int conditional_array[10];
    
    /* Constant condition ensures initialization is parsed */
    if (flag) {
        /* This should still trigger constant bounds checking */
        int temp[5] = { [0 ... 4] = 33 };  /* count = 5 > 2 */
        conditional_array[0] = temp[0];
    }
    
    /* Switch with constant case */
    switch (1) {
        case 1: {
            int switch_array[8] = { [1 ... 6] = 44 };  /* count = 6 > 2 */
            conditional_array[1] = switch_array[1];
            break;
        }
    }
    
    printf("Conditional array[0]=%d\n", conditional_array[0]);
}

/* 8. Zero-length array in struct (GNU extension) */
static void test_flexible_array(void) {
    struct Header {
        int type;
        int data[];
    };
    
    /* Compound literal with flexible array member */
    struct Header *hdr = &(struct Header){ 
        .type = 1
        /* No initialization for flexible array to keep it simple */
    };
    
    printf("Header type: %d\n", hdr->type);
}

int main(void) {
    printf("=== Testing constant bounds initialization coverage ===\n\n");
    
    /* Test 1: Register target and count <= 2 */
    printf("1. Testing register target (count <= 2):\n");
    test_register_target();
    printf("\n");
    
    /* Test 2: Memory target with large count */
    printf("2. Testing memory target with count > 2:\n");
    test_memory_target_large_count();
    printf("\n");
    
    /* Test 3: Automatic variables */
    printf("3. Testing automatic variables:\n");
    test_automatic_variable();
    printf("\n");
    
    /* Test 4: Multi-dimensional arrays */
    printf("4. Testing multi-dimensional arrays:\n");
    test_multi_dimensional();
    printf("\n");
    
    /* Test 5: Enum and const bounds */
    printf("5. Testing enum and const bounds:\n");
    test_enum_bounds();
    printf("\n");
    
    /* Test 6: Compound literals */
    printf("6. Testing compound literals:\n");
    test_compound_literals();
    printf("\n");
    
    /* Test 7: Conditional initialization */
    printf("7. Testing conditional initialization:\n");
    test_conditional_init();
    printf("\n");
    
    /* Test 8: Flexible array */
    printf("8. Testing flexible array member:\n");
    test_flexible_array();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    return 0;
}

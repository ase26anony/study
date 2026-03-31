/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>
#include <stdint.h>

/* GNU C extensions required for designated range initializers */
#pragma GCC diagnostic ignored "-Wpedantic"

/* 1. Register target with count <= 2 - should trigger !MEM_P(target) path */
static void test_register_target(void)
{
    /* Small struct that fits in register */
    struct SmallReg {
        int a;
        int b;
    };
    
    /* Use register keyword to encourage register allocation */
    register struct SmallReg reg_target = { 
        .a = 10,
        .b = 20 
    };
    
    /* Designated initializer with constant range (count = 2) */
    struct SmallReg reg_target2 = {
        .a = 1,
        .b = 2  /* This is essentially [0...1] initialization for two fields */
    };
    
    printf("Register target: %d %d\n", reg_target.a, reg_target2.b);
}

/* 2. Memory target with count <= 2 */
static void test_small_memory_target(void)
{
    /* Array with single element initialization - count = 1 */
    int small_array[10] = { [5] = 42 };  /* Only one element initialized */
    
    /* Array with two elements initialized - count = 2 */
    int two_elem[10] = { [2] = 100, [3] = 200 };  /* Two consecutive elements */
    
    /* Using constant enum bounds */
    enum { START = 3, END = 4 };
    int enum_bounded[10] = { [START ... END] = 99 };  /* count = 2 */
    
    printf("Small memory: %d %d %d\n", small_array[5], two_elem[2], enum_bounded[3]);
}

/* 3. Large memory target with count > 2 and constant element size */
static void test_large_memory_target(void)
{
    /* Large array with wide constant range - count > 2, MEM_P(target) true */
    static int big_array[1000] = { [100 ... 900] = 12345 };  /* count = 801 */
    
    /* Different element types with constant sizes */
    static char char_array[500] = { [100 ... 200] = 'X' };  /* count = 101 */
    
    /* Short type */
    static short short_array[300] = { [50 ... 150] = 999 };  /* count = 101 */
    
    printf("Large memory: %d %c %d\n", big_array[500], char_array[150], short_array[100]);
}

/* 4. Packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
    unsigned int d : 13;
};  /* Total size: 32 bits = 4 bytes (constant) */

static void test_packed_struct(void)
{
    /* Initialize packed struct array with constant range */
    struct PackedStruct packed_array[10] = { 
        [2 ... 5] = { .a = 127, .b = 511, .c = 7, .d = 8191 }
    };  /* count = 4 > 2 */
    
    printf("Packed struct: %u\n", packed_array[3].a);
}

/* 5. Multi-dimensional array with nested constant ranges */
static void test_multi_dimensional(void)
{
    /* 2D array with constant range in both dimensions */
    int matrix[10][20] = { 
        [2 ... 5][3 ... 8] = 777  /* count = 4 * 6 = 24 > 2 */
    };
    
    /* 3D array */
    int cube[5][5][5] = {
        [1 ... 3][2 ... 4][0 ... 2] = 888  /* count = 3 * 3 * 3 = 27 > 2 */
    };
    
    printf("Multi-dim: %d %d\n", matrix[3][5], cube[2][3][1]);
}

/* 6. Struct containing array with constant range */
struct Container {
    int header;
    int data[10];
    int footer;
};

static void test_nested_aggregate(void)
{
    /* Initialize only part of the internal array */
    struct Container container = {
        .header = 1,
        .data = { [2 ... 7] = 42 },  /* count = 6 > 2 */
        .footer = 3
    };
    
    printf("Nested aggregate: %d %d\n", container.header, container.data[5]);
}

/* 7. Volatile memory target (always MEM_P) */
static void test_volatile_target(void)
{
    /* Volatile ensures memory operand */
    volatile int volatile_array[50] = { [10 ... 40] = 999 };  /* count = 31 > 2 */
    
    printf("Volatile: %d\n", volatile_array[25]);
}

/* 8. Compound literal assignment */
static void test_compound_literal(void)
{
    struct Point {
        int x;
        int y;
        int z;
    };
    
    /* Compound literal creates initialization context */
    struct Point pt;
    pt = (struct Point){ .x = 1, .y = 2, .z = 3 };  /* count = 3 > 2 */
    
    printf("Compound literal: %d %d %d\n", pt.x, pt.y, pt.z);
}

/* 9. Mixed constant expressions */
static void test_mixed_constants(void)
{
    /* Using const variables that are compile-time constants */
    const int L = 0;
    const int H = 9;
    
    int mixed_array[100] = { [L ... H] = 111 };  /* count = 10 > 2 */
    
    /* Arithmetic in bounds */
    int computed_array[50] = { [2*3 ... 5+10] = 222 };  /* [6...15] count = 10 > 2 */
    
    printf("Mixed constants: %d %d\n", mixed_array[5], computed_array[10]);
}

/* 10. Union with constant range */
static void test_union_range(void)
{
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed union_array[8] = { [1 ... 6] = { .i = 0xDEADBEEF } };  /* count = 6 > 2 */
    
    printf("Union: 0x%x\n", union_array[3].i);
}

int main(void)
{
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* Execute tests in sequence to cover different paths */
    test_register_target();           /* !MEM_P(target) path */
    test_small_memory_target();       /* count <= 2 path */
    test_large_memory_target();       /* count > 2, MEM_P, constant size */
    test_packed_struct();             /* Packed struct with constant size */
    test_multi_dimensional();         /* Nested constant ranges */
    test_nested_aggregate();          /* Struct with array member */
    test_volatile_target();           /* Always memory operand */
    test_compound_literal();          /* Compound literal initialization */
    test_mixed_constants();           /* Various constant expressions */
    test_union_range();               /* Union type */
    
    printf("\nAll tests completed.\n");
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = 0;
    return dummy;
}

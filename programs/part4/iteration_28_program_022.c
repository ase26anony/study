/* Test program to cover constant bounds checking in GCC's expr.cc
 * Specifically targeting lines 7691-7700 related to array/aggregate
 * initialization with constant bounds.
 */

#include <stdio.h>
#include <stddef.h>

/* Use enum to ensure constant folding */
enum { 
    LO = 2, 
    HI = 5,
    BIG_LO = 10,
    BIG_HI = 90,
    SMALL_COUNT = 1
};

/* Packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
};

/* Struct containing array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* Global/static initializations (MEM_P(target) likely true) */
static int global_arr[100] = { [BIG_LO ... BIG_HI] = 99 };  /* count > 2, MEM_P */
static struct PackedStruct global_ps = { .a = 127, .b = 511, .c = 7 };

/* Function to force register target with !MEM_P(target) */
static void test_register_target(void) {
    /* Try to force register target with small struct */
    register struct PackedStruct reg_target = { 
        .a = 63, 
        .b = 255, 
        .c = 3 
    };
    
    /* Use designated initializer with constant range (count=4) */
    register int reg_arr[10] = { [LO ... HI] = 42 };
    
    /* Use the values to prevent elimination */
    printf("Register target - a: %u, b: %u\n", reg_target.a, reg_target.b);
    printf("Register array[%d]: %d\n", LO, reg_arr[LO]);
}

/* Test count <= 2 cases */
static void test_small_count(void) {
    /* Case 1: Single element (count=1) */
    int single[100] = { [50] = 123 };
    
    /* Case 2: Two elements (count=2) */
    int double_arr[100] = { [30 ... 31] = 456 };
    
    /* Case 3: Two elements with different values */
    int double_diff[100] = { [20] = 1, [21] = 2 };
    
    /* Volatile to ensure MEM_P classification */
    volatile int vol_single[10] = { [5] = 999 };
    
    printf("Small count tests - single: %d, double: %d\n", 
           single[50], double_arr[30]);
    printf("Volatile single: %d\n", vol_single[5]);
}

/* Test count > 2 with MEM_P(target) true */
static void test_large_count_memory(void) {
    /* Large range initialization (count=81) */
    int big_local[200] = { [BIG_LO ... BIG_HI] = 77 };
    
    /* With different element types */
    char char_big[1000] = { [100 ... 200] = 'A' };
    short short_big[500] = { [50 ... 150] = 1234 };
    
    /* Packed struct array */
    struct PackedStruct ps_arr[50] = { [10 ... 20] = { .a = 1, .b = 2, .c = 3 } };
    
    printf("Large count - big_local[%d]: %d\n", BIG_LO, big_local[BIG_LO]);
    printf("char_big[150]: %c, short_big[100]: %d\n", 
           char_big[150], short_big[100]);
    printf("ps_arr[15].a: %u\n", ps_arr[15].a);
}

/* Test multi-dimensional arrays */
static void test_multi_dimensional(void) {
    /* 2D array with constant range */
    int md[10][10] = { [2 ... 5][3 ... 7] = 888 };
    
    /* 3D array */
    int three_d[5][5][5] = { [1 ... 3][2 ... 4][0 ... 2] = 555 };
    
    /* Struct with nested array */
    struct WithArray wa = { 
        .header = 111,
        .data = { [1 ... 3] = 222 },
        .ps = { .a = 5, .b = 10, .c = 2 }
    };
    
    printf("Multi-dim - md[3][4]: %d\n", md[3][4]);
    printf("3D array: %d\n", three_d[2][3][1]);
    printf("Struct with array - data[2]: %d, ps.a: %u\n", 
           wa.data[2], wa.ps.a);
}

/* Test with compound literals */
static void test_compound_literals(void) {
    /* Compound literal assignment - may affect target classification */
    struct PackedStruct *ps_ptr;
    ps_ptr = &(struct PackedStruct){ .a = 31, .b = 100, .c = 1 };
    
    int *arr_ptr;
    arr_ptr = (int[20]){ [5 ... 15] = 333 };
    
    printf("Compound literal - ps_ptr->b: %u, arr_ptr[10]: %d\n",
           ps_ptr->b, arr_ptr[10]);
}

/* Test with conditional constant initialization */
static void test_conditional_init(void) {
    const int condition = 1;  /* Compile-time constant */
    
    if (condition) {
        /* This initialization should be processed */
        int cond_arr[50] = { [10 ... 20] = 444 };
        printf("Conditional init: %d\n", cond_arr[15]);
    }
    
    /* Switch with constant case */
    switch (3) {
        case 3: {
            int switch_arr[30] = { [5 ... 10] = 555 };
            printf("Switch init: %d\n", switch_arr[7]);
            break;
        }
    }
}

/* Test alignment attributes */
static void test_aligned_targets(void) {
    /* Aligned array might affect MEM_P classification */
    int __attribute__((aligned(64))) aligned_arr[100] = { [20 ... 40] = 666 };
    
    /* Packed and aligned struct */
    struct __attribute__((packed, aligned(2))) AlignedStruct {
        char x;
        int y;
    } as = { .x = 'X', .y = 777 };
    
    printf("Aligned array[30]: %d\n", aligned_arr[30]);
    printf("Aligned struct: %c %d\n", as.x, as.y);
}

int main(void) {
    printf("=== Testing constant bounds initialization ===\n\n");
    
    /* 1. Register target (hopefully !MEM_P) */
    printf("1. Testing register target:\n");
    test_register_target();
    printf("\n");
    
    /* 2. Small count (count <= 2) */
    printf("2. Testing small count (<=2):\n");
    test_small_count();
    printf("\n");
    
    /* 3. Large count with MEM_P target */
    printf("3. Testing large count with memory target:\n");
    test_large_count_memory();
    printf("\n");
    
    /* 4. Multi-dimensional arrays */
    printf("4. Testing multi-dimensional arrays:\n");
    test_multi_dimensional();
    printf("\n");
    
    /* 5. Compound literals */
    printf("5. Testing compound literals:\n");
    test_compound_literals();
    printf("\n");
    
    /* 6. Conditional initialization */
    printf("6. Testing conditional initialization:\n");
    test_conditional_init();
    printf("\n");
    
    /* 7. Aligned targets */
    printf("7. Testing aligned targets:\n");
    test_aligned_targets();
    printf("\n");
    
    /* Use global arrays to prevent elimination */
    printf("Global array[BIG_LO]: %d\n", global_arr[BIG_LO]);
    printf("Global packed struct: a=%u b=%u c=%u\n", 
           global_ps.a, global_ps.b, global_ps.c);
    
    return 0;
}

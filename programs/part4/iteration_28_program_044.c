/* Test program for GCC expr.cc constant bounds checking logic */
/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test.c -o test */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test.c -o test_opt */

#include <stdio.h>
#include <stddef.h>

/* Use enum to define constant bounds */
enum { L = 2, H = 5, SMALL_COUNT = 2 };
enum { BIG_LO = 10, BIG_HI = 90, BIG_COUNT = 81 };

/* Small packed struct with constant size */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:3;
    char d;
};

/* Struct containing array */
struct Container {
    int id;
    int data[8];
    struct PackedStruct ps;
};

/* Global/static initializations (MEM_P(target) likely true) */
static int global_arr[100] = { [BIG_LO ... BIG_HI] = 99 };  /* count > 2, MEM_P */

/* Multi-dimensional array with constant range */
static int md_arr[4][5] = { [0 ... 2][1 ... 3] = 7 };

/* Function to force register target (!MEM_P) scenario */
static void test_register_target(void) {
    /* Use register keyword to hint at register allocation */
    register struct PackedStruct reg_target = { 
        .a = 1, 
        .b = 2, 
        .c = 3,
        .d = 4
    };
    
    /* Small range initialization that might use registers */
    register int reg_arr[SMALL_COUNT] = { [0 ... 1] = 42 };  /* count <= 2 */
    
    /* Use the values to prevent optimization */
    printf("Register target: %d %d\n", reg_target.a, reg_arr[0]);
}

/* Function with automatic variables (stack-based, MEM_P likely true) */
static void test_stack_target(void) {
    /* Automatic array with constant range - count <= 2 */
    int small_arr[10] = { [3 ... 4] = 17 };  /* count = 2 */
    
    /* Automatic array with larger range - count > 2 */
    int medium_arr[20] = { [5 ... 15] = 123 };
    
    /* Volatile ensures MEM_P classification */
    volatile int volatile_arr[10] = { [2 ... 7] = 99 };
    
    /* Nested struct with array initialization */
    struct Container cont = {
        .id = 1,
        .data = { [1 ... 3] = 42 },  /* count = 3 */
        .ps = { .a = 5, .b = 10, .c = 2, .d = 'x' }
    };
    
    printf("Stack targets: %d %d %d\n", small_arr[3], medium_arr[10], cont.data[2]);
    printf("Volatile: %d\n", volatile_arr[5]);
}

/* Test with compound literals */
static void test_compound_literals(void) {
    struct Container *ptr;
    
    /* Compound literal assignment - creates initialization context */
    struct Container local = (struct Container){
        .id = 100,
        .data = { [0 ... 7] = 255 },  /* count = 8 > 2 */
        .ps = { .a = 127, .b = 511, .c = 7, .d = 'z' }
    };
    
    ptr = &(struct Container){
        .data = { [4 ... 6] = 777 }  /* count = 3 */
    };
    
    printf("Compound literal: %d %d\n", local.data[0], ptr->data[5]);
}

/* Test with conditional constant initialization */
static void test_conditional_init(void) {
    const int flag = 1;  /* Compile-time constant */
    
    if (flag) {
        /* This initialization should be processed */
        int cond_arr[10] = { [2 ... 8] = 333 };  /* count = 7 > 2 */
        printf("Conditional init: %d\n", cond_arr[5]);
    }
    
    /* Switch with constant case */
    switch (3) {
        case 3: {
            int switch_arr[5] = { [1 ... 2] = 444 };  /* count = 2 */
            printf("Switch init: %d\n", switch_arr[1]);
            break;
        }
    }
}

/* Test with different element types */
static void test_mixed_types(void) {
    /* char type - small constant size */
    char char_arr[50] = { [10 ... 40] = 'A' };  /* count = 31 > 2 */
    
    /* short type */
    short short_arr[30] = { [5 ... 25] = 32767 };
    
    /* long long type */
    long long ll_arr[10] = { [2 ... 7] = 0xFFFFFFFFLL };
    
    /* Array of packed structs */
    struct PackedStruct ps_arr[5] = { 
        [1 ... 3] = { .a = 63, .b = 255, .c = 3, .d = '!' }
    };
    
    printf("Mixed types: %c %d %lld %d\n", 
           char_arr[20], short_arr[15], ll_arr[5], ps_arr[2].a);
}

/* Test designated initializers with computed but constant bounds */
static void test_computed_bounds(void) {
    /* These are still compile-time constants */
    const int start = L * 2;
    const int end = H * 2;
    
    int computed_arr[20] = { [start ... end] = 999 };  /* [4 ... 10] = 999 */
    
    /* Using sizeof to create constant bounds */
    int size_based[100] = { [0 ... sizeof(struct PackedStruct)] = 111 };
    
    printf("Computed bounds: %d %d\n", computed_arr[7], size_based[5]);
}

int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* 1. Register target with count <= 2 */
    printf("1. Testing register target (count <= 2):\n");
    test_register_target();
    
    /* 2. Stack/Memory target with various counts */
    printf("\n2. Testing stack/memory targets:\n");
    test_stack_target();
    
    /* 3. Compound literals */
    printf("\n3. Testing compound literals:\n");
    test_compound_literals();
    
    /* 4. Conditional initialization */
    printf("\n4. Testing conditional initialization:\n");
    test_conditional_init();
    
    /* 5. Mixed element types */
    printf("\n5. Testing mixed element types:\n");
    test_mixed_types();
    
    /* 6. Computed but constant bounds */
    printf("\n6. Testing computed constant bounds:\n");
    test_computed_bounds();
    
    /* Use global/static arrays to prevent dead code elimination */
    printf("\n7. Global/static array values:\n");
    printf("   Global[50] = %d\n", global_arr[50]);
    printf("   MD array[1][2] = %d\n", md_arr[1][2]);
    
    /* Test single element range (count = 1) */
    printf("\n8. Testing single element range:\n");
    int single_range[10] = { [5] = 1234 };  /* count = 1 */
    printf("   Single range: %d\n", single_range[5]);
    
    /* Test exactly 2 elements (count = 2) */
    int double_range[10] = { [3 ... 4] = 5678 };
    printf("   Double range: %d %d\n", double_range[3], double_range[4]);
    
    return 0;
}

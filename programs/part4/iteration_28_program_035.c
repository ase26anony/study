/* Test program to cover constant bounds checking in GCC's expr.cc
   Specifically targets lines 7691-7700 related to array/aggregate
   initialization with constant bounds. */

#include <stdio.h>

/* Use enum to define constant bounds */
enum { L = 2, H = 5 };
enum { SMALL_COUNT = 2 };

/* Packed struct with constant bitfield size */
struct Packed {
    int a : 7;
    int b : 9;
    int c : 16;
} __attribute__((packed));

/* Struct containing an array */
struct WithArray {
    int x;
    int arr[10];
    struct Packed p;
};

/* Global/static initializations (MEM_P(target) likely true) */
static int global_arr[100] = { [10 ... 90] = 99 };  /* count > 2, constant size */
static struct Packed global_packed = { .a = 63, .b = 255, .c = 32767 };
static int small_range[10] = { [3 ... 4] = 7 };     /* count = 2 */

/* Multi-dimensional array with constant range */
static int md_arr[5][6] = { [1 ... 3][2 ... 4] = 42 };

/* Function to force register-target initialization */
static void test_register_target(void) {
    /* Try to force register target with small struct */
    register struct Packed reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* Small array initialization that might use registers */
    register int reg_arr[2] = { [0 ... 1] = 5 };  /* count = 2 */
    
    /* Use the values to prevent optimization */
    printf("reg_target: %d %d %d\n", reg_target.a, reg_target.b, reg_target.c);
    printf("reg_arr: %d %d\n", reg_arr[0], reg_arr[1]);
}

/* Function with various automatic variable initializations */
static void test_automatic_vars(void) {
    /* Automatic array with constant bounds (stack, MEM_P likely true) */
    int auto_arr[20] = { [L ... H] = 42 };  /* count = 4 > 2 */
    
    /* Volatile ensures memory operand */
    volatile int volatile_arr[10] = { [0 ... 9] = 123 };  /* count = 10 > 2 */
    
    /* Small count initialization */
    int small[5] = { [2] = 1 };  /* count = 1 <= 2 */
    
    /* Nested struct with array initialization */
    struct WithArray s = { 
        .x = 10,
        .arr = { [1 ... 3] = 7 },  /* count = 3 > 2 */
        .p = { .a = 3, .b = 4, .c = 5 }
    };
    
    /* Compound literal assignment (creates initialization context) */
    struct Packed *ptr = &(struct Packed){ .a = 6, .b = 7, .c = 8 };
    
    /* Use values */
    printf("auto_arr[%d]=%d\n", L, auto_arr[L]);
    printf("volatile_arr[0]=%d\n", volatile_arr[0]);
    printf("small[2]=%d\n", small[2]);
    printf("s.arr[2]=%d\n", s.arr[2]);
    printf("ptr->a=%d\n", ptr->a);
}

/* Function with conditional initialization */
static void test_conditional(void) {
    int flag = 1;
    
    if (flag) {
        /* This initialization should still be parsed */
        int cond_arr[8] = { [2 ... 5] = 99 };  /* count = 4 > 2 */
        printf("cond_arr[3]=%d\n", cond_arr[3]);
    }
    
    switch (flag) {
        case 1: {
            int switch_arr[6] = { [1 ... 4] = 77 };  /* count = 4 > 2 */
            printf("switch_arr[2]=%d\n", switch_arr[2]);
            break;
        }
    }
}

/* Test different element types with constant sizes */
static void test_different_types(void) {
    /* char - small element size */
    char char_arr[50] = { [10 ... 40] = 'A' };  /* count = 31 > 2 */
    
    /* short */
    short short_arr[30] = { [5 ... 15] = 1000 };  /* count = 11 > 2 */
    
    /* long long */
    long long ll_arr[10] = { [2 ... 7] = 0x123456789ABCDEFLL };  /* count = 6 > 2 */
    
    /* Array of packed structs */
    struct Packed packed_arr[5] = { 
        [0 ... 2] = { .a = 1, .b = 2, .c = 3 }  /* count = 3 > 2 */
    };
    
    printf("char_arr[20]=%c\n", char_arr[20]);
    printf("short_arr[10]=%d\n", short_arr[10]);
    printf("ll_arr[5]=%llx\n", ll_arr[5]);
    printf("packed_arr[1].a=%d\n", packed_arr[1].a);
}

int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* 1. Register target with small count (count <= 2) */
    test_register_target();
    
    /* 2. Large static array (count > 2, MEM_P true) */
    printf("global_arr[50]=%d\n", global_arr[50]);
    
    /* 3. Multi-dimensional array */
    printf("md_arr[2][3]=%d\n", md_arr[2][3]);
    
    /* 4. Automatic variables with various conditions */
    test_automatic_vars();
    
    /* 5. Conditional initializations */
    test_conditional();
    
    /* 6. Different element types */
    test_different_types();
    
    /* Use global small_range (count = 2) */
    printf("small_range[3]=%d\n", small_range[3]);
    
    /* Use global packed struct */
    printf("global_packed.b=%d\n", global_packed.b);
    
    return 0;
}

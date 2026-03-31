/* Test program to cover constant bounds checking in GCC's expr.cc
   Specifically targeting lines 7691-7700:
   const_bounds_p && tree_fits_shwi_p conditions with
   (!MEM_P(target) || count <= 2 || ...) logic
*/

#include <stdio.h>
#include <stddef.h>

/* Use enum to ensure constant folding */
enum { 
    L = 2, 
    H = 5,
    BIG_START = 10,
    BIG_END = 90,
    SMALL_COUNT = 2
};

/* Packed struct with constant bitfield size */
struct PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 4;
} __attribute__((packed));

/* Struct containing array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* Global/static initializations (MEM_P(target) likely true) */
static int global_arr[100] = { 
    [BIG_START ... BIG_END] = 99  /* count > 2, MEM_P, constant size */
};

/* Multi-dimensional array with constant range */
static int md_array[4][6] = {
    [0 ... 1][2 ... 3] = 7  /* Nested constant ranges */
};

/* Function to force register initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Small struct that might go in register */
    register struct PackedStruct reg_target = {
        .a = 1,
        .b = 2,
        .c = 3
    };
    
    /* Use designated initializer with exactly 2 elements (count <= 2) */
    register int reg_arr[5] = { [0] = 42, [1] = 43 }; /* count = 2 */
    
    /* Force usage to prevent optimization */
    asm volatile("" : : "r"(reg_target.a), "r"(reg_arr[0]));
}

/* Function with various initialization patterns */
static void test_mixed_initializers(void) {
    /* Automatic array with constant range (count > 2, stack MEM_P) */
    int auto_arr[20] = { [L ... H] = 255 };  /* count = 4 */
    
    /* Exactly 1 element range (count <= 2) */
    int single_range[50] = { [25] = 100 };   /* count = 1 */
    
    /* Volatile ensures MEM_P classification */
    volatile int volatile_arr[30] = { 
        [5 ... 15] = 999  /* count = 11 > 2, MEM_P true */
    };
    
    /* Packed struct array with constant range */
    struct PackedStruct ps_arr[10] = {
        [2 ... 6] = { .a = 3, .b = 127, .c = 8 }  /* count = 5 */
    };
    
    /* Nested struct with array initialization */
    struct WithArray nested = {
        .header = -1,
        .data = { [1 ... 3] = 77 },  /* count = 3 */
        .ps = { .a = 6, .b = 200, .c = 12 }
    };
    
    /* Compound literal assignment (creates initialization context) */
    int *ptr = (int[8]){ [0 ... 7] = 88 };  /* count = 8 */
    
    /* Use all variables to prevent dead code elimination */
    printf("auto_arr[%d] = %d\n", L, auto_arr[L]);
    printf("single_range[25] = %d\n", single_range[25]);
    printf("volatile_arr[10] = %d\n", volatile_arr[10]);
    printf("ps_arr[4].b = %u\n", ps_arr[4].b);
    printf("nested.data[2] = %d\n", nested.data[2]);
    printf("*ptr = %d\n", *ptr);
}

/* Test with conditional constant initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector == 0) {
        /* This path should still be analyzed */
        int cond_arr[10] = { [0 ... 4] = 33 };  /* count = 5 */
        printf("cond_arr[2] = %d\n", cond_arr[2]);
    } else {
        /* Alternative with different count */
        int cond_arr2[10] = { [8] = 44 };  /* count = 1 */
        printf("cond_arr2[8] = %d\n", cond_arr2[8]);
    }
}

/* Test different element sizes */
static void test_various_sizes(void) {
    /* char - small element size */
    char char_arr[100] = { [10 ... 20] = 'A' };  /* count = 11 */
    
    /* short */
    short short_arr[50] = { [5 ... 15] = 32000 };  /* count = 11 */
    
    /* long long */
    long long ll_arr[20] = { [2 ... 8] = 0x123456789ABCDEFLL };  /* count = 7 */
    
    /* Mixed in struct */
    struct Mixed {
        char c;
        int i;
        long long ll;
    } mixed_arr[5] = {
        [1 ... 3] = { 'z', 42, 0xFF }
    };
    
    printf("char_arr[15] = %c\n", char_arr[15]);
    printf("short_arr[10] = %d\n", short_arr[10]);
    printf("ll_arr[5] = %llx\n", ll_arr[5]);
    printf("mixed_arr[2].i = %d\n", mixed_arr[2].i);
}

int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* 1. Register target with count <= 2 */
    test_register_target();
    
    /* 2. Global/static (MEM_P) with count > 2 */
    printf("global_arr[%d] = %d\n", BIG_START, global_arr[BIG_START]);
    printf("md_array[0][2] = %d\n", md_array[0][2]);
    
    /* 3. Mixed automatic variables */
    test_mixed_initializers();
    
    /* 4. Conditional initialization */
    test_conditional_init(0);
    test_conditional_init(1);
    
    /* 5. Various element sizes */
    test_various_sizes();
    
    /* Use global arrays to prevent elimination */
    int sum = global_arr[BIG_START] + md_array[0][2];
    
    return sum > 0 ? 0 : 1;
}

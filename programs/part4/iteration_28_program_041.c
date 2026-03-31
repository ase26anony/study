/* Test program to cover constant bounds checking in GCC's expr.cc
   Specifically targeting lines 7691-7700:
   const_bounds_p && tree_fits_shwi_p conditions with
   (!MEM_P(target) || count <= 2 || ...) logic
*/

#include <stdio.h>
#include <stddef.h>

/* Use enum to ensure constant bounds */
enum { 
    L1 = 0, 
    H1 = 1,      /* count = 2 */
    L2 = 10, 
    H2 = 90,     /* count = 81 > 2 */
    L3 = 5,
    H3 = 5       /* count = 1 */
};

/* Packed struct with constant bitfield size */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 4;
    /* Total size: 20 bits = 2.5 bytes, but padded to 3 bytes in practice */
};

/* Struct containing array */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* Global/static initializations - MEM_P(target) likely true */
static int global_arr[100] = { [L2 ... H2] = 99 };  /* count > 2, memory target */
static struct PackedStruct global_ps = { .a = 127, .b = 511, .c = 15 };

/* Multi-dimensional array with constant range */
static int md_arr[4][6] = { [1 ... 2][2 ... 4] = 7 };

/* Function to force register target (!MEM_P) scenarios */
static void test_register_target(void) {
    /* Small struct that might go in register */
    register struct PackedStruct reg_target = { 
        .a = 1, 
        .b = 2, 
        .c = 3 
    };
    
    /* Use designated initializer with constant range (count=2) */
    int reg_arr[10] = { [L1 ... H1] = 42 };  /* Could be register promoted */
    
    /* Force computation to prevent elimination */
    printf("reg_target.a = %u\n", reg_target.a);
    printf("reg_arr[0] = %d\n", reg_arr[0]);
}

/* Function with various initialization patterns */
static void test_mixed_initializers(void) {
    /* Automatic array with count <= 2 */
    int small_range[20] = { [L3] = 100 };  /* count = 1 */
    
    /* Automatic array with count > 2 */
    int large_range[50] = { [10 ... 40] = 255 };  /* count = 31 > 2 */
    
    /* Volatile ensures MEM_P(target) = true */
    volatile int volatile_arr[30] = { [5 ... 15] = 999 };
    
    /* Packed struct array with constant range */
    struct PackedStruct ps_arr[5] = { [1 ... 3] = { .a = 63, .b = 255, .c = 8 } };
    
    /* Nested struct with array initialization */
    struct WithArray nested = { 
        .header = -1,
        .data = { [2 ... 5] = 77 },  /* count = 4 > 2 */
        .ps = { .a = 3, .b = 7, .c = 1 }
    };
    
    /* Compound literal - creates initialization context */
    struct PackedStruct *ptr = &(struct PackedStruct){ 
        .a = 15, 
        .b = 31, 
        .c = 3 
    };
    
    /* Multi-dimensional with constant bounds */
    int local_md[3][5] = { [0 ... 1][1 ... 3] = 42 };
    
    /* Use all variables to prevent dead code elimination */
    printf("small_range[%d] = %d\n", L3, small_range[L3]);
    printf("large_range[25] = %d\n", large_range[25]);
    printf("volatile_arr[10] = %d\n", volatile_arr[10]);
    printf("ps_arr[2].b = %u\n", ps_arr[2].b);
    printf("nested.data[3] = %d\n", nested.data[3]);
    printf("local_md[1][2] = %d\n", local_md[1][2]);
}

/* Test with different optimization contexts */
__attribute__((noinline)) 
static void test_noinline_context(void) {
    /* Different count scenarios in noinline function */
    int arr1[10] = { [0 ... 9] = 1 };      /* count = 10 > 2 */
    int arr2[10] = { [3] = 2 };            /* count = 1 */
    int arr3[10] = { [4 ... 5] = 3 };      /* count = 2 */
    
    /* Mixed element types with constant sizes */
    char char_arr[100] = { [20 ... 80] = 'X' };      /* count = 61 > 2 */
    short short_arr[50] = { [10 ... 20] = 1234 };    /* count = 11 > 2 */
    long long_arr[30] = { [5 ... 15] = 987654321L }; /* count = 11 > 2 */
    
    printf("arr1[5] = %d\n", arr1[5]);
    printf("char_arr[50] = %c\n", char_arr[50]);
    printf("short_arr[15] = %d\n", short_arr[15]);
}

int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* 1. Register target scenario (!MEM_P(target)) */
    test_register_target();
    
    /* 2. Various initializations with different count values */
    test_mixed_initializers();
    
    /* 3. Additional context with noinline */
    test_noinline_context();
    
    /* 4. Use global arrays */
    printf("global_arr[50] = %d\n", global_arr[50]);
    printf("md_arr[1][3] = %d\n", md_arr[1][3]);
    
    /* 5. Array with exactly 2 elements (count == 2) */
    int exact_two[10] = { [2 ... 3] = 888 };
    printf("exact_two[2] = %d\n", exact_two[2]);
    
    /* 6. Very large range (triggers different expansion) */
    static int huge_range[1000] = { [100 ... 899] = 777 };
    printf("huge_range[500] = %d\n", huge_range[500]);
    
    /* 7. Zero-length array GNU extension */
    struct ZeroTail {
        int count;
        int data[];
    } *zt = &(struct ZeroTail){ 
        .count = 0 
    };
    (void)zt;
    
    /* 8. Designated initializers with computed but constant indices */
    const int ci = 3;
    int computed_const[10] = { [ci] = 9, [ci+1] = 10 };  /* Still constant */
    printf("computed_const[3] = %d\n", computed_const[3]);
    
    return 0;
}

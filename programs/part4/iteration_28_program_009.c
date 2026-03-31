/* test-array-init-bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700, by creating various
 * initialization scenarios with constant array/struct bounds.
 * 
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test-array-init-bounds.c -o test
 *   gcc -O2 -ftree-vectorize -std=gnu11 test-array-init-bounds.c -o test-opt
 */

#include <stdio.h>
#include <string.h>

/* Use enum to define constant bounds that will be folded by the front-end */
enum { L = 2, H = 5 };
enum { SMALL_COUNT = 2 };
enum { BIG_START = 10, BIG_END = 90 };

/* Packed struct with constant bitfield sizes to ensure TYPE_SIZE is constant */
struct PackedBitfield {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
} __attribute__((packed));

/* Struct containing an array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct PackedBitfield bits;
};

/* Global/static initializations (MEM_P(target) likely true) */

/* 1. Large constant array with wide range (count > 2, MEM_P true) */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* 2. Multi-dimensional array with constant nested range */
static int md_array[3][4] = { [0 ... 1][2 ... 3] = 5 };

/* 3. Packed struct array with constant range */
static struct PackedBitfield packed_arr[10] = { [1 ... 4] = { .a = 1, .b = 2, .c = 3 } };

/* 4. Struct with nested array initialization */
static struct WithArray global_struct = { 
    .header = 42,
    .data = { [1 ... 3] = 7 },
    .bits = { .a = 5, .b = 10, .c = 2 }
};

/* Function to force register-target initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Use 'register' keyword to hint at register allocation */
    register int reg_target = { [0] = 123 };  /* count = 1 */
    
    /* Small struct that might go into registers */
    register struct { short a; short b; } reg_struct = { [0 ... 1] = 255 };
    
    /* Compound literal assignment to a register variable */
    register int reg_from_compound;
    reg_from_compound = (int){ [0] = 999 };
    
    /* Use the values to prevent elimination */
    printf("Register targets: %d, %d, %d\n", 
           reg_target, reg_struct.a, reg_from_compound);
}

/* Function to test automatic variables with various conditions */
static void test_automatic_vars(void) {
    /* Case 1: count <= 2 with automatic array (likely MEM_P) */
    int small_range[10] = { [8] = 1, [9] = 2 };  /* count = 2 for last two */
    
    /* Case 2: count <= 1 with designated initializer */
    int single_elem[5] = { [3] = 42 };  /* count = 1 */
    
    /* Case 3: volatile ensures MEM_P(target) */
    volatile int volatile_arr[20] = { [5 ... 6] = 77 };  /* count = 2 */
    
    /* Case 4: Nested block with constant bounds */
    {
        const int local_lo = 0, local_hi = 3;
        int local_arr[10] = { [local_lo ... local_hi] = 11 };  /* count = 4 */
        
        /* Use attribute to affect alignment */
        int aligned_arr[8] __attribute__((aligned(16))) = { [2 ... 5] = 33 };
        
        printf("Local array[0..3] = %d, aligned[2] = %d\n", 
               local_arr[0], aligned_arr[2]);
    }
    
    /* Use values to prevent dead code elimination */
    printf("Automatic: %d, %d, %d\n", 
           small_range[8], single_elem[3], volatile_arr[5]);
}

/* Function with conditional initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* This array might be optimized differently */
        int cond_arr[6] = { [1 ... 4] = selector * 10 };  /* count = 4 */
        printf("Conditional array[2] = %d\n", cond_arr[2]);
    } else {
        /* Different range with count = 3 */
        int cond_arr[6] = { [0 ... 2] = -1 };  /* count = 3 */
        printf("Conditional array[1] = %d\n", cond_arr[1]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 0: {
            int switch_arr[5] = { [0 ... 1] = 100 };  /* count = 2 */
            printf("Switch case 0: %d\n", switch_arr[0]);
            break;
        }
        case 1: {
            int switch_arr[5] = { [2 ... 4] = 200 };  /* count = 3 */
            printf("Switch case 1: %d\n", switch_arr[2]);
            break;
        }
    }
}

/* Test struct with bitfields and constant bounds */
static void test_bitfield_struct(void) {
    struct PackedBitfield local_bits[4] = { 
        [0 ... 2] = { .a = 3, .b = 7, .c = 1 }  /* count = 3 */
    };
    
    /* Array of structs with array member */
    struct WithArray struct_arr[3] = {
        [0 ... 1] = { 
            .header = 1,
            .data = { [0 ... 2] = 9 },  /* Nested range */
            .bits = { .a = 2, .b = 4, .c = 1 }
        }
    };
    
    printf("Bitfield[0].a = %u, struct_arr[0].data[1] = %d\n",
           local_bits[0].a, struct_arr[0].data[1]);
}

/* Main function orchestrates all test cases */
int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* 1. Register target with count <= 2 */
    test_register_target();
    
    /* 2. Large static array (count > 2, MEM_P true) */
    printf("Big array[%d] = %d (count = %d)\n", 
           BIG_START, big_array[BIG_START], BIG_END - BIG_START + 1);
    
    /* 3. Multi-dimensional array */
    printf("Multi-dim[0][2] = %d\n", md_array[0][2]);
    
    /* 4. Automatic variables with various conditions */
    test_automatic_vars();
    
    /* 5. Conditional initialization */
    test_conditional_init(1);
    test_conditional_init(0);
    
    /* 6. Bitfield struct tests */
    test_bitfield_struct();
    
    /* 7. Global struct with nested range */
    printf("Global struct data[2] = %d\n", global_struct.data[2]);
    
    /* 8. Additional test: String array with range */
    char str_arr[10] = { [3 ... 7] = 'A' };  /* count = 5 */
    printf("String array[5] = %c\n", str_arr[5]);
    
    /* 9. Union with array member */
    union Mixed {
        int nums[4];
        char chars[16];
    } u = { .nums = { [1 ... 2] = 0xABCD } };  /* count = 2 */
    printf("Union nums[1] = 0x%X\n", u.nums[1]);
    
    /* Ensure all data is used */
    int sum = big_array[50] + md_array[1][3] + packed_arr[2].a + str_arr[4];
    printf("Final checksum: %d\n", sum);
    
    return 0;
}

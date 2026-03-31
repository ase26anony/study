/* test-array-init-bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700.
 * It uses GNU C extensions for designated initializers with constant ranges.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test-array-init-bounds.c -o test
 *   gcc -O2 -ftree-vectorize -std=gnu11 -fextended-identifiers test-array-init-bounds.c -o test-opt
 */

#include <stdio.h>
#include <stdint.h>

/* Use enum to define constant bounds that will be folded by the front-end */
enum { L = 2, H = 5 };
enum { BIG_START = 10, BIG_END = 90 };

/* Packed struct with constant bitfield sizes to ensure TYPE_SIZE is constant */
struct __attribute__((packed)) PackedBitfield {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 16;
};

/* Struct containing an array for nested initialization */
struct WithArray {
    int x;
    int arr[8];
    struct PackedBitfield pb;
};

/* Global/static initializations (MEM_P(target) likely true) */

/* 1. Large static array with wide constant range (count > 2, MEM_P true) */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* 2. Packed struct array with constant range */
static struct PackedBitfield pb_array[10] = { [1 ... 4] = { .a = 1, .b = 2, .c = 3 } };

/* 3. Multi-dimensional array with nested constant range */
static int md_array[5][6] = { [0 ... 2][1 ... 3] = 7 };

/* 4. Struct with array member initialized with constant range */
static struct WithArray global_s = { .x = 1, .arr = { [1 ... 5] = 42 } };

/* Function to prevent dead code elimination */
__attribute__((noinline)) void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n");
    
    /* ============================================================
     * SCENARIO 1: Register target with count <= 2
     * Goal: Trigger (!MEM_P(target) || count <= 2) with count <= 2
     * ============================================================ */
    {
        /* Small struct that may be initialized in registers */
        register struct { int a; int b; } reg_struct = { .a = 10, .b = 20 };
        use_value(reg_struct.a + reg_struct.b);
        
        /* Array designated initializer with exactly 2 elements range */
        int small_range[10] = { [3 ... 4] = 100 };
        use_value(small_range[3] + small_range[4]);
        
        /* Single element range (count = 1) */
        int single_range[5] = { [2] = 77 };
        use_value(single_range[2]);
    }
    
    /* ============================================================
     * SCENARIO 2: Memory target with count > 2 but small element size
     * Goal: Trigger third condition: 
     *   MEM_P(target) && count > 2 && tree_fits_uhwi_p(TYPE_SIZE(elttype))
     * ============================================================ */
    {
        /* Automatic array with constant wide range (stack memory, MEM_P true) */
        int auto_array[50] = { [L ... H] = 255 };
        use_value(auto_array[L] + auto_array[H]);
        
        /* Volatile ensures memory operand */
        volatile int volatile_array[20] = { [5 ... 15] = 999 };
        use_value(volatile_array[10]);
        
        /* Packed struct array automatic */
        struct PackedBitfield local_pb[8] = { [0 ... 3] = { .a = 5, .b = 10, .c = 15 } };
        use_value(local_pb[1].c);
    }
    
    /* ============================================================
     * SCENARIO 3: Nested aggregates with constant bounds
     * ============================================================ */
    {
        /* Multi-dimensional array automatic */
        int local_md[4][5] = { [0 ... 2][1 ... 3] = 88 };
        use_value(local_md[1][2]);
        
        /* Struct with nested array initialization */
        struct WithArray local_s = { 
            .x = 2, 
            .arr = { [0 ... 7] = 33 },  /* Full range */
            .pb = { .a = 3, .b = 6, .c = 9 }
        };
        use_value(local_s.arr[3] + local_s.pb.c);
        
        /* Array of structs with constant range */
        struct WithArray struct_array[6] = { [1 ... 4] = { .x = 9, .arr = { [2 ... 3] = 11 } } };
        use_value(struct_array[2].arr[2]);
    }
    
    /* ============================================================
     * SCENARIO 4: Compound literals (create initialization contexts)
     * ============================================================ */
    {
        /* Compound literal assignment - target may be memory or register */
        struct PackedBitfield *pb_ptr = &(struct PackedBitfield){ .a = 1, .b = 2, .c = 3 };
        use_value(pb_ptr->c);
        
        /* Array compound literal with range */
        int *arr_ptr = (int[10]){ [2 ... 7] = 123 };
        use_value(arr_ptr[5]);
    }
    
    /* ============================================================
     * SCENARIO 5: Mixed with conditional code (constant conditions)
     * ============================================================ */
    if (1) {  /* Always true, but creates a basic block */
        /* Array with constant range in conditional block */
        int cond_array[8] = { [0 ... 3] = 444 };
        use_value(cond_array[2]);
    }
    
    switch (1) {
        case 1: {
            /* Switch case with initialization */
            int switch_array[5] = { [1 ... 4] = 555 };
            use_value(switch_array[3]);
            break;
        }
        default:
            break;
    }
    
    /* ============================================================
     * Use global/static arrays to prevent optimization
     * ============================================================ */
    use_value(big_array[BIG_START] + big_array[BIG_END]);
    use_value(md_array[1][2]);
    use_value(global_s.arr[2]);
    use_value(pb_array[2].b);
    
    printf("All initializations completed.\n");
    return 0;
}

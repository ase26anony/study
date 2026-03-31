/* test_expr_constant_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically the uncovered block that handles
 * constant-bounded array/aggregate initialization with various target
 * and count conditions.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_constant_bounds.c -o test_expr_constant_bounds
 *   gcc -O2 -ftree-vectorize -std=gnu11 test_expr_constant_bounds.c -o test_expr_constant_bounds_opt
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== 1. Constant bounds via enum ==================== */
enum ConstBounds {
    LOW_IDX = 2,
    HIGH_IDX = 5,
    SMALL_COUNT = 2
};

/* ==================== 2. Small packed struct ==================== */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* ==================== 3. Struct containing array ==================== */
struct WithArray {
    int header;
    int data[8];
    struct Packed7_9 packed;
};

/* ==================== 4. Multi-dimensional array type ==================== */
typedef int Matrix[4][6];

/* ==================== Helper to prevent dead code elimination ==================== */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(const void *p) {
    volatile const void *sink = p;
    (void)sink;
}

int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* ==================== SCENARIO 1: count <= 2 ==================== */
    /* This should trigger the count <= 2 path regardless of MEM_P(target) */
    {
        /* Automatic array with exactly 2 elements in range */
        int small_range[10] = { [LOW_IDX ... LOW_IDX + SMALL_COUNT - 1] = 42 };
        use_int(small_range[LOW_IDX]);
        use_int(small_range[LOW_IDX + 1]);
        
        /* Single element range (count = 1) */
        int single[5] = { [3] = 99 };
        use_int(single[3]);
    }
    
    /* ==================== SCENARIO 2: !MEM_P(target) ==================== */
    /* Try to force register target with small aggregate initialization */
    {
        /* Using register keyword hints register allocation */
        register struct Packed7_9 reg_target = { .a = 0x3F, .b = 0x1FF };
        use_int(reg_target.a + reg_target.b);
        
        /* Compound literal assigned to register variable */
        register int reg_arr[3] = { [0 ... 2] = 7 };
        use_int(reg_arr[1]);
    }
    
    /* ==================== SCENARIO 3: count > 2 && MEM_P(target) ==================== */
    /* Large constant array with wide range - definitely memory target */
    {
        static int large_array[100] = { 
            [10 ... 90] = 123,  /* count = 81 > 2, MEM_P(target) true */
            [0] = 1, [99] = 2   /* ensure edges are different for use */
        };
        use_int(large_array[10]);
        use_int(large_array[50]);
        use_int(large_array[90]);
        
        /* Another with different element type (char) */
        static char char_big[256] = { [32 ... 127] = 'A' };
        use_int(char_big[64]);
    }
    
    /* ==================== SCENARIO 4: Volatile ensures MEM_P ==================== */
    {
        volatile int vol_array[20] = { [5 ... 15] = 999 };
        use_int(vol_array[10]);
    }
    
    /* ==================== SCENARIO 5: Multi-dimensional array ==================== */
    {
        Matrix md = { [0 ... 1][2 ... 3] = 88 };
        use_int(md[0][2]);
        use_int(md[1][3]);
        
        /* 3D array with constant range */
        int threeD[3][4][5] = { [0 ... 2][1 ... 2][3 ... 4] = 77 };
        use_int(threeD[1][2][4]);
    }
    
    /* ==================== SCENARIO 6: Nested struct with array ==================== */
    {
        struct WithArray s = { 
            .header = -1,
            .data = { [1 ... 3] = 456 },  /* count = 3 > 2 */
            .packed = { .a = 0x7F, .b = 0x1FF }
        };
        use_int(s.data[2]);
        use_int(s.packed.a);
    }
    
    /* ==================== SCENARIO 7: Mixed with conditional ==================== */
    {
        const int flag = 1;  /* compile-time constant */
        if (flag) {
            /* This initialization should still be processed */
            int cond_array[8] = { [2 ... 5] = flag * 100 };
            use_int(cond_array[3]);
        }
        
        switch (3) {  /* constant switch */
            case 3: {
                int switch_array[5] = { [0 ... 4] = 333 };
                use_int(switch_array[2]);
                break;
            }
        }
    }
    
    /* ==================== SCENARIO 8: Attribute-aligned array ==================== */
    {
        int aligned_arr[16] __attribute__((aligned(64))) = { [4 ... 12] = 1024 };
        use_ptr(aligned_arr);
        use_int(aligned_arr[8]);
    }
    
    /* ==================== SCENARIO 9: Zero-length array in struct (GNU extension) ==================== */
    struct Flex {
        int count;
        int items[];
    };
    
    /* Compound literal with designators */
    {
        struct Flex *flex_ptr = &(struct Flex){ 
            .count = 3,
            /* Note: can't initialize flexible array member with designator ranges
               in standard C, but included as example of compound literal target */
        };
        use_int(flex_ptr->count);
    }
    
    printf("All constant bounds initializations completed.\n");
    
    return 0;
}

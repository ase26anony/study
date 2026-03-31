/* test_expr_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically the uncovered block that handles
 * array/aggregate initialization with constant bounds.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_bounds.c -o test_expr_bounds
 *   gcc -O2 -ftree-vectorize -std=gnu11 test_expr_bounds.c -o test_expr_bounds_opt
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== PART 1: Constant bounds definitions ==================== */
enum ConstBounds {
    L1 = 2,
    H1 = 5,
    L2 = 0,
    H2 = 1,
    L3 = 10,
    H3 = 90
};

const int C_LOW = 3;
const int C_HIGH = 8;

/* ==================== PART 2: Small packed struct with constant size ==================== */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* ==================== PART 3: Struct containing an array ==================== */
struct WithArray {
    int header;
    int data[6];
    struct Packed7_9 packed;
};

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    int result = 0;
    
    /* ==================== SCENARIO 1: count <= 2 (path: count <= 2) ==================== */
    {
        /* Automatic array with exactly 2 elements initialized via range */
        int small_range[10] = { [L2 ... H2] = 42 };  /* count = 2 */
        result += small_range[0] + small_range[1];
    }
    
    /* ==================== SCENARIO 2: !MEM_P(target) (register target) ==================== */
    {
        /* Use a small struct that likely goes into a register during initialization */
        register struct Packed7_9 reg_target = { .a = 0x3F, .b = 0x1FF };
        /* Compound literal with constant range (single element) */
        reg_target = (struct Packed7_9){ .a = 0x20, .b = 0x100 };
        result += reg_target.a + reg_target.b;
    }
    
    /* ==================== SCENARIO 3: MEM_P(target) && count > 2 && constant element size ==================== */
    {
        /* Large static array with wide constant range (count = 81 > 2) */
        static int big_array[100] = { [L3 ... H3] = 0xABCD };
        /* Use volatile to ensure MEM_P classification */
        volatile int *vol_ptr = &big_array[L3];
        result += *vol_ptr;
    }
    
    /* ==================== SCENARIO 4: Multi-dimensional array with constant nested range ==================== */
    {
        int md[3][4] = { [0 ... 1][2 ... 3] = 5 };  /* count = 4 > 2 */
        result += md[0][2] + md[1][3];
    }
    
    /* ==================== SCENARIO 5: Struct with array member using constant range ==================== */
    {
        struct WithArray s = { 
            .header = 1, 
            .data = { [C_LOW ... C_HIGH] = 99 },  /* count = 6 > 2 */
            .packed = { .a = 1, .b = 2 }
        };
        result += s.data[C_LOW] + s.data[C_HIGH];
    }
    
    /* ==================== SCENARIO 6: Enum bounds with automatic array ==================== */
    {
        int enum_arr[10] = { [L1 ... H1] = 123 };  /* count = 4 > 2 */
        result += enum_arr[L1] + enum_arr[H1];
    }
    
    /* ==================== SCENARIO 7: Volatile memory target (ensures MEM_P) ==================== */
    {
        volatile int volatile_arr[20] = { [5 ... 9] = 777 };  /* count = 5 > 2 */
        result += volatile_arr[5];
    }
    
    /* ==================== SCENARIO 8: Mixed initializers with constant ranges ==================== */
    {
        int mixed[10] = { 0, 1, [L1 ... H1] = 2, 3, 4 };  /* count = 4 > 2 */
        result += mixed[0] + mixed[L1];
    }
    
    /* ==================== SCENARIO 9: Nested block with automatic array ==================== */
    if (1) {  /* Constant condition ensures initialization is parsed */
        int nested_arr[15] = { [2 ... 12] = 888 };  /* count = 11 > 2 */
        result += nested_arr[2];
    }
    
    /* ==================== SCENARIO 10: Bit-field array with constant range ==================== */
    {
        struct BitFieldArray {
            unsigned int bits[8];
        } bfa = { .bits = { [1 ... 4] = 0xFFFFFFFF } };  /* count = 4 > 2 */
        result += (bfa.bits[1] > 0) ? 1 : 0;
    }
    
    printf("Result: %d\n", result);
    return 0;
}

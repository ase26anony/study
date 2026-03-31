/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>
#include <stdint.h>

/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test.c -o test */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test.c -o test_opt */

/* ========== SCENARIO 1: Register target with !MEM_P(target) ========== */
/* Small struct that fits in registers */
struct SmallReg {
    int a;
    short b;
    char c;
} __attribute__((packed));

/* Packed struct with constant bitfield sizes */
struct PackedBitfield {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 4;
} __attribute__((packed));

/* ========== SCENARIO 2: Memory target with count <= 2 ========== */
/* Use enum for constant bounds */
enum { L = 2, H = 3 };  /* count = 2 */

/* ========== SCENARIO 3: Memory target with count > 2 and constant size ========== */
/* Large array with wide range */
#define BIG_START 10
#define BIG_END   90  /* count = 81 > 2 */

/* ========== SCENARIO 4: Multi-dimensional array ========== */
/* Constant bounds for multi-dim */
enum { DIM1_START = 0, DIM1_END = 1, DIM2_START = 2, DIM2_END = 3 };

/* Static initializers (MEM_P target) */
static int static_arr[100] = { [L ... H] = 42 };  /* count = 2, MEM_P true */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };  /* count > 2, MEM_P true */

/* Struct with array member */
struct WithArray {
    int x;
    int arr[10];
    char tag;
};

/* Global with designated init */
struct WithArray global_struct = { 
    .x = 1,
    .arr = { [2 ... 5] = 7 },  /* count = 4 > 2 */
    .tag = 'A'
};

/* Multi-dimensional array static */
static int md_static[4][5] = { 
    [DIM1_START ... DIM1_END][DIM2_START ... DIM2_END] = 123 
};

int main(void) {
    int result = 0;
    
    /* ========== Test 1: Register target (!MEM_P) ========== */
    /* Force register storage with 'register' keyword and small initialization */
    {
        register struct SmallReg reg_target = { 
            .a = 1,
            .b = 2,
            .c = 3
        };
        /* Use compound literal to potentially trigger initialization path */
        register struct PackedBitfield reg_bitfield = (struct PackedBitfield){ 
            .a = 0x7F,  /* max 7-bit */
            .b = 0x1FF, /* max 9-bit */
            .c = 0xF    /* max 4-bit */
        };
        
        /* Use the values to prevent elimination */
        result += reg_target.a + reg_bitfield.a;
    }
    
    /* ========== Test 2: Automatic variable with count <= 2 ========== */
    {
        int auto_arr[10] = { [0] = 1, [1] = 2 };  /* Two separate initializers, count=1 each? */
        /* Actually for range: */
        int auto_range[10] = { [3 ... 4] = 5 };   /* count = 2 */
        
        result += auto_arr[0] + auto_range[3];
    }
    
    /* ========== Test 3: Volatile memory target ========== */
    /* Volatile forces MEM_P classification */
    {
        volatile int volatile_arr[20] = { [5 ... 9] = 42 };  /* count = 5 > 2 */
        result += volatile_arr[5];
    }
    
    /* ========== Test 4: Nested block with automatic array ========== */
    {
        /* Mixed initialization with partial range */
        char char_arr[50] = { 
            [10] = 'A',
            [20 ... 25] = 'B',  /* count = 6 > 2 */
            [30 ... 35] = 'C'   /* count = 6 > 2 */
        };
        
        /* Struct with array member, automatic storage */
        struct WithArray local_struct = {
            .x = 100,
            .arr = { [1 ... 3] = 999 },  /* count = 3 > 2 */
            .tag = 'L'
        };
        
        result += char_arr[10] + local_struct.arr[1];
    }
    
    /* ========== Test 5: Multi-dimensional array automatic ========== */
    {
        int md_auto[3][4] = { [0 ... 1][2 ... 3] = 55 };  /* Nested range */
        
        /* Different element type sizes */
        short short_md[5][6] = { [1 ... 3][2 ... 4] = 32767 };
        
        result += md_auto[0][2] + short_md[1][2];
    }
    
    /* ========== Test 6: Zero-length range (count = 1) ========== */
    {
        int single[10] = { [7] = 77 };  /* Single element, count = 1 */
        result += single[7];
    }
    
    /* ========== Test 7: Using __builtin_constant_p to assert constness ========== */
    {
        /* These should all be constant at compile time */
        if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
            int checked_arr[10] = { [L ... H] = 88 };
            result += checked_arr[L];
        }
    }
    
    /* ========== Test 8: Array of structs with range init ========== */
    {
        struct Point { int x, y; } points[10] = { 
            [2 ... 5] = { .x = 10, .y = 20 }  /* count = 4 > 2 */
        };
        result += points[2].x;
    }
    
    /* ========== Test 9: Very large count with small element ========== */
    {
        /* char has size 1, which fits in uhwi */
        char massive[1000] = { [100 ... 899] = 'X' };  /* count = 800 > 2 */
        result += massive[100];
    }
    
    /* ========== Test 10: Conditional initialization ========== */
    {
        /* Constant condition ensures initialization is parsed */
        if (1) {  /* Always true, but constant */
            int cond_arr[10] = { [1 ... 8] = 33 };  /* count = 8 > 2 */
            result += cond_arr[1];
        }
        
        switch (1) {  /* Constant switch */
            case 1: {
                int switch_arr[5] = { [0 ... 2] = 44 };  /* count = 3 > 2 */
                result += switch_arr[0];
                break;
            }
        }
    }
    
    /* Use static/global variables to prevent elimination */
    result += static_arr[L] + big_array[BIG_START] + global_struct.arr[2] + md_static[0][2];
    
    printf("Result: %d\n", result);
    printf("All initializations executed.\n");
    
    return 0;
}

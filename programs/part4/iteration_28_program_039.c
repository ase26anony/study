/* Test program to cover constant bounds checking in GCC's expr.cc
 * Specifically targeting lines 7691-7700:
 *   const_bounds_p && tree_fits_shwi_p (lo_index) && tree_fits_shwi_p (hi_index)
 *   with conditions on MEM_P(target) and count
 */

#include <stdio.h>
#include <stdint.h>

/* ==================== PART 1: Trigger count <= 2 path ==================== */
/* This should trigger the count <= 2 condition regardless of MEM_P(target) */

/* Struct with bitfields to ensure constant size */
struct SmallStruct {
    int a:7;
    int b:9;
    int c:3;
} __attribute__((packed));

/* Enum for constant bounds */
enum { L1 = 0, H1 = 1 };  /* count = 2 */

/* ==================== PART 2: Trigger !MEM_P(target) path ==================== */
/* Force initialization into register target with small struct */

struct Point {
    int x;
    int y;
} __attribute__((aligned(4)));

/* ==================== PART 3: Trigger MEM_P(target) && count > 2 path ==================== */
/* Large array with wide constant range, element size fits in uhwi */

#define BIG_START 10
#define BIG_END   90  /* count = 81 > 2 */

/* ==================== PART 4: Multi-dimensional array with constant ranges ==================== */

enum { MDIM1_START = 0, MDIM1_END = 1, MDIM2_START = 2, MDIM2_END = 3 };

/* ==================== PART 5: Volatile memory target ==================== */

/* Global/static variables are always memory targets */
static int static_array[100] = { [5 ... 15] = 123 };  /* count = 11 > 2 */

/* Function to use variables and prevent dead code elimination */
__attribute__((noinline))
static void use_var(const void *p, int size) {
    volatile int sink = 0;
    (void)p;
    (void)size;
    sink = 1;
}

int main(void) {
    printf("Testing constant bounds initialization paths\n");
    
    /* ========== Test 1: count <= 2 with automatic variable ========== */
    {
        /* Automatic array with exactly 2 elements in range */
        int arr1[10] = { [L1 ... H1] = 42 };  /* count = 2 */
        use_var(arr1, sizeof(arr1));
        printf("Test1: arr1[0]=%d, arr1[1]=%d\n", arr1[0], arr1[1]);
    }
    
    /* ========== Test 2: !MEM_P(target) with register struct ========== */
    {
        /* Try to force register target with small struct initialization */
        register struct Point reg_target = { .x = 1, .y = 2 };
        /* Compound literal assignment might create initialization context */
        struct Point mem_target;
        mem_target = (struct Point){ .x = 3, .y = 4 };
        use_var(&reg_target, sizeof(reg_target));
        use_var(&mem_target, sizeof(mem_target));
        printf("Test2: reg_target.x=%d, mem_target.x=%d\n", reg_target.x, mem_target.x);
    }
    
    /* ========== Test 3: MEM_P(target) && count > 2 ========== */
    {
        /* Large automatic array with wide constant range */
        int big_array[100] = { [BIG_START ... BIG_END] = 99 };  /* count = 81 > 2 */
        use_var(big_array, sizeof(big_array));
        printf("Test3: big_array[%d]=%d, big_array[%d]=%d\n", 
               BIG_START, big_array[BIG_START], BIG_END, big_array[BIG_END]);
    }
    
    /* ========== Test 4: Static/global memory target ========== */
    {
        /* static_array already initialized */
        use_var(static_array, sizeof(static_array));
        printf("Test4: static_array[5]=%d, static_array[15]=%d\n", 
               static_array[5], static_array[15]);
    }
    
    /* ========== Test 5: Volatile memory target ========== */
    {
        /* Volatile forces memory operand */
        volatile int volatile_array[20] = { [3 ... 8] = 77 };  /* count = 6 > 2 */
        use_var((void*)volatile_array, sizeof(volatile_array));
        printf("Test5: volatile_array[3]=%d\n", volatile_array[3]);
    }
    
    /* ========== Test 6: Multi-dimensional array with constant ranges ========== */
    {
        /* GCC extension for multi-dimensional designated initializers */
        int md_array[3][4] = { 
            [MDIM1_START ... MDIM1_END][MDIM2_START ... MDIM2_END] = 55 
        };
        use_var(md_array, sizeof(md_array));
        printf("Test6: md_array[0][2]=%d, md_array[1][3]=%d\n", 
               md_array[0][2], md_array[1][3]);
    }
    
    /* ========== Test 7: Nested struct with array ========== */
    {
        struct Container {
            int id;
            int data[8];
        };
        
        struct Container c = { 
            .id = 100,
            .data = { [1 ... 4] = 200 }  /* count = 4 > 2 */
        };
        use_var(&c, sizeof(c));
        printf("Test7: c.data[1]=%d, c.data[4]=%d\n", c.data[1], c.data[4]);
    }
    
    /* ========== Test 8: Small packed struct with bitfields ========== */
    {
        struct SmallStruct ss = { .a = 1, .b = 2, .c = 3 };
        /* Array of packed structs with range initialization */
        struct SmallStruct ss_array[5] = { [0 ... 2] = { .a = 4, .b = 5, .c = 6 } };
        use_var(&ss, sizeof(ss));
        use_var(ss_array, sizeof(ss_array));
        printf("Test8: ss.a=%d, ss_array[1].b=%d\n", ss.a, ss_array[1].b);
    }
    
    /* ========== Test 9: Conditional initialization with constant condition ========== */
    {
        int cond_array[10];
        if (1) {  /* Always true constant condition */
            /* This initialization should still be processed */
            int temp[10] = { [2 ... 5] = 888 };  /* count = 4 > 2 */
            for (int i = 0; i < 10; i++) cond_array[i] = temp[i];
        }
        use_var(cond_array, sizeof(cond_array));
        printf("Test9: cond_array[2]=%d\n", cond_array[2]);
    }
    
    /* ========== Test 10: Character array with different element size ========== */
    {
        char char_array[50] = { [10 ... 40] = 'X' };  /* count = 31 > 2, eltsize = 1 */
        use_var(char_array, sizeof(char_array));
        printf("Test10: char_array[10]=%c, char_array[40]=%c\n", 
               char_array[10], char_array[40]);
    }
    
    return 0;
}

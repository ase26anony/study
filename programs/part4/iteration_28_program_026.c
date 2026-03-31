/* expr_coverage.c
 * 
 * This program is designed to exercise the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700.
 * It uses GNU C extensions for designated initializers with ranges.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers expr_coverage.c -o expr_coverage
 *   gcc -O2 -ftree-vectorize -std=gnu11 expr_coverage.c -o expr_coverage_opt
 */

#include <stdio.h>
#include <string.h>

/* ==================== 1. Constant bounds via enum ==================== */
enum { L = 2, H = 5 };

/* ==================== 2. Small packed struct with constant size ==================== */
struct Packed {
    int a : 7;
    int b : 9;
} __attribute__((packed));

/* ==================== 3. Struct containing an array ==================== */
struct WithArray {
    int x;
    int arr[10];
};

/* ==================== 4. Multi-dimensional array type ==================== */
typedef int MD[3][4];

/* ==================== 5. Static large array (MEM_P target, count > 2) ==================== */
static int big_array[100] = { [10 ... 90] = 99 };  /* count = 81 > 2 */

/* ==================== 6. Static packed struct array ==================== */
static struct Packed packed_array[8] = { [1 ... 6] = { .a = 3, .b = 5 } }; /* count = 6 > 2 */

/* ==================== 7. Global struct with array designated init ==================== */
struct WithArray global_struct = { .x = 1, .arr = { [L ... H] = 42 } }; /* count = 4 > 2 */

/* ==================== 8. Multi-dimensional global ==================== */
MD global_md = { [0 ... 1][2 ... 3] = 7 }; /* 2x2 = 4 elements > 2 */

/* ==================== Helper to prevent dead code elimination ==================== */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(const void *p) {
    volatile const void *sink = p;
    (void)sink;
}

int main() {
    /* ==================== Scenario A: Register target (!MEM_P) ==================== */
    /* Small struct that may be initialized in registers */
    register struct Packed reg_target = { .a = 1, .b = 2 };  /* No range, but constant init */
    /* Use designated range on a local scalar array? Not directly possible.
       Instead, use a compound literal with a range to try to get a register target. */
    int reg_arr[2] = { [0 ... 1] = 37 };  /* count = 2, may be MEM_P or register depending */
    use_int(reg_arr[0]);

    /* ==================== Scenario B: count <= 2 with MEM_P target ==================== */
    volatile int small_range[10] = { [5] = 100 };  /* count = 1, volatile forces MEM_P */
    use_int(small_range[5]);

    volatile int two_elem[10] = { [3 ... 4] = 200 };  /* count = 2, volatile forces MEM_P */
    use_int(two_elem[3]);

    /* ==================== Scenario C: Automatic array, count > 2, MEM_P ==================== */
    int auto_big[50] = { [10 ... 40] = 123 };  /* count = 31 > 2, stack memory */
    use_int(auto_big[20]);

    /* ==================== Scenario D: Nested block with constant bounds ==================== */
    {
        /* Enum bounds used in automatic array */
        int local_arr[10] = { [L ... H] = 999 };  /* count = 4 > 2 */
        use_int(local_arr[L]);
    }

    /* ==================== Scenario E: Multi-dimensional local ==================== */
    MD local_md = { [0 ... 2][1 ... 2] = 55 };  /* 3x2 = 6 > 2 */
    use_int(local_md[0][1]);

    /* ==================== Scenario F: Packed struct local with range ==================== */
    struct Packed local_packed[5] = { [0 ... 4] = { .a = 6, .b = 9 } }; /* count = 5 > 2 */
    use_int(local_packed[0].a);

    /* ==================== Scenario G: Compound literal assignment ==================== */
    struct WithArray *ptr = &global_struct;
    *ptr = (struct WithArray){ .arr = { [1 ... 8] = 77 } }; /* count = 8 > 2 */
    use_int(ptr->arr[2]);

    /* ==================== Use globals to prevent elimination ==================== */
    use_int(big_array[50]);
    use_int(packed_array[2].a);
    use_int(global_struct.arr[3]);
    use_int(global_md[1][2]);

    /* Print something to see effects */
    printf("All initializations performed.\n");
    printf("big_array[50] = %d\n", big_array[50]);
    printf("global_md[1][2] = %d\n", global_md[1][2]);

    return 0;
}

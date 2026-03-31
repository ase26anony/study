/* test-array-init-bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700, by creating various
 * constant‑bounded initializations that exercise different paths through
 * the condition:
 *
 *   (!MEM_P(target) || count <= 2 || (tree_fits_uhwi_p(TYPE_SIZE(elttype)) && ...))
 *
 * We use GNU C extensions (designated initializer ranges, packed structs,
 * compound literals, etc.) to ensure the compiler sees constant bounds
 * and chooses different expansion strategies.
 */

#include <stdio.h>
#include <stdint.h>

/* ------------------------------------------------------------------
 * 1. Register target with count <= 2: trigger !MEM_P(target) path
 * ------------------------------------------------------------------ */
static void test_register_target(void)
{
    /* A small struct that can be initialized into a register.
       Use designated range for exactly 2 elements. */
    struct Small {
        char a;
        char b;
        char c;
    } __attribute__((packed));

    /* 'reg' is likely to be a register target during initialization.
       The range [0...1] gives count = 2, satisfying count <= 2. */
    register struct Small reg = { [0 ... 1] = 0x55 };
    printf("reg target: a=%d, b=%d, c=%d\n", reg.a, reg.b, reg.c);
}

/* ------------------------------------------------------------------
 * 2. Static array with wide constant range: count > 2, MEM_P(target) true
 * ------------------------------------------------------------------ */
/* Large static array, initialized with a wide constant range.
   This will be in .data section → definitely a memory operand.
   count = 91 - 10 + 1 = 82 > 2, so the third condition must be evaluated. */
static int big_array[100] = { [10 ... 90] = 0x12345678 };

/* ------------------------------------------------------------------
 * 3. Automatic array with constant range, possibly register‑promoted
 * ------------------------------------------------------------------ */
static void test_auto_array(void)
{
    /* Automatic array with constant range of 3 elements (count > 2).
       Depending on optimization, the compiler might try to use registers
       for parts of the initialization. */
    int auto_arr[10] = { [3 ... 5] = 99 };
    printf("auto_arr[3]=%d, auto_arr[4]=%d, auto_arr[5]=%d\n",
           auto_arr[3], auto_arr[4], auto_arr[5]);
}

/* ------------------------------------------------------------------
 * 4. Volatile array: forces MEM_P(target) to be true
 * ------------------------------------------------------------------ */
static void test_volatile_array(void)
{
    /* Volatile qualifier ensures the target is treated as a memory operand.
       Use a range of 4 elements (count > 2). */
    volatile char volatile_arr[8] = { [1 ... 4] = 0xFF };
    printf("volatile_arr[1]=%d\n", (int)volatile_arr[1]);
}

/* ------------------------------------------------------------------
 * 5. Multi‑dimensional array with nested constant ranges
 * ------------------------------------------------------------------ */
static void test_multi_dim(void)
{
    /* 2D array with a constant range in both dimensions.
       This stresses the bounds‑checking logic recursively. */
    int md[5][5] = { [1 ... 3][2 ... 4] = 7 };
    printf("md[2][3] = %d\n", md[2][3]);
}

/* ------------------------------------------------------------------
 * 6. Struct containing an array with constant range
 * ------------------------------------------------------------------ */
struct Container {
    int header;
    int data[10];
    int footer;
};

static void test_struct_with_array(void)
{
    /* Initialize only a sub‑range of the inner array. */
    struct Container c = { .data = { [2 ... 6] = 42 } };
    printf("c.data[3] = %d\n", c.data[3]);
}

/* ------------------------------------------------------------------
 * 7. Enum‑defined bounds (compile‑time constants)
 * ------------------------------------------------------------------ */
enum { L = 3, H = 7 };

static void test_enum_bounds(void)
{
    int arr[10] = { [L ... H] = 0xABCD };
    printf("arr[5] = %d\n", arr[5]);
}

/* ------------------------------------------------------------------
 * 8. Compound literal assignment (creates an initialization context)
 * ------------------------------------------------------------------ */
static void test_compound_literal(void)
{
    struct Point { int x; int y; } p;
    /* Compound literal with designated initializers (no range here,
       but still a constant initializer). */
    p = (struct Point){ .x = 10, .y = 20 };
    printf("p.x = %d, p.y = %d\n", p.x, p.y);
}

/* ------------------------------------------------------------------
 * 9. Packed struct with odd constant size (ensures TYPE_SIZE fits in UHWI)
 * ------------------------------------------------------------------ */
struct PackedOdd {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 13;
} __attribute__((packed));

static void test_packed_struct(void)
{
    /* Initialize all fields via a range? Not directly possible with bit‑fields,
       but we can initialize the whole struct via a compound literal. */
    struct PackedOdd po = { .a = 0x7F, .b = 0x1FF, .c = 0x1FFF };
    printf("packed struct size = %zu bytes\n", sizeof(po));
}

/* ------------------------------------------------------------------
 * 10. Conditional initialization with constant condition
 * ------------------------------------------------------------------ */
static void test_conditional_init(void)
{
    int x = 1;
    /* The condition is constant after inlining / optimization,
       but the initialization is still parsed. */
    if (__builtin_constant_p(x) ? 1 : 0) {
        int cond_arr[8] = { [0 ... 3] = x };
        printf("cond_arr[2] = %d\n", cond_arr[2]);
    }
}

/* ------------------------------------------------------------------
 * main: call all test functions to ensure code is executed
 * ------------------------------------------------------------------ */
int main(void)
{
    test_register_target();
    printf("big_array[50] = %d\n", big_array[50]);
    test_auto_array();
    test_volatile_array();
    test_multi_dim();
    test_struct_with_array();
    test_enum_bounds();
    test_compound_literal();
    test_packed_struct();
    test_conditional_init();
    return 0;
}

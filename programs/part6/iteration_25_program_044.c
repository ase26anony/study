/* test_complex_types.c
 * Complex C declarations to exercise gengtype-parse.cc's consume_balanced()
 */

/* ===== 1. Complex Function Declarators with Nested Parentheses ===== */

/* Pointer to function taking pointer to function (returning int), returning pointer to function (returning int) */
int (*(*func_ptr1)(int (*)(double)))(char);

/* Typedef chain building complex function pointer types */
typedef int (*simple_fn)(void);
typedef simple_fn (*fn_returning_fn)(int);
typedef fn_returning_fn (*super_fn)(char, double);

/* Global instance */
super_fn global_super = 0;

/* Function to assign to function pointers */
int simple_func(void) { return 42; }
simple_fn get_simple_fn(void) { return simple_func; }
fn_returning_fn get_fn_returner(void) { 
    static fn_returning_fn f = 0;
    if (!f) {
        /* Nested function to return */
        simple_fn local_getter(void) { return simple_func; }
        f = local_getter;
    }
    return f;
}

/* ===== 2. Multi-Dimensional and Complex Array Declarations ===== */

/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Array with parenthesized size expression */
char (*strings[(2+3)])[20];

/* Three-dimensional array with typedef */
typedef int matrix[3][3];
matrix multi_array[2];

/* ===== 3. Nested Aggregate Initializers and Type Definitions ===== */

/* Deeply nested struct */
struct Level3 {
    float data[2];
    struct { unsigned char bits[4]; } packed;
};

struct Level2 {
    int id;
    struct Level3 l3;
    void (*callback)(struct Level3*);
};

struct Level1 {
    struct Level2 items[2];
    union {
        long big;
        short small[4];
    } choice;
};

/* Global with deeply nested initializer */
struct Level1 global_nested = {
    .items = {
        [0] = {
            .id = 100,
            .l3 = {
                .data = {3.14f, 2.718f},
                .packed = { .bits = {0xAB, 0xCD, 0xEF, 0x12} }
            },
            .callback = 0
        },
        [1] = {
            .id = 200,
            .l3 = {
                .data = {1.414f, 1.732f},
                .packed = { .bits = {0x34, 0x56, 0x78, 0x9A} }
            },
            .callback = 0
        }
    },
    .choice = { .big = 0x12345678ABCDEF00ULL }
};

/* Array with designated initializers */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* ===== 4. Combine Constructs in Single Declarations ===== */

/* Function pointer with array parameter and struct return */
struct Small { int x; char c; };
struct Small (*(*complex_mixer)(int (*)(int), struct Small[]))[2];

/* Variable with compound literal initializer */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Union containing function pointer array */
union Mix {
    int (*func_array[3])(void);
    struct { char *name; int value; } meta;
} global_mix = {
    .func_array = { simple_func, simple_func, simple_func }
};

/* ===== 5. Additional Global Complex Types ===== */

/* K&R style function pointer (extra parentheses) */
int ((*old_style_ptr)());

/* Pointer to array of function pointers */
int (*(*pointer_to_func_ptr_array)[5])(void);

/* Const-volatile qualified function pointer */
int (* const volatile cv_fp)(float, ...);

/* ===== 6. Main Function - Use Everything ===== */

int main(void) {
    int result = 0;
    
    /* Use function pointer chain */
    global_super = (super_fn)get_fn_returner;
    if (global_super) {
        fn_returning_fn fr = ((fn_returning_fn (*)(char, double))global_super)('a', 3.14);
        if (fr) {
            simple_fn sf = fr(42);
            if (sf) {
                result += sf();
            }
        }
    }
    
    /* Access nested global struct */
    result += global_nested.items[0].l3.packed.bits[1];
    result += global_nested.items[1].l3.packed.bits[2];
    
    /* Use designated array */
    result += designated_arr[0][1] + designated_arr[1][2];
    
    /* Use compound literal pointer */
    result += ptr_to_array[1][0] + ptr_to_array[2][1];
    
    /* Use union */
    result += global_mix.func_array[0]();
    
    /* Use multi-dimensional array */
    multi_array[0][1][2] = result;
    multi_array[1][0][0] = result * 2;
    result += multi_array[0][1][2] + multi_array[1][0][0];
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}

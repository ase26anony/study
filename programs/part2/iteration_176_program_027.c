/* test-gengtype-coverage.c
 * This file is specifically crafted to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED_MACRO(a) FOO(BAR((a), 2))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
enum { SIZE = 15 };
int var_size[SIZE];
const int const_size = 30;
int var_arr[const_size > 20 ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(16)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. C++-like alignas (C11/C++11) */
_Alignas(16) int aligned_var;

/* 6. Struct with nested union and complex initializer */
struct Outer {
    int a;
    union Inner {
        int x;
        double y;
        char z[10];
    } u;
    struct {
        int nested_a;
        int nested_b[5];
    } s;
};

/* 7. Global instance with nested initializers */
struct Outer global_instance = {
    .a = 1,
    .u = { .y = 3.14 },
    .s = { 
        .nested_a = 42,
        .nested_b = { [0] = 1, [4] = 2 }
    }
};

/* 8. Another struct with designated initializers */
struct Point {
    int x, y, z;
};

struct Line {
    struct Point start;
    struct Point end;
} line = {
    .start = { .x = 0, .y = 0, .z = {0} },
    .end.x = 10,
    .end = { .y = 20, .z = 30 }
};

/* 9. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 10. __typeof__ usage */
__typeof__(*complex_func_ptr) func_type;

/* 11. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Point *point_ptr = &(struct Point){.x = 1, .y = 2, .z = 3};

/* 12. Function with all constructs mixed */
int main(void) {
    /* Trigger parentheses with macro expansion */
    int a = FOO(42);
    int b = BAR(a, NESTED_MACRO(a));
    
    /* Trigger brackets with array access */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += multi_dim[i][0];
        sum += var_arr[i];
    }
    
    /* Trigger braces with compound literals */
    int *dynamic_array = (int[]){10, 20, 30, 40};
    struct Point temp_point = (struct Point){.x = sum, .y = b, .z = a};
    
    /* Trigger parentheses with __builtin_choose_expr */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        (int){100},
        (int){200}
    );
    
    /* Nested switch cases in close proximity */
    int (*local_func)(int) = (int (*)(int)){0};  /* Cast with parentheses */
    int local_array[][3] = {{1, 2, 3}, {4, 5, 6}};  /* Nested braces and brackets */
    
    /* Complex expression mixing all delimiters */
    int result = (a + b) * (sum + choice) + 
                 (dynamic_array[0] + temp_point.x) * 
                 (local_array[0][0] + global_instance.a);
    
    /* Prevent dead code elimination */
    __builtin_printf("Result: %d\n", result);
    
    /* Return statement with ternary operator (parentheses) */
    return GCC_SPECIFIC(result > 0 ? 0 : 1);
}

/* 13. Additional constructs at file scope */
union FinalUnion {
    struct {
        int a;
        int b;
    } s;
    long long ll;
    double d;
} final_union = { .s = { .a = 1, .b = 2 } };

/* 14. Function pointer array with initializer */
int (*func_array[])(int, int) = {
    (int (*)(int, int))BAR,
    NULL,
    (int (*)(int, int))FOO
};

/* 15. __attribute__ with section containing brackets */
int section_var __attribute__((section(".data#special"))) = 99;

/* 16. Static assertion (C11) */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 17. Inline assembly with braces (GCC extension) */
void asm_example(void) {
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        : /* no output */
        : "r"(global_instance.a)
        : "%eax"
    );
}

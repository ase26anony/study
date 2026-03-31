/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal_handler)(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
const int const_arr[5] = {1, 2, 3, 4, 5};
enum { SIZE = 8 };
int enum_sized[SIZE];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(16)));

/* 5. Struct/union definitions with braces */
struct Outer {
    int a;
    struct Inner {
        int x;
        union {
            int i;
            float f;
        } u;
    } inner;
    int b;
};

/* 6. Complex initializers with nested braces */
struct Outer global_outer = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = 4
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. __typeof__ usage */
__typeof__(global_outer.a) type_var;

/* 9. Compound literals */
int *compound_lit = (int[]){1, 2, 3, 4, 5};

/* 10. Variable-length array (C99) */
void use_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = i * i;
    }
}

/* 11. Nested switch cases in main */
int main(void) {
    /* Function pointer usage with parentheses */
    int (*local_func)(int) = NULL;
    
    /* Array with computed size using ternary */
    int computed_size[__builtin_constant_p(1) ? 10 : 20];
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){10, 20, 30, 40, 50})[i];
    }
    
    /* Nested struct initialization */
    struct Outer local_outer = {
        .a = ADD(1, 2),
        .inner = {
            .x = MAX(3, 4),
            .u = { .i = SQUARE(5) }
        },
        .b = 6
    };
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        sizeof(int),
        sizeof(long)
    );
    
    /* Lambda-like function definition (GCC extension) */
    int result = ({
        int temp = local_outer.a + local_outer.b;
        temp * temp;
    });
    
    /* Use VLA */
    use_vla(10);
    
    /* Prevent dead code elimination */
    if (complex_func_ptr) {}
    if (aligned_var) {}
    
    return result + sum + chosen;
}

/* 12. More complex cases after main */
union ComplexUnion {
    struct {
        int a[5];
        char b[10];
    } s;
    long long ll;
    double d;
};

/* 13. Designated initializers with arrays */
int designated_array[10] = {
    [0] = 1,
    [5] = 2,
    [9] = 3
};

/* 14. Nested parentheses in expressions */
int nested_parens = ((((1 + 2) * 3) - 4) / 5);

/* 15. Attribute with array syntax */
int section_var __attribute__((section(".data"))) = 42;

/* 16. __alignof__ usage (more parentheses) */
size_t align_val = __alignof__(struct Outer);

/* 17. Static assertions (C11) */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 18. Inline assembly with braces (GCC) */
void asm_example(void) {
    __asm__ volatile (
        "mov %0, %%eax\n"
        "add $1, %%eax\n"
        : /* outputs */
        : /* inputs */
        : /* clobbers */
    );
}

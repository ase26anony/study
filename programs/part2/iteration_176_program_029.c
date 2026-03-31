/* test_gengtype_coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 * It contains constructs that cause the parser to encounter
 * parentheses '()', brackets '[]', and braces '{}' in various contexts.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) \
    ((a) > (b) ? (a) : ((b) > (c) ? (b) : (c)))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int multi_dim_array[10][20][30];
extern int incomplete_array[];
const int const_array[FOO(5)];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int x;
            int y;
        } point;
        struct {
            int width;
            int height;
        } rect;
        int array[4];
    } data;
    int (*callback)(void);
};

/* Global instance with nested brace initializer */
struct Outer global_instance = {
    .type = 1,
    .data = {
        .point = {
            .x = FOO(10),
            .y = BAR(2, 3)
        }
    },
    .callback = 0
};

/* 6. Another struct with designated initializers */
struct Nested {
    int a;
    struct {
        int b;
        int c[3];
    } inner;
    int d;
} nested_var = {
    .a = 1,
    .inner = {
        .b = 2,
        .c = {3, 4, 5}
    },
    .d = 6
};

/* 7. Preprocessor conditional with complex content */
#ifdef TEST_DEFINE
    #define CONDITIONAL_MACRO(x) ({ \
        typeof(x) _x = (x); \
        _x * _x; \
    })
    
    int conditional_array[__builtin_constant_p(1) ? 10 : 20];
#endif

/* 8. C++ style alignas (C11/C++11) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    _Alignas(16) int aligned_var;
#endif

/* 9. Function using __typeof__ */
void process_value(void *ptr) {
    __typeof__(*(int*)ptr) value = *(int*)ptr;
    value = FOO(value);
}

/* 10. Main function with mixed constructs */
int main(void) {
    /* Compound literal with braces */
    int *dynamic_array = (int[]){1, 2, 3, 4, 5};
    
    /* Array with computed size using ternary */
    int size = 10;
    int variable_array[size > 5 ? size : 5];
    
    /* Nested struct initialization */
    struct Outer local_instance = {
        .type = 2,
        .data = {
            .rect = {
                .width = 100,
                .height = 200
            }
        },
        .callback = 0
    };
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        42,
        24
    );
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += dynamic_array[i];
        }
        sum;
    });
    
    /* Complex expression with nested parentheses */
    int computed = BAR(
        FOO(result),
        COMPLEX_MACRO(
            chosen,
            local_instance.data.rect.width,
            nested_var.inner.c[0]
        )
    );
    
    /* Use all variables to prevent dead code elimination */
    global_instance.data.array[0] = computed;
    variable_array[0] = attr_var + vector_var;
    
    return computed > 0 ? 0 : 1;
}

/* 11. Additional constructs at file scope */
enum Color {
    RED = 1,
    GREEN = 2,
    BLUE = 3
};

/* Function pointer array */
static int (*func_array[])(void) = {
    main,
    0,
    (int (*)(void))process_value
};

/* 12. Union with array and nested initializer */
union MixedData {
    int i;
    float f;
    char str[20];
    struct {
        short s;
        char c;
    } nested;
} mixed = {
    .nested = {
        .s = 255,
        .c = 'A'
    }
};

/* 13. __attribute__ with constructor/destructor */
void __attribute__((constructor)) init_func(void) {
    global_instance.type = 0;
}

void __attribute__((destructor)) cleanup_func(void) {
    /* Empty */
}

/* 14. Asm statement with braces (GCC extension) */
void dummy_asm(void) {
    __asm__ volatile (
        "nop\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "memory"
    );
}

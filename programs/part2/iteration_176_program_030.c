/* test-gengtype-coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
int (*(*more_complex)[5])(void);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[10][20];
int arr3[][5] = {{1,2,3,4,5}, {6,7,8,9,10}};

/* 4. Variable-length array style (using enum) */
enum { SIZE = 20 };
int var_arr[SIZE];
int expr_arr[FOO(5) * 2];

/* 5. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((deprecated));

/* 6. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int a;
            int b[3];
        } s;
        double d;
        void *p;
    } data;
};

/* Global instance with nested brace initializer */
struct Outer global = {
    .type = 1,
    .data = {
        .s = {
            .a = 42,
            .b = {1, 2, {3}}  /* Nested braces */
        }
    }
};

/* 7. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = {.x = 1, .y = 2, .z = 3},
    [1] = {.x = 4, .y = 5, .z = {6}},  /* More nested braces */
    [2] = {7, 8, 9}
};

/* 8. Preprocessor conditional with constructs inside */
#ifdef TEST_DEFINE
    #define CONDITIONAL_MACRO(a) (a * 2)
    int conditional_var[CONDITIONAL_MACRO(5)];
#else
    #define CONDITIONAL_MACRO(a) (a * 3)
#endif

/* 9. __typeof__ usage */
int typeof_var;
__typeof__(typeof_var) typeof_copy;
__typeof__(*complex_func_ptr) typeof_func_result;

/* 10. Compound literal */
int *compound_lit = (int[]){1, 2, 3, 4, 5};

/* 11. Nested structure with anonymous struct */
struct Container {
    struct {
        int inner_a;
        int inner_b;
    };
    int outer_c;
};

/* 12. Union with array */
union DataUnion {
    int i;
    float f;
    char str[20];
};

/* 13. Function declaration with array parameter */
void process_array(int matrix[][10], int rows);

/* 14. Main function containing all triggering constructs */
int main(void) {
    /* Use function-like macro */
    int a = FOO(10);
    int b = BAR(a, 2);
    
    /* Complex declarator usage */
    int (*local_func_ptr)(double) = 0;
    
    /* Array access with brackets */
    arr2[5][10] = 100;
    
    /* GCC builtin with parentheses */
    int choice = __builtin_choose_expr(1, 10, 20);
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){1, 2, 3, 4, 5})[i];
    }
    
    /* Nested struct initialization */
    struct Outer local = {
        .type = 2,
        .data = {
            .d = 3.14159
        }
    };
    
    /* Designated initializer with nested braces */
    struct Point p = {.x = 1, .y = {2}, .z = 3};
    
    /* Array with computed size */
    int dyn_arr[choice];
    
    /* __typeof__ in declaration */
    __typeof__(a) a_copy = a;
    
    /* Prevent dead code elimination */
    if (global.data.s.a + sum + a + b + choice + arr2[5][10] + p.x > 0) {
        return 0;
    }
    
    return 1;
}

/* 15. Additional function with complex signature */
int (*(*register_callback(void (*handler)(int, char *)))[5])(void) {
    static int (*array[5])(void) = {0};
    return &array;
}

/* 16. Enum with last comma (C99) */
enum Colors {
    RED,
    GREEN,
    BLUE,
};

/* 17. Zero-length array at end of struct (GCC extension) */
struct FlexArray {
    int count;
    int data[0];
};

/* 18. Statement expression (GCC extension) */
int stmt_expr = ({
    int temp = 5;
    temp * 2;
});

/* 19. Alignas specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[64];

/* 20. Static assertion */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 21. Nested parentheses in macro arguments */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
int max_val = MAX(FOO(5), BAR(3, 4));

/* 22. Attribute on struct */
struct AttributedStruct {
    int x;
} __attribute__((packed));

/* 23. Inline assembly with braces (GCC extension) */
void asm_example(void) {
    __asm__ volatile (
        "mov %0, %%eax\n"
        : /* no outputs */
        : "r" (42)
        : "%eax"
    );
}

/* 24. Try to trigger bracket parsing in attributes */
int vector __attribute__((vector_size(16 * sizeof(int))));

/* 25. Multiple levels of nesting */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int deep;
        } l3;
    } l2[2];
} l1 = { .l2 = { [0] = { .l3 = { .deep = 99 } } } };

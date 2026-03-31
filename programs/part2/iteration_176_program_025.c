/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_a + _b) * (c); \
})

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int variable_array[FOO(5)][BAR(2, 3)];
extern int incomplete_array[];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int section_var __attribute__((section(".data")));

/* 5. Struct/union definitions with nested braces */
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
struct Outer global_struct = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = {4}
};

/* 7. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
} data = { .str = "Hello" };

/* 8. Array with compound literal initializer */
int *ptr_array = (int[]){1, 2, 3, 4, 5};

/* 9. Nested struct with array */
struct WithArray {
    int values[3][2];
    struct Outer *next;
} with_array = {
    .values = {{1, 2}, {3, 4}, {5, 6}},
    .next = &global_struct
};

/* 10. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
    int gcc_specific __attribute__((deprecated));
#else
    #define GCC_VERSION 0
#endif

/* 11. __typeof__ usage */
__typeof__(*ptr_array) typed_val;

/* 12. C++ style alignas (C11/C23) */
_Alignas(32) char aligned_buffer[256];

/* 13. Function declaration with complex return type */
int (*(*make_array(void))[])(void) {
    static int (*array[5])(void);
    return &array;
}

/* 14. Main function with mixed constructs */
int main(void) {
    /* Compound literal in expression */
    int sum = 0;
    int *dynamic = (int[FOO(3)]){1, 2, 3};
    
    /* Array subscript with expression */
    sum += dynamic[FOO(1) - 1];
    
    /* Nested parentheses in expression */
    sum += BAR((FOO(2)), (3 + 4));
    
    /* GCC builtin with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        FOO(10),
        BAR(20, 30)
    );
    
    /* __typeof__ with parentheses */
    __typeof__(chosen * 2) doubled = chosen * 2;
    
    /* Nested struct access with array subscript */
    sum += with_array.values[1][1];
    
    /* Pointer to array with parentheses */
    int (*array_ptr)[2] = with_array.values;
    sum += (*array_ptr)[0];
    
    /* Lambda-like statement expression (GCC extension) */
    int result = COMPLEX_MACRO(sum, doubled, 2);
    
    /* Struct initialization with nested braces */
    struct Outer local = {
        .a = result,
        .inner = {
            .x = 100,
            .u = { .i = 200 }
        },
        .b = 300
    };
    
    /* Array initialization with nested braces */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Union with nested initializer */
    union Data local_data = { .str = { 't', 'e', 's', 't', '\0' } };
    
    return local.a + matrix[0][0] + local_data.str[0];
}

/* 15. Additional edge cases */
/* Function pointer array */
void (*callbacks[])(void) = { NULL, NULL, NULL };

/* Struct with flexible array member */
struct Flex {
    int count;
    int data[];
};

/* Nested parentheses in macro arguments */
#define NESTED_CALL(f, x) f((x) + 1)
int nested_result = NESTED_CALL(FOO, BAR(2, 3));

/* Attribute on struct */
struct __attribute__((packed)) Packed {
    char a;
    int b;
};

/* 16. More complex array declarations */
int (*complex_array[5])(int, float);
int (*(*more_complex)[10])(void);

/* 17. Initializer with designators and nested braces */
struct NestedInit {
    struct {
        int x;
        int y;
    } point;
    int values[2][2];
} nested_init = {
    .point = { .x = 1, .y = 2 },
    .values = { {1, 2}, {3, 4} }
};

/* 18. Enum with last comma (C99) */
enum Colors {
    RED,
    GREEN,
    BLUE,
};

/* 19. Static assertion (C11) */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 20. Final check - ensure all constructs are used */
volatile int force_usage = 
    global_struct.a +
    data.i +
    ptr_array[0] +
    attr_var +
    vector_var +
    section_var +
    typed_val +
    aligned_buffer[0] +
    gcc_specific +
    nested_result;

/* test-gengtype-balanced.c - Test file for gengtype balanced character parsing */

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
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets (multi-dimensional and variable) */
int multi_dim_array[3][4][5];
enum { ARRAY_SIZE = 10 };
int sized_array[ARRAY_SIZE];
const int const_size = 20;
int var_array[const_size];
int builtin_array[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int section_var __attribute__((section(".data")));

/* 5. Struct/union definitions with nested initializers */
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

/* 6. Complex initializer with nested braces */
struct Outer global_struct = { 
    .a = FOO(1),
    .inner = { 
        .x = 2, 
        .u = { .f = 3.14f } 
    },
    .b = {4}
};

/* 7. Another struct with array member */
struct WithArray {
    int data[5];
    struct Outer *ptr;
};

/* 8. Preprocessor conditional with balanced characters */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) [(x) + 1]
    int conditional_array[CONDITIONAL_MACRO(5)];
#else
    #define CONDITIONAL_MACRO(x) ((x) * 2)
#endif

/* 9. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 10. Function using __typeof__ */
__typeof__(*global_ptr) get_first(void) {
    return global_ptr[0];
}

/* Main function containing various balanced character constructs */
int main(void) {
    /* 11. Local compound literal */
    int *local_ptr = (int[]){10, 20, 30};
    
    /* 12. Nested struct initializer */
    struct WithArray local_struct = {
        .data = {1, 2, 3, 4, 5},
        .ptr = &global_struct
    };
    
    /* 13. GCC built-in with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        FOO(42),
        BAR(10, 20)
    );
    
    /* 14. Complex expression with all balanced characters */
    int result = COMPLEX_MACRO(
        chosen,
        local_ptr[1],
        (int){global_struct.inner.u.i}
    );
    
    /* 15. Array access with complex subscript */
    int array_access = multi_dim_array[1][2][3];
    
    /* 16. Function pointer call */
    if (complex_func_ptr) {
        result = complex_func_ptr(3.14, array_access);
    }
    
    /* 17. Statement expression with nested braces */
    int stmt_expr = ({
        int temp = result;
        for (int i = 0; i < 5; i++) {
            temp += local_struct.data[i];
        }
        temp;
    });
    
    /* 18. __alignof__ with parentheses */
    size_t align_val = __alignof__(struct Outer);
    
    /* 19. Offsetof with nested parentheses */
    size_t offset = __builtin_offsetof(struct Outer, inner.u.f);
    
    /* 20. Prevent dead code elimination */
    return stmt_expr + align_val + offset + get_first();
}

/* 21. Additional test cases outside main */

/* Nested struct definition with bitfield */
struct BitfieldStruct {
    unsigned int a : 5;
    unsigned int b : 3;
    struct {
        unsigned int c : 8;
        unsigned int d : 16;
    } nested;
};

/* Array of function pointers */
int (*func_array[3])(int, int) = {
    [0] = NULL,
    [1] = NULL,
    [2] = NULL
};

/* Typedef with function pointer */
typedef int (*comparator_t)(const void *, const void *);

/* Variable with __attribute__ containing array declarator */
char special_string[32] __attribute__((aligned(32)));

/* Final struct with all balanced characters combined */
struct FinalTest {
    int (*func)(int[][5], struct Outer *);
    struct {
        int array[3][4];
        union {
            char *str;
            void *ptr;
        } u;
    } nested;
} final_instance = {
    .func = NULL,
    .nested = {
        .array = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
        .u = { .str = "test" }
    }
};

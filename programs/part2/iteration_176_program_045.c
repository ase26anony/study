/* test-gengtype-coverage.c - Test file for gengtype balanced character parsing */

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

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int variable_array[FOO(5)][BAR(2, 3)];
enum { SIZE = 15 };
int enum_array[SIZE];
const int const_size = 20;
int const_array[const_size];

/* 4. GCC attributes with brackets and parentheses */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Nested structures with brace initializers */
struct Inner {
    int x;
    double y;
    char z[10];
};

union NestedUnion {
    int i;
    float f;
    struct Inner s;
};

struct Outer {
    int id;
    struct Inner inner;
    union NestedUnion uni;
    int *ptr_array[5];
};

/* Global instance with complex initializer */
struct Outer global_struct = {
    .id = 42,
    .inner = { 
        .x = 1, 
        .y = 3.14, 
        .z = { 'a', 'b', 'c', '\0' } 
    },
    .uni = { .s = { 2, 2.71, { 'd', 'e', 'f' } } },
    .ptr_array = { NULL, NULL, NULL, NULL, NULL }
};

/* 6. Preprocessor conditionals */
#ifdef __GNUC__
#define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
#define GCC_SPECIFIC(x) (x)
#endif

/* 7. Compound literals */
typedef struct {
    int a;
    int b[3];
} CompoundType;

/* 8. __typeof__ usage */
__typeof__(global_struct.inner) inner_copy;

/* Main function containing various constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int dyn_array[GCC_SPECIFIC(1) ? 10 : 20];
    
    /* Compound literal */
    CompoundType *comp = &(CompoundType){ 
        .a = 100, 
        .b = { 1, 2, 3 } 
    };
    
    /* Nested initializer with braces */
    struct Outer local_struct = {
        .id = 99,
        .inner = { 4, 5.0, { 'x', 'y', 'z' } },
        .uni = { .i = 123 },
        .ptr_array = { 
            &dyn_array[0], 
            &comp->b[0], 
            &global_struct.id, 
            NULL, 
            NULL 
        }
    };
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int*) == 8,
        (long)local_func_ptr,
        (int)local_func_ptr
    );
    
    /* Lambda-like expression using statement expression */
    int result = COMPLEX_MACRO(
        global_struct.inner.x,
        local_struct.inner.x,
        chosen % 10
    );
    
    /* Multi-dimensional array access with brackets */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            multi_dim[i][j] = i * j + result;
        }
    }
    
    /* Complex array declarator in local scope */
    int (*(*local_complex)[5])(void) = (int (*(*)[5])(void))&complex_array;
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = result;
    
    /* Alignas specifier (C11/C++11) */
    _Alignas(32) char aligned_buffer[64];
    
    return result > 0 ? 0 : 1;
}

/* Additional C++-like constructs (commented for C, uncomment for C++) */
/*
// Template-like macro
#define TEMPLATE_TYPE(T) struct Template_##T { T value; }

// Namespace-like prefix
typedef struct {
    int (*vtable[3])(void*);
} Interface;

// Complex expression with nested parentheses
int very_nested = ((((1 + 2) * (3 - 4)) / ((5 % 6) + 7)) - (8 * (9 / 10)));
*/

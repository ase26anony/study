/* test-gengtype-coverage.c
 * This file is specifically crafted to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[3][4][5];
enum { SIZE = 10 };
int var_size[SIZE];
int runtime_size[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_type __attribute__((vector_size(16)));

/* 5. C++-like alignas (C11/C++11) */
#ifdef __cplusplus
alignas(32) double aligned_double;
#else
_Alignas(32) double aligned_double;
#endif

/* 6. Struct/union definitions with nested initializers */
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

/* Global instance with complex initializer */
struct Outer global_var = { 
    .a = 1, 
    .inner = { 
        .x = 2, 
        .u = { .f = 3.14f } 
    }, 
    .b = 4 
};

/* 7. Another struct with array member */
struct WithArray {
    int data[5];
    struct Outer *ptr;
};

/* 8. Preprocessor conditional with balanced characters */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) ({ \
        typeof(x) _x = (x); \
        _x * _x; \
    })
#else
    #define CONDITIONAL_MACRO(x) ((x) + 1)
#endif

/* 9. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 10. Function using all constructs */
int main(void) {
    /* Use function-like macro */
    int x = ADD(5, 3);
    int y = MAX(x, 10);
    
    /* Use complex function pointer type */
    int (*local_func)(double) = (int (*)(double))0;
    
    /* Multi-dimensional array access */
    multi_dim[0][1][2] = x;
    
    /* Array with computed size */
    var_size[SIZE - 1] = y;
    
    /* Compound literal */
    struct WithArray *wa = &(struct WithArray){
        .data = {1, 2, 3, 4, 5},
        .ptr = &global_var
    };
    
    /* Nested struct access with multiple brackets/parentheses */
    wa->ptr->inner.u.i = 42;
    
    /* __typeof__ with parentheses */
    __typeof__(*global_ptr) val = global_ptr[0];
    
    /* GCC builtin with nested parentheses */
    int choice = __builtin_choose_expr(
        __builtin_constant_p(x),
        sizeof(int[10]),
        sizeof(int[20])
    );
    
    /* Lambda-like expression (GCC extension) */
    int (*lambda)(int) = ({
        int __fn(int n) { return n * n; }
        __fn;
    });
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 100;
    
    /* Designated initializer with nested braces */
    int matrix[2][2] = { [0][0] = 1, [0][1] = 2, [1][0] = 3, [1][1] = 4 };
    
    /* Return statement with expression containing all bracket types */
    return (ADD(
        wa->data[0],
        (matrix[0][1] + global_var.inner.x)
    ));
}

/* 11. Additional constructs at file scope to ensure parsing */
/* Function definition with attributes */
__attribute__((noinline))
static void helper_func(int arg[static 5]) {
    /* Empty function body still contains braces */
}

/* Array of function pointers */
void (*func_array[])(void) = { helper_func, 0 };

/* Struct containing array of structs containing arrays */
struct Recursive {
    struct Recursive *next;
    int values[3];
    struct {
        char data[10];
    } embedded;
} recursive_var = {
    .next = &recursive_var,
    .values = {1, 2, 3},
    .embedded.data = "test"
};

/* 12. C++ specific constructs (if compiled as C++) */
#ifdef __cplusplus
template<typename T>
class SimpleTemplate {
    T value;
public:
    SimpleTemplate(T v) : value(v) {}
    T get() { return value; }
};

SimpleTemplate<int> template_instance(42);

namespace TestNamespace {
    int namespace_var = 100;
    
    auto lambda = []() -> int {
        return namespace_var * 2;
    };
}
#endif

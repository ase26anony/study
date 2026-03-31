/* Complex type declarations to exercise consume_balanced parsing */
#include "complex_types.h"
#include <stdio.h>

/* Macro to generate delimiter-heavy code */
#define MAKE_COMPLEX_ARRAY(n) int (*(*array_ptr##n)[n])(char (*)[n])
#define NESTED_ATTRIBUTE __attribute__((aligned(32))) __attribute__((format(printf, 2, 3)))

/* Function with nested parentheses in attributes */
void debug_print(int level, const char *fmt, ...) NESTED_ATTRIBUTE;

/* Complex function pointer type with nested parameter list */
int (*(*global_callback)(int (*)(float, double[2]), 
                         struct Outer*))(void) = NULL;

/* Multi-dimensional array with complex initializer */
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
};

/* Array of function pointers */
typedef int (*func_array[5])(char, ...);
func_array my_funcs;

/* Even more complex: pointer to array of function pointers returning pointers to arrays */
int (*(*(*ultimate_type)[3])(int))[5];

/* Nested structure with anonymous unions and bitfields */
struct Container {
    union {
        struct {
            int flags : 5;
            int mode : 3;
            int : 4;  /* unnamed bitfield */
        };
        unsigned int raw;
    };
    
    /* Function pointer array member */
    int (*(*func_ptrs[2])(void))[3];
    
    /* Nested structure with flexible array member */
    struct {
        int count;
        double values[];
    } dynamic;
};

/* Compound type in sizeof context */
size_t complex_size = sizeof(int (*(*)[10])(char (*)[20]));

/* Type definition with deeply nested parentheses */
typedef int (*(*array_of_func_ptrs[5])(int (*)(float), ...))[10];
array_of_func_ptrs complex_var;

/* GCC statement expression with nested delimiters */
#define COMPOUND_ASSIGN(dest, src) ({ \
    typeof(dest) _tmp = (src); \
    (dest) = _tmp; \
    _tmp; \
})

int main(void) {
    /* Use various complex types to ensure they're parsed */
    MAKE_COMPLEX_ARRAY(5) local_var = NULL;
    
    /* sizeof with nested array types */
    printf("Sizes:\n");
    printf("  matrix: %zu\n", sizeof(matrix));
    printf("  complex_size: %zu\n", complex_size);
    printf("  struct Container: %zu\n", sizeof(struct Container));
    printf("  ultimate_type: %zu\n", sizeof(ultimate_type));
    
    /* Compound literal with nested braces */
    int (*arr_ptr)[4] = &(int[3][4]){{0}};
    
    /* Cast expression with nested parentheses */
    int value = (int)((long)(*(int (*)[3])matrix)[0]);
    
    /* Nested attribute in declaration */
    int __attribute__((aligned(64))) aligned_array[10] 
        __attribute__((unused));
    
    /* Use macro-generated type */
    array_ptr5 ptr5 = NULL;
    
    return 0;
}

/* Function definitions */
void debug_print(int level, const char *fmt, ...) {
    /* Implementation not needed for parsing coverage */
    (void)level;
    (void)fmt;
}

/* complex-types.h - Test file for gengtype's consume_balanced function */
/* This header contains deeply nested parentheses, brackets, and braces */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Level 1: Basic nested parentheses in function pointers */
typedef int (*simple_fp)(int, int);
typedef void (*(*signal_fp)(int, void (*)(int)))(int);

/* Level 2: Function pointers with array parameters */
typedef int (*array_param_fp)(int arr[10], int (*callback)[5]);

/* Level 3: Deeply nested parentheses - 4 levels */
typedef int (*(*(*deep_nested_fp)(void))(int))(char);

/* Level 4: Function returning pointer to array of function pointers */
typedef int (*(*func_ret_array_fp)(void))[10];

/* Level 5: Array of function pointers returning function pointers */
typedef int (*(*complex_array[5])(int))(double);

/* Struct with nested braces in bitfields and arrays */
struct NestedStruct {
    /* Bitfield with complex expression */
    unsigned int flags : (sizeof(int) * 8 - 1);
    
    /* Array with size from expression containing parentheses */
    int dynamic_array[(sizeof(struct NestedStruct) > 16) ? 8 : 16];
    
    /* Function pointer member */
    void (*handler)(struct NestedStruct *self, int (*(*arg)(void))[10]);
    
    /* Nested anonymous struct with initializer-like design */
    struct {
        int x;
        union {
            int i;
            void *p;
        } data[2];
    } inner;
};

/* Union with complex nested types */
union ComplexUnion {
    /* Function pointer variant */
    int (*(*fp_variant)(int, int (*)(char)))[5];
    
    /* Struct variant with nested array */
    struct {
        int matrix[3][(sizeof(int) + 2)];
        void (*operations[2])(int, int);
    } struct_variant;
    
    /* Array variant with VLAs in type expression */
    int (*array_variant)[(sizeof(union ComplexUnion) / 4)];
};

/* Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*complex_type_##n)(int (*(*inner_##n)[n])(void)))[n * 2]

DECLARE_COMPLEX_TYPE(1);
DECLARE_COMPLEX_TYPE(2);
DECLARE_COMPLEX_TYPE(3);

/* Template-like macro for function pointers */
#define CALLBACK_TYPE(ret, name) \
    typedef ret (*name##_cb)(int, ret (*)(void), name##_cb*)

CALLBACK_TYPE(int, IntCallback);
CALLBACK_TYPE(void*, PtrCallback);

/* Extern declarations to force cross-file processing */
extern struct NestedStruct global_nested;
extern union ComplexUnion *global_union_array[];

/* Function prototypes with complex parameter types */
extern void process_complex(int (*(*param1)(void))[5],
                           struct NestedStruct (*param2)[(10 + 2)],
                           void (*param3)(int, ...));

/* Variable declaration with nested brackets */
extern int (*(*global_table)[(sizeof(int) * 8)])[10];

#endif /* COMPLEX_TYPES_H */

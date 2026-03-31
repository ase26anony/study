/* 
 * Comprehensive type declaration test for gengtype.cc coverage
 * Targeting TYPE_* enumeration classification in GCC's type system
 */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED array */
struct forward_declared_struct;         /* Forward declaration */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool scalar_bool = 0;
__attribute__((unused)) int scalar_int = 42;
__attribute__((unused)) float scalar_float = 3.14f;
__attribute__((unused)) volatile const int volatile_const_scalar = 100;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";
__attribute__((unused)) volatile const char* volatile_const_string = "VolatileConst";

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    float field2;
    const char* field3;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    long data;
    void* ptr;
} user_struct_t;

/* ========== TYPE_UNION with complex nesting ========== */
union __attribute__((annotate("gengtype"))) complex_union {
    int as_int;
    float as_float;
    struct {
        char byte1;
        char byte2;
    } as_bytes;
    void* as_pointer;
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_pointer = &scalar_int;
__attribute__((unused)) const float* const const_float_pointer = &scalar_float;
__attribute__((unused)) volatile const int* volatile volatile_const_int_pointer;
__attribute__((unused)) void* void_pointer = NULL;
__attribute__((unused)) struct annotated_struct* struct_pointer = NULL;
__attribute__((unused)) union complex_union* union_pointer = NULL;

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) const char* string_array[] = {"one", "two", "three"};
__attribute__((unused)) volatile int volatile_array[5];
__attribute__((unused)) struct annotated_struct struct_array[3];
__attribute__((unused)) int multi_dim_array[2][3][4];

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union complex_union);

__attribute__((unused)) __attribute__((annotate("gengtype"))) 
int (*func_pointer)(int, char**) = NULL;

__attribute__((unused)) simple_callback_t callback_var = NULL;
__attribute__((unused)) volatile complex_callback_t volatile_callback = NULL;

/* ========== Nested type constructs ========== */
struct __attribute__((annotate("gengtype"))) mega_nested_struct {
    /* Contains multiple type classifications */
    int scalar_field;                          /* TYPE_SCALAR */
    char* string_field;                        /* TYPE_STRING */
    struct annotated_struct nested_struct;     /* TYPE_STRUCT */
    union complex_union nested_union;          /* TYPE_UNION */
    int* pointer_field;                        /* TYPE_POINTER */
    int array_field[5];                        /* TYPE_ARRAY */
    simple_callback_t callback_field;          /* TYPE_CALLBACK */
    
    /* Pointer to incomplete type */
    struct forward_declared_struct* incomplete_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Array of function pointers */
__attribute__((unused)) simple_callback_t callback_array[3];

/* Union containing pointer to array of structs */
union pointer_container {
    struct mega_nested_struct* (*get_struct_array)(void);
    struct annotated_struct** struct_ptr_ptr;
};

/* ========== Function using __builtin_types_compatible_p ========== */
static void perform_type_comparisons(void) {
    /* Trigger type classification through compatibility checks */
    int check1 = __builtin_types_compatible_p(int, float);  /* scalar vs scalar */
    int check2 = __builtin_types_compatible_p(int*, float*); /* pointer vs pointer */
    int check3 = __builtin_types_compatible_p(struct annotated_struct*, 
                                             union complex_union*); /* struct vs union pointer */
    int check4 = __builtin_types_compatible_p(int[10], int*); /* array vs pointer */
    int check5 = __builtin_types_compatible_p(simple_callback_t, 
                                             void*); /* callback vs pointer */
    int check6 = __builtin_types_compatible_p(user_struct_t, 
                                             struct annotated_struct); /* user struct vs struct */
    
    /* Use volatile to prevent optimization */
    volatile int dummy __attribute__((unused)) = 
        check1 + check2 + check3 + check4 + check5 + check6;
}

/* ========== Function using incomplete types ========== */
static size_t use_incomplete_types(void) {
    /* sizeof with incomplete type (pointer to incomplete is complete) */
    size_t s1 = sizeof(struct undefined_extern_struct*);
    size_t s2 = sizeof(int (*)(struct forward_declared_struct*));
    
    /* Array of incomplete type (as parameter) */
    extern void process_array(int undefined_extern_array[]);
    
    return s1 + s2;
}

/* ========== Callback function implementations ========== */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void complex_callback_impl(struct annotated_struct* s, union complex_union u) {
    if (s) s->field1 = u.as_int;
}

/* ========== Main function ========== */
int main(void) {
    /* Variable declarations with various qualifiers */
    const volatile int* const restrict qualified_pointer __attribute__((unused)) = &scalar_int;
    struct mega_nested_struct nested_var __attribute__((unused));
    union pointer_container container __attribute__((unused));
    
    /* Initialize function pointers */
    callback_var = sample_callback;
    volatile_callback = complex_callback_impl;
    
    /* Use the types to ensure they're processed */
    nested_var.scalar_field = 42;
    nested_var.string_field = (char*)string_literal;
    nested_var.callback_field = sample_callback;
    
    /* Call through function pointer */
    if (callback_var) {
        int result = callback_var(10, 20.5f);
        nested_var.scalar_field = result;
    }
    
    /* Use arrays */
    for (int i = 0; i < 5; i++) {
        nested_var.array_field[i] = i * 2;
        volatile_array[i] = i;
    }
    
    /* Perform type comparisons (triggers internal classification) */
    perform_type_comparisons();
    
    /* Use incomplete types */
    size_t incomplete_size = use_incomplete_types();
    nested_var.scalar_field += (int)incomplete_size;
    
    /* Complex nested access */
    struct annotated_struct* sp = &nested_var.nested_struct;
    sp->field1 = 100;
    sp->field3 = "Nested";
    
    /* Union type punning */
    nested_var.nested_union.as_int = 0xDEADBEEF;
    float from_union = nested_var.nested_union.as_float;
    
    /* Prevent dead code elimination */
    volatile float union_result __attribute__((unused)) = from_union;
    
    return 0;
}

/* ========== Additional incomplete type definitions ========== */
struct forward_declared_struct {
    int finally_defined;
    struct mega_nested_struct* link;
};

/* External array definition */
int undefined_extern_array[5] = {1, 2, 3, 4, 5};

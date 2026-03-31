/* 
 * Complex type declarations to exercise GCC's gengtype type classification
 * Targeting uncovered switch cases in gengtype.cc lines 182-213
 */

#include <stddef.h>

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern union undefined_union;             /* TYPE_UNDEFINED */
extern int incomplete_array[];            /* TYPE_UNDEFINED for array size */

/* TYPE_SCALAR: Basic scalar types */
__attribute__((unused)) _Bool bool_scalar;                    /* TYPE_SCALAR */
__attribute__((unused)) int int_scalar;                       /* TYPE_SCALAR */
__attribute__((unused)) float float_scalar;                   /* TYPE_SCALAR */
__attribute__((unused)) volatile const long volatile_const_scalar; /* TYPE_SCALAR with qualifiers */

/* TYPE_STRING: String literals and char pointers */
__attribute__((unused)) char* string_literal = "test_string"; /* TYPE_STRING */
__attribute__((unused)) const char* const const_string = "const_test"; /* TYPE_STRING with qualifiers */

/* TYPE_STRUCT: Named struct types */
__attribute__((annotate("gengtype")))
struct named_struct {                     /* TYPE_STRUCT */
    int field1;
    float field2;
    volatile char field3;
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {                          /* TYPE_USER_STRUCT */
    double data;
    void* ptr;
} __attribute__((annotate("gengtype"))) user_struct_t;

/* TYPE_UNION: Named union types */
__attribute__((annotate("gengtype")))
union named_union {                       /* TYPE_UNION */
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_POINTER: Pointers to various types */
__attribute__((unused)) int* int_ptr;                      /* TYPE_POINTER */
__attribute__((unused)) volatile const int* const volatile_const_ptr; /* TYPE_POINTER with qualifiers */
__attribute__((unused)) struct named_struct* struct_ptr;   /* TYPE_POINTER to TYPE_STRUCT */
__attribute__((unused)) user_struct_t* user_struct_ptr;    /* TYPE_POINTER to TYPE_USER_STRUCT */
__attribute__((unused)) union named_union* union_ptr;      /* TYPE_POINTER to TYPE_UNION */

/* TYPE_ARRAY: Various array types */
__attribute__((unused)) int fixed_array[10];               /* TYPE_ARRAY fixed size */
__attribute__((unused)) float volatile_array[5];           /* TYPE_ARRAY with volatile */
__attribute__((unused)) struct named_struct struct_array[3]; /* TYPE_ARRAY of TYPE_STRUCT */
__attribute__((unused)) user_struct_t user_struct_array[2]; /* TYPE_ARRAY of TYPE_USER_STRUCT */

/* TYPE_CALLBACK: Function pointers */
typedef int (*simple_callback)(int, float);                /* TYPE_CALLBACK */
__attribute__((annotate("gengtype")))
typedef void (*complex_callback)(struct named_struct*, user_struct_t*); /* TYPE_CALLBACK */

/* Complex nested type: struct containing array of function pointers */
__attribute__((annotate("gengtype")))
struct container_struct {                 /* TYPE_STRUCT */
    int id;
    /* Nested union */
    union {
        int int_data;
        float float_data;
        struct named_struct* nested_struct_ptr; /* TYPE_POINTER in TYPE_UNION */
    } data_union;                          /* TYPE_UNION inside TYPE_STRUCT */
    
    /* Array of function pointers */
    simple_callback callbacks[5];          /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Pointer to complex callback */
    complex_callback* callback_ptr;        /* TYPE_POINTER to TYPE_CALLBACK */
};

/* TYPE_LANG_STRUCT: Use GCC extension for language-specific struct */
#ifdef __GNUC__
struct __attribute__((transaction_safe)) transaction_struct { /* May become TYPE_LANG_STRUCT */
    int value;
    void* data;
};
#endif

/* Function using the complex callback type */
static int sample_function(int a, float b) __attribute__((unused));
static int sample_function(int a, float b) {
    return a + (int)b;
}

/* Another function with complex signature */
static void process_structures(struct named_struct* ns, user_struct_t* us) 
    __attribute__((unused));
static void process_structures(struct named_struct* ns, user_struct_t* us) {
    if (ns && us) {
        ns->field1 = (int)us->data;
    }
}

/* Variable Length Array (VLA) - creates special array type */
void use_vla(int size) __attribute__((unused));
void use_vla(int size) {
    int vla[size];                         /* TYPE_ARRAY (VLA) */
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}

/* Main function with type comparisons and usage */
int main(void) {
    /* Variable declarations using all the complex types */
    struct container_struct container __attribute__((unused));
    user_struct_t user_struct __attribute__((unused));
    union named_union nu __attribute__((unused));
    
    /* Initialize some values to prevent dead code elimination */
    int_scalar = 42;
    float_scalar = 3.14f;
    container.id = 1;
    user_struct.data = 2.71828;
    
    /* Use __builtin_types_compatible_p for type comparisons */
    int type_checks __attribute__((unused)) = 0;
    
    /* Compare scalar vs pointer */
    type_checks = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    type_checks += __builtin_types_compatible_p(struct named_struct, union named_union);
    
    /* Compare pointer types */
    type_checks += __builtin_types_compatible_p(int*, float*);
    
    /* Compare array types */
    type_checks += __builtin_types_compatible_p(int[10], float[10]);
    
    /* Compare function pointer types */
    type_checks += __builtin_types_compatible_p(simple_callback, complex_callback);
    
    /* Use incomplete types in sizeof (allowed in some contexts) */
    size_t incomplete_size __attribute__((unused)) = 
        sizeof(struct undefined_struct*);  /* Pointer to TYPE_UNDEFINED */
    
    /* Use function pointers */
    container.callbacks[0] = sample_function;
    if (container.callbacks[0]) {
        int result __attribute__((unused)) = container.callbacks[0](5, 2.5f);
    }
    
    /* Complex nested type usage */
    container.data_union.nested_struct_ptr = &container;
    
    /* Use the complex callback */
    complex_callback cb __attribute__((unused)) = process_structures;
    
    /* Trigger VLA usage */
    use_vla(10);
    
    /* Return without using potentially uninitialized variables */
    return 0;
}

/* Additional external declarations for incomplete types */
extern struct external_undefined_struct;   /* TYPE_UNDEFINED */
extern int external_incomplete_array[];    /* TYPE_UNDEFINED */

/* Multi-dimensional array for additional TYPE_ARRAY coverage */
__attribute__((unused)) int multi_dim_array[3][4][5]; /* TYPE_ARRAY of TYPE_ARRAY */

/* Const pointer to volatile array */
__attribute__((unused)) volatile int (*volatile_array_ptr)[10]; /* TYPE_POINTER to TYPE_ARRAY */

/* Typedef for pointer to function returning pointer to struct */
typedef struct named_struct* (*struct_factory)(void); /* TYPE_CALLBACK */

/* Anonymous struct/union in typedef (TYPE_USER_STRUCT) */
typedef struct {                          /* TYPE_USER_STRUCT */
    union {                               /* TYPE_UNION inside TYPE_USER_STRUCT */
        int x;
        long y;
    } data;
    const char* name;
} anonymous_wrapper_t;

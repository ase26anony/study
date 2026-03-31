/* gengtype_coverage.c - Program to exercise GCC's gengtype type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_extern;                     /* TYPE_UNDEFINED */
extern int undefined_array_extern[];                /* TYPE_UNDEFINED array */
struct forward_declared;                            /* Forward declaration */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused, annotate("gengtype"))) 
volatile const _Bool scalar_bool = 1;               /* TYPE_SCALAR */

__attribute__((unused)) 
volatile int scalar_int = 42;                       /* TYPE_SCALAR */

__attribute__((unused)) 
const float scalar_float = 3.14f;                   /* TYPE_SCALAR */

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused, annotate("gengtype"))) 
char* string_literal = "Hello, gengtype!";          /* TYPE_STRING */

__attribute__((unused)) 
const char* const const_string = "Constant string"; /* TYPE_STRING with qualifiers */

/* ========== TYPE_STRUCT definitions ========== */
__attribute__((unused, annotate("gengtype"))) 
struct named_struct {                               /* TYPE_STRUCT */
    int x;
    float y;
    char* str;
};

/* Nested struct with complex members */
__attribute__((annotate("gengtype"))) 
struct container_struct {                           /* TYPE_STRUCT */
    struct named_struct inner;
    volatile const long counter;
};

/* ========== TYPE_USER_STRUCT via typedef ========== */
typedef struct named_struct user_struct_t;          /* TYPE_USER_STRUCT */

__attribute__((unused, annotate("gengtype"))) 
typedef struct {                                    /* TYPE_USER_STRUCT (anonymous) */
    int id;
    char name[32];
} user_anon_struct_t;

/* ========== TYPE_UNION definitions ========== */
__attribute__((unused, annotate("gengtype"))) 
union data_union {                                  /* TYPE_UNION */
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* Union within struct */
struct union_container {                            /* TYPE_STRUCT containing TYPE_UNION */
    int tag;
    union {
        int num;
        char* str;
    } value;                                        /* Nested anonymous union */
};

/* ========== TYPE_POINTER declarations ========== */
__attribute__((unused, annotate("gengtype"))) 
volatile const int* const volatile_ptr = &scalar_int; /* TYPE_POINTER with qualifiers */

__attribute__((unused)) 
struct named_struct* struct_ptr = 0;                /* TYPE_POINTER to TYPE_STRUCT */

__attribute__((unused)) 
union data_union* union_ptr = 0;                    /* TYPE_POINTER to TYPE_UNION */

__attribute__((unused)) 
void (*func_ptr)(void) = 0;                         /* TYPE_POINTER (to function) */

/* Triple pointer for complexity */
__attribute__((unused)) 
int*** triple_ptr = 0;                              /* TYPE_POINTER to TYPE_POINTER to TYPE_POINTER */

/* ========== TYPE_ARRAY declarations ========== */
__attribute__((unused, annotate("gengtype"))) 
int fixed_array[10];                                /* TYPE_ARRAY fixed size */

__attribute__((unused)) 
float multi_dim_array[5][3];                        /* TYPE_ARRAY multi-dimensional */

/* Array of pointers */
__attribute__((unused)) 
char* string_array[] = {"one", "two", "three"};     /* TYPE_ARRAY of TYPE_STRING */

/* Variable-length array (C99) in function */
void use_vla(int size) {
    __attribute__((unused)) 
    int vla[size];                                  /* TYPE_ARRAY variable length */
    (void)vla;
}

/* ========== TYPE_CALLBACK function pointers ========== */
/* Complex callback signature */
typedef int (*complex_callback_t)(                 /* TYPE_USER_STRUCT -> TYPE_CALLBACK */
    struct named_struct*,
    union data_union*,
    void (*)(int)
) __attribute__((annotate("gengtype")));

/* Function pointer with attributes */
__attribute__((unused, annotate("gengtype"))) 
void (*callback_with_attrs)(int) 
    __attribute__((noreturn));

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be triggered by these */
struct __attribute__((annotate("gengtype"))) lang_like_struct {
    int gcc_internal_field;
};

/* ========== Complex nested type ========== */
/* Struct containing array of function pointers */
__attribute__((annotate("gengtype"))) 
struct nested_complex {
    int (*callbacks[5])(void);                      /* TYPE_ARRAY of TYPE_CALLBACK */
    union data_union* union_array[3];               /* TYPE_ARRAY of TYPE_POINTER to TYPE_UNION */
    struct container_struct** double_indirect;      /* TYPE_POINTER to TYPE_POINTER to TYPE_STRUCT */
};

/* ========== Type comparison expressions ========== */
/* These may trigger __builtin_types_compatible_p internal processing */
static int type_comparisons(void) {
    int result = 0;
    
    /* Compare scalar vs pointer */
    result += __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    result += __builtin_types_compatible_p(struct named_struct, union data_union);
    
    /* Compare pointer types with different qualifiers */
    result += __builtin_types_compatible_p(const int*, volatile int*);
    
    /* Compare array vs pointer (decay) */
    result += __builtin_types_compatible_p(int[10], int*);
    
    /* Compare function pointer signatures */
    typedef void (*func1_t)(int);
    typedef int (*func2_t)(int);
    result += __builtin_types_compatible_p(func1_t, func2_t);
    
    return result;
}

/* ========== Function using many types ========== */
static void use_types(void) {
    /* Declare variables of various types */
    user_struct_t user_var;
    user_anon_struct_t anon_var;
    struct nested_complex complex_var;
    complex_callback_t callback_var = 0;
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t s1 = sizeof(struct forward_declared*);   /* Pointer to incomplete type */
    size_t s2 = sizeof(undefined_array_extern);     /* Incomplete array type */
    
    /* Initialize to avoid unused warnings */
    (void)user_var;
    (void)anon_var;
    (void)complex_var;
    (void)callback_var;
    (void)s1;
    (void)s2;
    
    /* Use variable-length array */
    use_vla(5);
}

/* ========== Main function ========== */
int main(void) {
    /* Force processing of all types */
    use_types();
    
    /* Trigger type comparisons */
    __attribute__((unused)) 
    int comp_result = type_comparisons();
    
    /* Use some pointers to prevent dead code elimination */
    if (struct_ptr) {
        return 1;
    }
    
    if (union_ptr) {
        return 2;
    }
    
    /* Call via function pointer if non-null */
    if (func_ptr) {
        func_ptr();
    }
    
    /* Use arrays */
    fixed_array[0] = scalar_int;
    multi_dim_array[0][0] = scalar_float;
    
    return 0;
}

/* Function definitions for callbacks */
__attribute__((noreturn)) 
void callback_with_attrs(int x) {
    (void)x;
    while(1); /* Actually noreturn */
}

/* Complete the forward declaration */
struct forward_declared {
    int completed;
};

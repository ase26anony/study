/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classifier */

/* ========== TYPE_UNDEFINED and TYPE_LANG_STRUCT ========== */
/* Forward declarations creating incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED candidate */
extern union undefined_union;             /* TYPE_UNDEFINED candidate */
struct forward_declared_struct;           /* Forward declaration */
union forward_declared_union;             /* Forward declaration */

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types */
__attribute__((unused)) _Bool bool_scalar = 0;                    /* TYPE_SCALAR */
__attribute__((unused)) int int_scalar = 42;                      /* TYPE_SCALAR */
__attribute__((unused)) float float_scalar = 3.14f;               /* TYPE_SCALAR */
__attribute__((unused)) double double_scalar = 2.71828;           /* TYPE_SCALAR */
__attribute__((unused)) char char_scalar = 'A';                   /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
/* String literals and character arrays */
__attribute__((unused)) const char* string_literal = "Hello";     /* TYPE_STRING */
__attribute__((unused)) char string_array[] = "World";            /* TYPE_ARRAY of TYPE_SCALAR */

/* ========== TYPE_STRUCT with annotations ========== */
/* Annotated struct to force metadata generation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;  /* TYPE_POINTER to TYPE_SCALAR */
};

/* Nested struct with complex members */
struct complex_struct {
    struct annotated_struct base;          /* TYPE_STRUCT */
    union {
        int as_int;
        float as_float;
    } variant;                            /* TYPE_UNION inside struct */
    void (*callback)(int, float);         /* TYPE_CALLBACK field */
};

/* ========== TYPE_UNION with annotations ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int ival;
    float fval;
    double dval;
    char* str;                            /* TYPE_POINTER */
    struct complex_struct cs;             /* TYPE_STRUCT inside union */
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
/* Typedef creates TYPE_USER_STRUCT */
typedef struct {
    int id;
    char name[32];                        /* TYPE_ARRAY */
    struct complex_struct* next;          /* TYPE_POINTER */
} user_struct_t;

/* Another typedef with function pointer */
typedef int (*comparator_t)(const void*, const void*);  /* TYPE_CALLBACK typedef */

/* ========== TYPE_POINTER variations ========== */
/* Various pointer types with qualifiers */
__attribute__((unused)) volatile const int* const volatile_ptr = &int_scalar;  /* TYPE_POINTER */
__attribute__((unused)) struct complex_struct* struct_ptr = 0;                 /* TYPE_POINTER */
__attribute__((unused)) union annotated_union* union_ptr = 0;                  /* TYPE_POINTER */
__attribute__((unused)) user_struct_t* user_struct_ptr = 0;                    /* TYPE_POINTER */
__attribute__((unused)) char** double_ptr = &string_literal;                   /* TYPE_POINTER to TYPE_POINTER */

/* Function pointer with annotation */
int (* __attribute__((annotate("gengtype"))) annotated_func_ptr)(int, char*) = 0;  /* TYPE_CALLBACK */

/* ========== TYPE_ARRAY variations ========== */
/* Fixed-size arrays */
__attribute__((unused)) int int_array[10];                                   /* TYPE_ARRAY */
__attribute__((unused)) struct complex_struct struct_array[5];               /* TYPE_ARRAY of TYPE_STRUCT */
__attribute__((unused)) union annotated_union union_array[3];                /* TYPE_ARRAY of TYPE_UNION */

/* Multi-dimensional array */
__attribute__((unused)) float matrix[3][3];                                  /* TYPE_ARRAY of TYPE_ARRAY */

/* Array of function pointers */
int (*func_ptr_array[5])(void);                                             /* TYPE_ARRAY of TYPE_CALLBACK */

/* ========== TYPE_CALLBACK variations ========== */
/* Function pointer types */
__attribute__((unused)) void (*simple_callback)(void);                      /* TYPE_CALLBACK */
__attribute__((unused)) char* (*string_processor)(const char*, int);        /* TYPE_CALLBACK */

/* Complex callback signature */
typedef struct {
    int result;
    char error_msg[64];
} result_t;

result_t (*complex_callback)(int, float, void*);                           /* TYPE_CALLBACK */

/* ========== Nested Complex Type ========== */
/* Struct containing array of function pointers */
struct container {
    int id;
    union annotated_union data;                     /* TYPE_UNION */
    comparator_t comparators[4];                    /* TYPE_ARRAY of TYPE_CALLBACK */
    struct container* next;                         /* TYPE_POINTER */
    volatile const char* const volatile_name;       /* TYPE_POINTER with qualifiers */
};

/* ========== Type comparison expressions ========== */
/* Use __builtin_types_compatible_p to force type analysis */
#define CHECK_TYPE_COMPAT(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* ========== Main function ========== */
int main(void) {
    /* Variable declarations using our complex types */
    struct complex_struct cs_instance = {0};
    union annotated_union au_instance;
    user_struct_t us_instance = {0};
    struct container container_instance = {0};
    
    /* Initialize some values to avoid dead code elimination */
    cs_instance.base.x = 1;
    cs_instance.base.y = 2.0f;
    cs_instance.base.name = "test";
    cs_instance.variant.as_int = 42;
    
    au_instance.ival = 100;
    
    us_instance.id = 1;
    
    container_instance.id = 99;
    container_instance.data.ival = 200;
    
    /* Use sizeof with various types (including incomplete) */
    __attribute__((unused)) size_t sizes[] = {
        sizeof(_Bool),                      /* TYPE_SCALAR */
        sizeof(int),                        /* TYPE_SCALAR */
        sizeof(struct complex_struct),      /* TYPE_STRUCT */
        sizeof(union annotated_union),      /* TYPE_UNION */
        sizeof(user_struct_t),              /* TYPE_USER_STRUCT */
        sizeof(int*),                       /* TYPE_POINTER */
        sizeof(int[10]),                    /* TYPE_ARRAY */
        sizeof(struct forward_declared_struct*), /* TYPE_POINTER to TYPE_UNDEFINED */
    };
    
    /* Type compatibility checks to trigger type analysis */
    __attribute__((unused)) int type_checks[] = {
        CHECK_TYPE_COMPAT(typeof(int), typeof(float)),          /* Scalar vs scalar */
        CHECK_TYPE_COMPAT(typeof(int*), typeof(float*)),        /* Pointer vs pointer */
        CHECK_TYPE_COMPAT(typeof(struct complex_struct), 
                         typeof(union annotated_union)),        /* Struct vs union */
        CHECK_TYPE_COMPAT(typeof(int(*)(void)), 
                         typeof(char*(*)(void))),               /* Callback vs callback */
        CHECK_TYPE_COMPAT(typeof(int[10]), typeof(int[5])),     /* Array vs array */
        CHECK_TYPE_COMPAT(typeof(user_struct_t), 
                         typeof(struct complex_struct)),        /* User struct vs struct */
    };
    
    /* Use function pointer if non-null */
    if (annotated_func_ptr) {
        annotated_func_ptr(0, 0);
    }
    
    /* Use array of function pointers */
    for (int i = 0; i < 5; i++) {
        if (func_ptr_array[i]) {
            func_ptr_array[i]();
        }
    }
    
    /* Use complex callback */
    if (complex_callback) {
        result_t r = complex_callback(1, 2.0f, &cs_instance);
    }
    
    /* Return the container's ID to ensure it's used */
    return container_instance.id == 99 ? 0 : 1;
}

/* ========== Function definitions ========== */
/* Define some callbacks */
static int sample_callback(void) {
    return 42;
}

static result_t sample_complex_callback(int a, float b, void* ctx) {
    result_t r = {0};
    r.result = a + (int)b;
    return r;
}

/* Initialize function pointer array */
__attribute__((constructor)) 
static void init_func_ptrs(void) {
    func_ptr_array[0] = sample_callback;
    complex_callback = sample_complex_callback;
}

/* ========== External references ========== */
/* Define previously extern-declared types */
struct undefined_struct {
    int dummy;
};

union undefined_union {
    int x;
    float y;
};

/* Complete forward declarations */
struct forward_declared_struct {
    int complete;
};

union forward_declared_union {
    int complete;
};

/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED) */
extern struct incomplete_struct;      /* TYPE_UNDEFINED */
extern union incomplete_union;        /* TYPE_UNDEFINED */
extern int incomplete_array[];        /* TYPE_UNDEFINED */

/* TYPE_LANG_STRUCT simulation - GCC internal language-specific struct */
struct __attribute__((annotate("gengtype"))) lang_specific {
    int lang_field;
};

/* Complex nested type definitions with annotations */
struct __attribute__((annotate("gengtype"))) outer_struct {
    /* TYPE_SCALAR variations with qualifiers */
    _Bool flag;                       /* TYPE_SCALAR */
    volatile const int counter;       /* TYPE_SCALAR with qualifiers */
    float precise_value;              /* TYPE_SCALAR */
    
    /* TYPE_STRING */
    char* message;                    /* TYPE_STRING */
    const char* const fixed_msg;      /* TYPE_STRING with qualifiers */
    
    /* TYPE_STRUCT nested */
    struct inner_struct {
        int x;
        double y;
    } inner;                          /* TYPE_STRUCT */
    
    /* TYPE_UNION nested */
    union data_union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;                           /* TYPE_UNION */
    
    /* TYPE_ARRAY variations */
    int fixed_array[10];              /* TYPE_ARRAY fixed size */
    int* vla_ptr;                     /* For variable-length array simulation */
    
    /* TYPE_POINTER variations */
    volatile const int* const volatile restrict_ptr;  /* TYPE_POINTER with qualifiers */
    struct inner_struct* struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
    
    /* TYPE_CALLBACK */
    int (*callback)(int, float);      /* TYPE_CALLBACK */
    
    /* Array of function pointers */
    void (*handlers[5])(void);        /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Pointer to incomplete type */
    struct incomplete_struct* inc_ptr; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT via typedef */
typedef struct __attribute__((annotate("gengtype"))) {
    int id;
    char name[50];
    struct outer_struct* link;
} user_struct_t;                      /* TYPE_USER_STRUCT */

/* Complex function pointer type */
typedef int (*complex_callback_t)(
    struct outer_struct*,
    user_struct_t*,
    void (*)(int)
);                                    /* TYPE_CALLBACK in typedef */

/* Union with nested complex types */
union __attribute__((annotate("gengtype"))) mega_union {
    struct outer_struct as_struct;
    user_struct_t as_user;
    complex_callback_t as_callback;
    void* as_pointer;
    int as_array[20];
};                                    /* TYPE_UNION */

/* Global variables with diverse types */
static volatile const int global_scalar = 42;                 /* TYPE_SCALAR */
static char* global_string = "Coverage test";                /* TYPE_STRING */
static struct outer_struct global_struct __attribute__((unused)); /* TYPE_STRUCT */
static user_struct_t global_user_struct __attribute__((unused)); /* TYPE_USER_STRUCT */
static union mega_union global_union __attribute__((unused));    /* TYPE_UNION */

/* Pointer variables covering all classifications */
static int* scalar_ptr __attribute__((unused)) = &global_scalar; /* TYPE_POINTER */
static char** string_ptr __attribute__((unused)) = &global_string; /* TYPE_POINTER to TYPE_POINTER */
static int (*array_ptr)[10] __attribute__((unused));          /* TYPE_POINTER to TYPE_ARRAY */
static complex_callback_t callback_ptr __attribute__((unused)); /* TYPE_POINTER to TYPE_CALLBACK */

/* Multi-dimensional array */
static int matrix[3][4] __attribute__((unused));              /* TYPE_ARRAY of TYPE_ARRAY */

/* Function using __builtin_types_compatible_p for type comparisons */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* Compare various type combinations to trigger classification */
    int check1 = __builtin_types_compatible_p(int, float);    /* Scalar vs scalar */
    int check2 = __builtin_types_compatible_p(int*, float*);  /* Pointer vs pointer */
    int check3 = __builtin_types_compatible_p(struct outer_struct*, 
                                             union mega_union*); /* Struct vs union pointer */
    int check4 = __builtin_types_compatible_p(user_struct_t,
                                             struct outer_struct); /* User struct vs struct */
    int check5 = __builtin_types_compatible_p(int(*)(void), 
                                             void(*)(void));  /* Callback vs callback */
    int check6 = __builtin_types_compatible_p(int[10], 
                                             int*);           /* Array vs pointer */
    
    /* Use volatile to prevent optimization */
    volatile int dummy __attribute__((unused)) = 
        check1 + check2 + check3 + check4 + check5 + check6;
}

/* Function with complex parameter types */
static int complex_function(
    struct outer_struct* param_struct,
    user_struct_t* param_user,
    int (*param_callback)(int, float),
    volatile const int* const param_ptr
) __attribute__((unused));

static int complex_function(
    struct outer_struct* param_struct,
    user_struct_t* param_user,
    int (*param_callback)(int, float),
    volatile const int* const param_ptr
) {
    /* Use parameters to prevent dead code elimination */
    if (param_struct && param_user && param_callback && param_ptr) {
        return param_callback(param_struct->counter, 3.14f);
    }
    return 0;
}

/* Variable-length array simulation */
static void use_vla(int size) __attribute__((unused));
static void use_vla(int size) {
    int vla[size];                     /* TYPE_ARRAY variable length */
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    
    /* Use VLA to prevent optimization */
    volatile int sum __attribute__((unused)) = 0;
    for (int i = 0; i < size; i++) {
        sum += vla[i];
    }
}

/* Main function - brings all types together */
int main(void) {
    /* Local variables covering all type classifications */
    _Bool local_bool = 1;                                 /* TYPE_SCALAR */
    int local_int = 100;                                  /* TYPE_SCALAR */
    float local_float = 3.14159f;                         /* TYPE_SCALAR */
    char* local_string = "Local string";                  /* TYPE_STRING */
    
    struct outer_struct local_struct;                     /* TYPE_STRUCT */
    user_struct_t local_user_struct;                      /* TYPE_USER_STRUCT */
    union mega_union local_union;                         /* TYPE_UNION */
    
    /* Complex pointer declarations with qualifiers */
    volatile const int* const volatile complex_ptr = &local_int; /* TYPE_POINTER */
    const struct outer_struct* struct_const_ptr = &local_struct; /* TYPE_POINTER */
    
    /* Function pointer */
    int (*local_callback)(int, float) = 0;                /* TYPE_CALLBACK */
    
    /* Arrays */
    int local_array[5] = {1, 2, 3, 4, 5};                 /* TYPE_ARRAY */
    void* ptr_array[3];                                   /* TYPE_ARRAY of TYPE_POINTER */
    
    /* Initialize struct members */
    local_struct.flag = local_bool;
    local_struct.counter = local_int;
    local_struct.precise_value = local_float;
    local_struct.message = local_string;
    local_struct.fixed_msg = "Fixed message";
    local_struct.inner.x = 10;
    local_struct.inner.y = 20.5;
    local_struct.data.as_int = 42;
    
    /* Use incomplete types in sizeof (valid in some contexts) */
    size_t inc_size = sizeof(struct incomplete_struct*);  /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Perform type comparisons */
    type_comparisons();
    
    /* Use variable-length array */
    use_vla(10);
    
    /* Call complex function */
    int result = complex_function(
        &local_struct,
        &local_user_struct,
        local_callback,
        complex_ptr
    );
    
    /* Ensure all variables are used to prevent dead code elimination */
    volatile int dummy __attribute__((unused)) = 
        local_bool + local_int + (int)local_float + 
        (local_string != 0) + (int)inc_size + result;
    
    /* Use array and pointer operations */
    for (int i = 0; i < 5; i++) {
        local_array[i] *= 2;
        ptr_array[i % 3] = (void*)&local_array[i];
    }
    
    /* Access union through different type lenses */
    local_union.as_struct = local_struct;
    local_union.as_user = local_user_struct;
    
    return 0;
}

/* Additional incomplete type usage */
static struct forward_declared;       /* TYPE_UNDEFINED forward declaration */

struct forward_declared* get_forward_ptr(void) __attribute__((unused));
struct forward_declared* get_forward_ptr(void) {
    /* This creates TYPE_POINTER to TYPE_UNDEFINED */
    return (struct forward_declared*)0;
}

/* Array of incomplete type pointers */
static struct incomplete_struct* inc_ptr_array[5] __attribute__((unused));

/* Const volatile qualified function pointer */
static int (volatile * const volatile qualified_callback)(void) __attribute__((unused));

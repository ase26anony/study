/* 
 * gengtype_test.c
 * Designed to exercise GCC's internal type classification system
 * targeting uncovered lines in gengtype.cc (lines 182-213)
 */

/* ========== TYPE_UNDEFINED and TYPE_LANG_STRUCT ========== */
/* Forward declarations creating incomplete types */
extern struct undefined_extern_struct;      /* TYPE_UNDEFINED candidate */
extern union undefined_extern_union;        /* TYPE_UNDEFINED candidate */
struct forward_declared_struct;             /* Incomplete type */
union forward_declared_union;               /* Incomplete type */

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types */
_Bool __attribute__((unused)) scalar_bool = 0;
int __attribute__((unused)) scalar_int = 42;
float __attribute__((unused)) scalar_float = 3.14f;
double __attribute__((unused)) scalar_double = 2.71828;
long double __attribute__((unused)) scalar_long_double = 1.618034L;
enum color { RED, GREEN, BLUE } __attribute__((unused)) scalar_enum = GREEN;

/* ========== TYPE_STRING ========== */
/* String literals and character pointers */
char* __attribute__((unused)) string_literal = "Hello, gengtype!";
const char* __attribute__((unused)) const_string = "Constant string";
volatile char* __attribute__((unused)) volatile_string = "Volatile string";

/* ========== TYPE_STRUCT and TYPE_UNION ========== */
/* Annotated struct and union to trigger metadata generation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Nested struct with complex members */
struct complex_container {
    struct annotated_struct nested_struct;
    union annotated_union nested_union;
    struct {
        int anonymous_x;
        char anonymous_y;
    } anonymous_struct;
};

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef struct creating TYPE_USER_STRUCT */
typedef struct {
    int id;
    char data[64];
} user_struct_t;

typedef union {
    long long_value;
    double double_value;
} user_union_t;

/* ========== TYPE_POINTER ========== */
/* Various pointer types with qualifiers */
int* __attribute__((unused)) pointer_int;
const float* __attribute__((unused)) pointer_const_float;
volatile char* __attribute__((unused)) pointer_volatile_char;
const volatile int* __attribute__((unused)) pointer_const_volatile_int;
int* const __attribute__((unused)) const_pointer_int = &scalar_int;
volatile const int* const __attribute__((unused)) complex_pointer = &scalar_int;

/* Pointer to incomplete types */
struct forward_declared_struct* __attribute__((unused)) ptr_to_incomplete;
extern struct undefined_extern_struct* __attribute__((unused)) extern_ptr;

/* ========== TYPE_ARRAY ========== */
/* Fixed-size arrays */
int __attribute__((unused)) fixed_array[10];
float __attribute__((unused)) const_array[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

/* Multi-dimensional arrays */
int __attribute__((unused)) matrix[3][3];

/* Array of pointers */
void* __attribute__((unused)) pointer_array[8];

/* Variable-length array (in function scope) */
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) vla[i] = i;
}

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct annotated_struct*, user_union_t);

/* Annotated function pointer */
int (* __attribute__((annotate("gengtype"))) annotated_func_ptr)(int, float);

/* Struct containing function pointers */
struct callback_container {
    simple_callback_t simple_cb;
    complex_callback_t complex_cb;
    int (*inline_cb)(char*, size_t);
};

/* ========== COMPLEX NESTED TYPES ========== */
/* Combining multiple type classifications */
struct __attribute__((annotate("gengtype"))) mega_struct {
    /* TYPE_SCALAR */
    _Bool flag;
    
    /* TYPE_STRING */
    char* description;
    
    /* TYPE_ARRAY of function pointers (TYPE_CALLBACK) */
    simple_callback_t callbacks[5];
    
    /* TYPE_POINTER to union */
    union annotated_union* union_ptr;
    
    /* TYPE_ARRAY of structs */
    struct annotated_struct struct_array[3];
    
    /* TYPE_POINTER to array */
    int (*matrix_ptr)[4][4];
    
    /* Nested anonymous union */
    union {
        int option_a;
        struct {
            float x, y;
        } option_b;
    } choice;
};

/* ========== TYPE COMPARISONS ========== */
/* Use __builtin_types_compatible_p to force type analysis */
static void perform_type_comparisons(void) {
    /* These comparisons should trigger type classification */
    int is_same1 = __builtin_types_compatible_p(int, float);  /* Scalar vs scalar */
    int is_same2 = __builtin_types_compatible_p(int*, float*); /* Pointer vs pointer */
    int is_same3 = __builtin_types_compatible_p(struct annotated_struct*, 
                                               union annotated_union*);
    int is_same4 = __builtin_types_compatible_p(int[10], int*);
    int is_same5 = __builtin_types_compatible_p(simple_callback_t, 
                                               void (*)(void));
    
    /* Use results to prevent dead code elimination */
    volatile int dummy __attribute__((unused)) = 
        is_same1 + is_same2 + is_same3 + is_same4 + is_same5;
}

/* ========== FUNCTION USING COMPLEX TYPES ========== */
/* Function with complex parameter types */
static void process_complex_types(
    struct mega_struct* ms,
    const volatile int* data,
    simple_callback_t cb
) __attribute__((unused));

static void process_complex_types(
    struct mega_struct* ms,
    const volatile int* data,
    simple_callback_t cb
) {
    /* Use parameters to prevent dead code elimination */
    if (ms && data && cb) {
        ms->flag = 1;
        int result = cb(ms->struct_array[0].x, *(int*)data);
        ms->description = (char*)result;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Declare and initialize complex types */
    struct mega_struct ms_instance = {
        .flag = 1,
        .description = "Mega struct instance",
        .union_ptr = 0,
    };
    
    struct callback_container cb_container = {
        .simple_cb = 0,
        .complex_cb = 0,
        .inline_cb = 0,
    };
    
    /* Use variable-length array */
    use_vla(20);
    
    /* Perform type comparisons */
    perform_type_comparisons();
    
    /* Use incomplete types in sizeof (valid in some contexts) */
    size_t incomplete_size = sizeof(struct forward_declared_struct*);
    size_t extern_size = sizeof(extern struct undefined_extern_struct*);
    
    /* Use function pointer */
    simple_callback_t local_cb = 0;
    if (local_cb) {
        local_cb(1, 2);
    }
    
    /* Access complex nested types */
    ms_instance.struct_array[0].x = 100;
    ms_instance.choice.option_a = 42;
    
    /* Use pointers with qualifiers */
    const volatile int* complex_ptr = &scalar_int;
    volatile int read_value __attribute__((unused)) = *complex_ptr;
    
    /* Use arrays */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * i;
    }
    
    /* Ensure everything is used to prevent optimization */
    volatile void* use_all __attribute__((unused)) = 
        &ms_instance, &cb_container, &scalar_bool, &string_literal;
    
    return 0;
}

/* ========== EXTERNAL LINKAGE DECLARATIONS ========== */
/* Force external linkage with incomplete types */
extern struct undefined_extern_struct external_undefined_struct;
extern union undefined_extern_union external_undefined_union;
extern int external_array[];  /* Incomplete array type */

/* ========== ADDITIONAL COMPLEX TYPEDEFS ========== */
/* Recursive pointer type */
typedef struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
} recursive_node_t;

/* Typedef for complex function pointer */
typedef int (*(*complex_func_factory)(int))(float, double);

/* Const-qualified typedef */
typedef const int const_int_t;
typedef volatile const char* vc_string_t;

/* ========== STATIC INITIALIZATION ========== */
/* Static initialization of complex types */
static struct annotated_struct static_annotated = {
    .x = 10,
    .y = 20.5f,
    .name = "Static annotated"
};

static user_struct_t static_user_struct = {
    .id = 999,
    .data = "User struct data"
};

/* Array of mixed pointers */
static void* mixed_pointer_array[] = {
    &scalar_int,
    &scalar_float,
    string_literal,
    &static_annotated,
    &static_user_struct,
    0  /* NULL terminator */
};

/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* 1. Diverse Type Declarations with __attribute__((annotate("gengtype"))) */

/* TYPE_UNDEFINED: Incomplete/forward declarations */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED array */

/* TYPE_SCALAR: Basic scalar types */
_Bool __attribute__((unused)) scalar_bool = 1;
int __attribute__((unused)) scalar_int = 42;
float __attribute__((unused)) scalar_float = 3.14f;
volatile const long __attribute__((unused)) scalar_volatile_const = 100L;

/* TYPE_STRING: String literals */
char* __attribute__((unused)) string_literal = "Hello, gengtype!";
const char* __attribute__((unused)) const_string = "Constant string";
volatile char* __attribute__((unused)) volatile_string = "Volatile string";

/* TYPE_STRUCT: Named struct types */
struct __attribute__((annotate("gengtype"))) my_struct {
    int x;
    float y;
    char* name;
};

struct __attribute__((annotate("gengtype"))) complex_struct {
    /* Nested type constructs */
    struct my_struct nested;
    volatile const int flags;
    void (*callback)(int);  /* Function pointer field */
};

/* TYPE_USER_STRUCT: Typedef structs */
typedef struct __attribute__((annotate("gengtype"))) {
    int id;
    char data[64];
    struct my_struct* link;  /* Pointer to another struct */
} user_struct_t;

/* TYPE_UNION: Named union types */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    char* as_string;
    void* as_pointer;
};

/* TYPE_POINTER: Various pointer types */
int* __attribute__((unused)) int_ptr = &scalar_int;
float* __attribute__((unused)) float_ptr = &scalar_float;
char** __attribute__((unused)) string_ptr_ptr = &string_literal;
volatile const int* const __attribute__((unused)) complex_ptr = &scalar_int;
struct my_struct* __attribute__((unused)) struct_ptr = 0;
user_struct_t* __attribute__((unused)) user_struct_ptr = 0;
union data_union* __attribute__((unused)) union_ptr = 0;

/* TYPE_ARRAY: Fixed-size and variable-length arrays */
int __attribute__((unused)) fixed_array[10] = {0};
float __attribute__((unused)) multi_dim_array[5][5];
char* __attribute__((unused)) pointer_array[20];
struct my_struct __attribute__((unused)) struct_array[3];
user_struct_t __attribute__((unused)) user_struct_array[2];

/* Variable Length Array (VLA) - C99 feature */
void use_vla(int size) {
    int vla[size];  /* TYPE_ARRAY with variable size */
    for (int i = 0; i < size; i++) vla[i] = i;
}

/* TYPE_CALLBACK: Function pointers */
typedef int (*__attribute__((annotate("gengtype"))) simple_callback_t)(int, int);
typedef void (*__attribute__((annotate("gengtype"))) complex_callback_t)(
    struct my_struct*, 
    union data_union, 
    int (*nested_callback)(float)
);

/* Complex function pointer with nested types */
int (*__attribute__((unused)) func_ptr_array[5])(const char*, int);

/* 2. Complex Nested Type Constructs */

/* Struct containing array of function pointers */
struct __attribute__((annotate("gengtype"))) callback_container {
    simple_callback_t callbacks[4];
    complex_callback_t complex_cb;
    int (*volatile volatile_cb)(void);
};

/* Union with pointer to struct */
union __attribute__((annotate("gengtype"))) pointer_union {
    struct my_struct* struct_ptr;
    user_struct_t* user_ptr;
    void* generic_ptr;
    int (*callback_ptr)(int);
};

/* Typedef for complex function callback signature */
typedef union data_union (*__attribute__((annotate("gengtype"))) mega_callback_t)(
    struct callback_container*,
    user_struct_t[][2],  /* 2D array parameter */
    volatile const int*
);

/* 3. Type comparisons using __builtin_types_compatible_p */
static void check_type_compatibility(void) {
    /* These comparisons should trigger type classification */
    int is_same1 = __builtin_types_compatible_p(int, float);  /* Scalar vs Scalar */
    int is_same2 = __builtin_types_compatible_p(int*, float*); /* Pointer vs Pointer */
    int is_same3 = __builtin_types_compatible_p(struct my_struct*, union data_union*);
    int is_same4 = __builtin_types_compatible_p(simple_callback_t, complex_callback_t);
    int is_same5 = __builtin_types_compatible_p(int[10], int[]);
    int is_same6 = __builtin_types_compatible_p(volatile int, const int);
    int is_same7 = __builtin_types_compatible_p(user_struct_t, struct my_struct);
    
    /* Use results to avoid dead code elimination */
    volatile int dummy = is_same1 + is_same2 + is_same3 + is_same4 + 
                        is_same5 + is_same6 + is_same7;
    (void)dummy;
}

/* 4. Function using many of the declared types */
static void process_types(
    struct my_struct* s,
    union data_union u,
    simple_callback_t cb,
    user_struct_t users[],
    int count
) __attribute__((unused));

static void process_types(
    struct my_struct* s,
    union data_union u,
    simple_callback_t cb,
    user_struct_t users[],
    int count
) {
    /* Use various types to ensure they're processed */
    if (s && count > 0) {
        users[0].id = cb(s->x, (int)u.as_float);
    }
    
    /* Array operations */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * 2;
    }
    
    /* Pointer arithmetic */
    char* ptr = string_literal;
    while (*ptr) {
        ptr++;
    }
}

/* 5. Main function with type declarations and minimal runtime logic */
int main(void) {
    /* Variable declarations with various qualifiers */
    const volatile int __attribute__((unused)) cv_int = 255;
    static struct my_struct __attribute__((unused)) static_struct = {1, 2.0f, "test"};
    register int __attribute__((unused)) reg_var = 99;
    
    /* Initialize union */
    union data_union __attribute__((unused)) my_union;
    my_union.as_int = 100;
    
    /* Initialize struct array */
    user_struct_t __attribute__((unused)) users[2] = {
        {1, "First", &static_struct},
        {2, "Second", 0}
    };
    
    /* Function pointer assignment */
    simple_callback_t __attribute__((unused)) my_callback = 0;
    
    /* Use sizeof with various types (works with incomplete types for pointers) */
    size_t __attribute__((unused)) sizes[] = {
        sizeof(int),
        sizeof(int*),
        sizeof(struct my_struct),
        sizeof(user_struct_t),
        sizeof(union data_union),
        sizeof(fixed_array),
        sizeof(simple_callback_t),
        sizeof(struct undefined_struct*),  /* Pointer to incomplete type */
        sizeof(undefined_array)            /* Incomplete array type */
    };
    
    /* Trigger type compatibility checks */
    check_type_compatibility();
    
    /* Use VLA */
    use_vla(5);
    
    /* Simple operation to use variables and prevent dead code elimination */
    scalar_int += (int)scalar_float;
    if (string_literal[0]) {
        scalar_int++;
    }
    
    return 0;
}

/* Additional external declarations for TYPE_UNDEFINED */
extern struct __attribute__((annotate("gengtype"))) external_struct;
extern union __attribute__((annotate("gengtype"))) external_union;

/* TYPE_LANG_STRUCT simulation - GCC internal types */
/* These might be triggered by GCC extensions or internal types */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void (*commit)(void);
};

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
v4si __attribute__((unused)) vector_var = {1, 2, 3, 4};

/* Aligned types */
struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
};

/* Packed struct */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
};

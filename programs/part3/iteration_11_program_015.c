/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED / TYPE_LANG_STRUCT ==================== */
/* Forward declarations creating incomplete/undefined types */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED array */
struct forward_declared_struct;         /* Forward declaration */
union forward_declared_union;           /* Forward declaration */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) double double_scalar = 2.71828;
__attribute__((unused)) long long long_long_scalar = 9999999999LL;

/* ==================== TYPE_STRING ==================== */
/* String literals and character pointers */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";
__attribute__((unused)) const char* const const_string_ptr = "Constant pointer to constant string";

/* ==================== TYPE_STRUCT ==================== */
/* Named struct types */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct __attribute__((annotate("gengtype"))) annotated_struct {
    int tag;
    void* data;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef structs (TYPE_USER_STRUCT) */
typedef struct {
    int a;
    double b;
    char name[32];
} user_struct_t;

typedef struct simple_struct renamed_struct_t;

/* ==================== TYPE_UNION ==================== */
/* Named union types */
union data_union {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
};

union __attribute__((annotate("gengtype"))) tagged_union {
    struct {
        int type;
    } header;
    struct {
        int type;
        int value;
    } int_data;
    struct {
        int type;
        float value;
    } float_data;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types with qualifiers */
__attribute__((unused)) volatile const int* const volatile_pointer = &int_scalar;
__attribute__((unused)) struct simple_struct* struct_pointer = 0;
__attribute__((unused)) union data_union* union_pointer = 0;
__attribute__((unused)) user_struct_t* user_struct_pointer = 0;
__attribute__((unused)) const char* const* pointer_to_string_pointer = &string_literal;
__attribute__((unused)) void (*function_pointer)(void);  /* Also TYPE_CALLBACK */

/* ==================== TYPE_ARRAY ==================== */
/* Various array types */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float matrix[3][3];
__attribute__((unused)) struct simple_struct struct_array[5];
__attribute__((unused)) union data_union union_array[8];
__attribute__((unused)) user_struct_t user_struct_array[4];

/* Variable Length Array (VLA) - size determined at runtime */
__attribute__((unused)) int* vla_pointer;  /* Will point to VLA */

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (callbacks) */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(struct simple_struct*, union data_union*, user_struct_t*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
void (*annotated_callback)(int, float, const char*);

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Struct containing array of function pointers */
struct processor {
    int id;
    complex_callback handlers[4];
    void* context;
};

/* Union with pointer to struct */
union container {
    struct processor* proc;
    user_struct_t* data;
    void* generic;
};

/* Typedef for complex function callback signature */
typedef union container* (*factory_t)(int, const char*);

/* Struct with nested union field */
struct nested_example {
    int type;
    union {
        int int_value;
        float float_value;
        struct processor* proc_ptr;
        comparator_t compare_func;
    } data;
    struct nested_example* next;  /* Self-referential pointer */
};

/* ==================== QUALIFIED TYPE VARIATIONS ==================== */
/* Complex qualified types */
__attribute__((unused)) volatile const int* const restrict volatile_qualified = &int_scalar;
__attribute__((unused)) const struct simple_struct* const const_struct_ptr = 0;
__attribute__((unused)) volatile union data_union* volatile volatile_union_ptr = 0;

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p for type comparisons */
#define CHECK_TYPE_COMPAT(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* ==================== FUNCTION DECLARATIONS ==================== */
/* Function using various types */
static void use_types(void) {
    /* Force evaluation of type compatibility checks */
    int compat1 = CHECK_TYPE_COMPAT(int, float);              /* Scalar vs Scalar */
    int compat2 = CHECK_TYPE_COMPAT(struct simple_struct*, union data_union*); /* Struct* vs Union* */
    int compat3 = CHECK_TYPE_COMPAT(user_struct_t, struct simple_struct); /* User struct vs Struct */
    int compat4 = CHECK_TYPE_COMPAT(int*, int[]);            /* Pointer vs Array */
    int compat5 = CHECK_TYPE_COMPAT(comparator_t, void*);    /* Callback vs Pointer */
    int compat6 = CHECK_TYPE_COMPAT(const char*, char*);     /* Qualified vs unqualified */
    
    /* Use VLAs */
    int vla_size = 20;
    int vla[vla_size];
    vla_pointer = vla;
    
    /* Use incomplete types in sizeof (valid in some contexts) */
    size_t s1 = sizeof(struct forward_declared_struct*);
    size_t s2 = sizeof(union forward_declared_union*);
    
    /* Trivial operations to prevent dead code elimination */
    if (compat1) int_scalar++;
    if (compat2) float_scalar *= 2.0f;
    if (vla_pointer) vla[0] = 42;
}

/* Callback function implementations */
static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(void) {
    /* Empty callback */
}

static int complex_handler(struct simple_struct* s, union data_union* u, user_struct_t* us) {
    if (s && u && us) return 1;
    return 0;
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Variable declarations with initialization */
    struct simple_struct my_struct = {1, 2.5f, 'A'};
    union data_union my_union = {.as_int = 100};
    user_struct_t my_user_struct = {10, 3.14159, "Test"};
    struct processor my_processor = {0};
    union container my_container = {0};
    struct nested_example my_nested = {0};
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) fixed_array[i] = i * i;
    struct_array[0] = my_struct;
    
    /* Initialize function pointers */
    comparator_t my_comparator = sample_comparator;
    simple_callback my_simple_cb = sample_callback;
    complex_callback my_complex_cb = complex_handler;
    annotated_callback = 0;  /* Will be set to null */
    
    /* Initialize nested structure */
    my_nested.type = 1;
    my_nested.data.int_value = 42;
    my_nested.next = &my_nested;  /* Self-reference */
    
    /* Use the types in trivial operations */
    my_struct.x++;
    my_union.as_float = 3.14f;
    my_user_struct.a = my_comparator(&int_scalar, &fixed_array[0]);
    
    /* Call the type usage function */
    use_types();
    
    /* Use function pointers (trivial calls) */
    if (my_simple_cb) my_simple_cb();
    if (my_complex_cb) my_complex_cb(&my_struct, &my_union, &my_user_struct);
    
    /* Return success */
    return 0;
}

/* ==================== TYPE DEFINITIONS FOR FORWARD DECLARATIONS ==================== */
/* Define previously forward-declared types */
struct forward_declared_struct {
    int dummy;
};

union forward_declared_union {
    int x;
    float y;
};

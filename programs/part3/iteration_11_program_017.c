/* gengtype_trigger.c
 * 
 * This program is designed to trigger coverage of the type classification
 * switch cases in gengtype.cc (lines 182-213) by presenting a diverse set of
 * type declarations and constructs to GCC's internal type system.
 * 
 * Compilation recommendations:
 *   gcc -O1 -fdump-tree-gimple -frandom-seed=1 gengtype_trigger.c -o gengtype_trigger
 *   gcc -O2 -g -dA gengtype_trigger.c -o gengtype_trigger_annotated
 *   gcc -flto -O2 -fno-fat-lto-objects gengtype_trigger.c -o gengtype_trigger_lto
 */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */

/* Forward declarations creating incomplete/undefined types */
extern struct undefined_extern_struct;           /* TYPE_UNDEFINED */
extern int undefined_extern_array[];             /* TYPE_UNDEFINED */
struct forward_declared_struct;                  /* TYPE_UNDEFINED */
union forward_declared_union;                    /* TYPE_UNDEFINED */

/* ==================== TYPE_SCALAR ==================== */

/* Basic scalar types */
static _Bool __attribute__((unused)) scalar_bool = 0;                    /* TYPE_SCALAR */
static int __attribute__((unused)) scalar_int = 42;                      /* TYPE_SCALAR */
static float __attribute__((unused)) scalar_float = 3.14f;               /* TYPE_SCALAR */
static volatile const long __attribute__((unused)) scalar_volatile_const = 100L; /* TYPE_SCALAR */

/* ==================== TYPE_STRING ==================== */

/* String literals and pointers */
static char* __attribute__((unused)) string_literal = "Hello, gengtype!"; /* TYPE_STRING */
static const char* const __attribute__((unused)) const_string_ptr = "Constant"; /* TYPE_STRING */
static volatile char* __attribute__((unused)) volatile_string_ptr = "Volatile"; /* TYPE_STRING */

/* ==================== TYPE_STRUCT & TYPE_UNION ==================== */

/* Basic struct and union types with annotations */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
}; /* TYPE_STRUCT */

union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
}; /* TYPE_UNION */

/* Complex nested struct */
struct nested_container {
    struct annotated_struct inner_struct;        /* TYPE_STRUCT */
    union annotated_union inner_union;           /* TYPE_UNION */
    struct nested_container* next;               /* TYPE_POINTER */
}; /* TYPE_STRUCT */

/* ==================== TYPE_USER_STRUCT ==================== */

/* Typedef struct creating TYPE_USER_STRUCT */
typedef struct {
    int id;
    char tag;
} user_struct_t; /* TYPE_USER_STRUCT */

/* Another typedef with qualifiers */
typedef volatile const struct nested_container vc_nested_t; /* TYPE_USER_STRUCT */

/* ==================== TYPE_POINTER ==================== */

/* Pointers to various types */
static int* __attribute__((unused)) int_ptr = NULL;                      /* TYPE_POINTER */
static volatile const int* const __attribute__((unused)) complex_ptr = (int*)0x1000; /* TYPE_POINTER */
static struct annotated_struct* __attribute__((unused)) struct_ptr = NULL; /* TYPE_POINTER */
static union annotated_union* __attribute__((unused)) union_ptr = NULL;   /* TYPE_POINTER */
static user_struct_t* __attribute__((unused)) user_struct_ptr = NULL;     /* TYPE_POINTER */
static void (* __attribute__((unused)) func_ptr)(void);                   /* TYPE_POINTER */

/* Pointer to incomplete type */
static struct forward_declared_struct* __attribute__((unused)) incomplete_ptr = NULL; /* TYPE_POINTER */

/* ==================== TYPE_ARRAY ==================== */

/* Various array types */
static int __attribute__((unused)) fixed_array[10];                      /* TYPE_ARRAY */
static float __attribute__((unused)) volatile_array[5][3];               /* TYPE_ARRAY */
static const char* __attribute__((unused)) string_array[] = {"a", "b", "c"}; /* TYPE_ARRAY */
static struct annotated_struct __attribute__((unused)) struct_array[2];  /* TYPE_ARRAY */

/* Variable-length array (in function scope) */
static void use_vla(int n) {
    int __attribute__((unused)) vla[n];                                  /* TYPE_ARRAY */
    /* Use to avoid dead code elimination */
    for (int i = 0; i < n; i++) vla[i] = i;
}

/* ==================== TYPE_CALLBACK ==================== */

/* Function pointer types (callbacks) */
typedef int (*binary_op_t)(int, int);                                    /* TYPE_CALLBACK */
typedef void (* __attribute__((annotate("gengtype"))) event_callback_t)(void* data, int id); /* TYPE_CALLBACK */

/* Struct containing function pointers */
struct callback_container {
    binary_op_t op;                                                      /* TYPE_CALLBACK */
    event_callback_t handler;                                            /* TYPE_CALLBACK */
    void* user_data;                                                     /* TYPE_POINTER */
}; /* TYPE_STRUCT */

/* ==================== TYPE_LANG_STRUCT ==================== */

/* Using __builtin_types_compatible_p for type comparisons */
/* These expressions force GCC to classify types internally */
#define CHECK_TYPE_COMPAT(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    /* Variable declarations using our diverse types */
    struct annotated_struct __attribute__((unused)) my_struct = {1, 2.0f, "test"};
    union annotated_union __attribute__((unused)) my_union;
    my_union.as_int = 42;
    
    user_struct_t __attribute__((unused)) my_user_struct = {100, 'A'};
    struct nested_container __attribute__((unused)) nested = {my_struct, my_union, NULL};
    
    /* Initialize function pointers */
    binary_op_t __attribute__((unused)) add = NULL;
    event_callback_t __attribute__((unused)) callback = NULL;
    
    /* Use variable-length array */
    use_vla(5);
    
    /* Use sizeof with incomplete types (valid for pointers) */
    size_t __attribute__((unused)) sz1 = sizeof(struct forward_declared_struct*);
    size_t __attribute__((unused)) sz2 = sizeof(undefined_extern_array);
    
    /* Trigger type comparisons using __builtin_types_compatible_p */
    int __attribute__((unused)) compat_results[] = {
        CHECK_TYPE_COMPAT(int, float),              /* scalar vs scalar */
        CHECK_TYPE_COMPAT(int*, int),               /* pointer vs scalar */
        CHECK_TYPE_COMPAT(struct annotated_struct*, union annotated_union*), /* struct vs union */
        CHECK_TYPE_COMPAT(int[10], int*),           /* array vs pointer */
        CHECK_TYPE_COMPAT(binary_op_t, void*),      /* callback vs pointer */
        CHECK_TYPE_COMPAT(user_struct_t, struct annotated_struct), /* user struct vs struct */
    };
    
    /* Use complex pointer */
    volatile const int* const __attribute__((unused)) restrict_ptr = (int*)0x2000;
    
    /* Use struct with callback */
    struct callback_container __attribute__((unused)) cb_container = {NULL, NULL, NULL};
    
    /* Ensure no dead code elimination */
    if (scalar_int > 0) {
        scalar_float += 1.0f;
        if (string_literal) {
            scalar_int = (int)scalar_float;
        }
    }
    
    /* Return statement to avoid fallthrough warning */
    return 0;
}

/* ==================== DEFINITIONS FOR FORWARD DECLARATIONS ==================== */

/* Define previously forward-declared types */
struct forward_declared_struct {
    int defined_now;
    float another_field;
};

union forward_declared_union {
    long as_long;
    double as_double;
};

/* External declarations get tentative definitions */
struct undefined_extern_struct undefined_struct_instance;
int undefined_extern_array[5] = {1, 2, 3, 4, 5};

/* test_gty.h - Comprehensive GTY annotation test header */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) my_undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    struct my_struct * GTY((skip)) next;
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    double c;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree GTY((user)) my_tree_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct * GTY((length)) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
my_callback_fn GTY((user)) callback_ptr;

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_id;
    union {
        tree GTY((tag("0"))) t;
        rtx GTY((tag("1"))) r;
    } GTY((desc("%1.lang_id"))) u;
};

/* Additional complex type to ensure thorough parsing */
struct GTY(()) complex_container {
    /* Contains multiple type categories */
    my_scalar_t scalar_field;          /* TYPE_SCALAR */
    const char * GTY((length)) str;    /* TYPE_STRING */
    struct my_struct * GTY((skip)) p;  /* TYPE_POINTER */
    int GTY((length)) arr[20];         /* TYPE_ARRAY */
    union my_union GTY((desc("0"))) u; /* TYPE_UNION */
};

/* Global variables with GTY annotations */
extern struct my_struct GTY(()) *global_struct_ptr;
extern int GTY((length)) global_int_array[100];
extern const char * GTY((length)) global_string_array[50];

/* Nested structure with callback */
struct GTY(()) nested_with_callback {
    int id;
    my_callback_fn GTY((user)) handler;
    struct nested_with_callback * GTY((skip)) next;
};

/* Template-like pattern (not actual C++ template) */
#define DEFINE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type GTY((skip)) data; \
        struct name * GTY((skip)) next; \
    }

DEFINE_GTY_STRUCT(gty_list_int, int);
DEFINE_GTY_STRUCT(gty_list_ptr, void*);

/* Ensure we have at least one of each category:
   - TYPE_UNDEFINED: my_undefined_struct (line 14)
   - TYPE_SCALAR: my_scalar_t (line 17)
   - TYPE_STRING: my_string (line 20)
   - TYPE_STRUCT: my_struct (line 23)
   - TYPE_USER_STRUCT: my_user_struct_t (line 31)
   - TYPE_UNION: my_union (line 34)
   - TYPE_POINTER: my_pointer (line 41)
   - TYPE_ARRAY: my_array (line 44)
   - TYPE_CALLBACK: my_callback_fn (line 47)
   - TYPE_LANG_STRUCT: lang_specific_struct (line 51)
*/

#endif /* TEST_GTY_H */

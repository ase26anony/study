/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (as in standalone gengtype run) */
#ifndef GTY
#define GTY(x) __attribute__((gty(x)))
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct with tag */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field1;
    tree field2;  /* Use dummy GCC type */
    void* field3;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef with user marker */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ==================== TYPE_UNION ==================== */
/* Union with desc tag for discrimination */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char * GTY((skip)) b;  /* Skip this pointer field */
    struct my_struct * GTY((tag("my_struct"))) c;
    double d;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointer type with skip attribute */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* Another pointer type */
union my_union * GTY((tag("my_union"))) my_union_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Array with length attribute */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */

/* Array of pointers */
struct my_struct * GTY((length("struct_count"))) struct_array[5];

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific struct - often has special handling */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
    int lang_specific;
    union {
        int a;
        void *p;
        tree t;
    } u;
    rtx code;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar type with user marker */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

typedef double GTY((user)) my_double_t;

/* ==================== TYPE_STRING ==================== */
/* String pointer with length attribute */
const char * GTY((length("strlen(my_string)+1"))) my_string;  /* TYPE_STRING */

/* Another string example */
char * GTY((length("custom_len_func()"))) dynamic_string;

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int);  /* TYPE_CALLBACK */

/* More complex callback */
typedef int (*GTY((user)) complex_callback)(tree, rtx, void*);

/* ==================== Nested and Complex Types ==================== */

/* Struct containing various types */
struct GTY((tag("container"))) container_struct {
    my_scalar_t scalar_field;          /* TYPE_SCALAR via typedef */
    struct my_struct * GTY((skip)) ptr_field;  /* TYPE_POINTER */
    int GTY((length("array_len"))) array_field[20];  /* TYPE_ARRAY */
    const char * GTY((length("name_len"))) name;  /* TYPE_STRING */
    union my_union union_field;        /* TYPE_UNION */
    my_callback_fn callback;           /* TYPE_CALLBACK */
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct container_struct s;
    struct {
        int x;
        char * GTY((skip)) y;
    } nested;
    my_double_t dbl;
};

/* Array of unions */
union complex_union GTY((length("union_count"))) union_array[8];

/* Pointer to lang struct */
struct my_lang_struct * GTY((tag("lang_ptr"))) lang_struct_ptr;

/* Undefined pointer type */
struct undefined_container * GTY((skip)) undefined_ptr;

/* Forward declaration for undefined pointer target */
struct undefined_container;

#endif /* TEST_STATE_GTY_H */

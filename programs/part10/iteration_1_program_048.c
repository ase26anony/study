/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* Forward declaration for undefined type */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* Simple struct type */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field1;
    tree field2;
    struct my_struct *next;
};

/* User struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* Union type with desc tag */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char * GTY((skip)) b;
    tree c;
    struct {
        int x;
        int y;
    } nested;
};

/* Pointer type */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* Array type with length attribute */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */
extern int my_array_len;

/* Language-specific struct */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
    int lang_specific;
    union {
        int a;
        void *p;
        tree t;
    } u;
    struct my_lang_struct *next;
};

/* Scalar type */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* String type */
const char * GTY((length("strlen(my_string)"))) my_string;  /* TYPE_STRING */

/* Callback (function pointer) type */
typedef void (*GTY((user)) my_callback_fn)(int, tree);  /* TYPE_CALLBACK */

/* Additional complex types to ensure thorough coverage */

/* Nested struct with pointer chain */
struct GTY((tag("complex_struct"))) complex_struct {
    struct my_struct * GTY((skip)) ptr1;
    union my_union data;
    int GTY((length("count"))) *dynamic_array;
    int count;
};

/* Another undefined type for good measure */
struct GTY(()) another_undefined;  /* TYPE_UNDEFINED */

/* Array of pointers */
struct my_struct * GTY((length("ptr_count"))) ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
extern int ptr_count;

/* Union with nested struct */
union GTY((desc("1"))) nested_union {
    struct {
        int x;
        int y;
    } point;
    tree node;
    rtx insn;
};

/* Struct with callback field */
struct GTY((tag("with_callback"))) struct_with_callback {
    int id;
    my_callback_fn GTY((skip)) handler;
};

/* Ensure all types are referenced to avoid unused type warnings */
void dummy_references(void) {
    struct my_undefined_struct *undef_ptr = 0;
    struct my_struct s = {0};
    my_user_struct_t us;
    union my_union u;
    struct my_struct *p = my_pointer;
    int arr_val = my_array[0];
    struct my_lang_struct ls;
    my_scalar_t scalar = 0;
    const char *str = my_string;
    my_callback_fn cb = 0;
    struct complex_struct cs;
    struct another_undefined *au = 0;
    struct my_struct **pa = ptr_array;
    union nested_union nu;
    struct struct_with_callback wc;
    
    /* Silence unused variable warnings */
    (void)undef_ptr;
    (void)s;
    (void)us;
    (void)u;
    (void)p;
    (void)arr_val;
    (void)ls;
    (void)scalar;
    (void)str;
    (void)cb;
    (void)cs;
    (void)au;
    (void)pa;
    (void)nu;
    (void)wc;
}

#endif /* TEST_STATE_GTY_H */

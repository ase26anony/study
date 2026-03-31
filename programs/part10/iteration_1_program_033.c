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

/* Forward declarations for undefined types */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* TYPE_STRUCT: Simple struct with tag */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    char *field2;
    tree gcc_tree_field;  /* Use dummy GCC type */
};

/* TYPE_USER_STRUCT: Typedef with user marker */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* TYPE_UNION: Union with descriminant */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
    rtx d;  /* Use dummy GCC type */
};

/* TYPE_POINTER: Various pointer types */
struct my_struct * GTY((skip)) my_pointer;
union my_union * GTY((skip)) my_union_ptr;
tree * GTY((skip)) tree_ptr;

/* TYPE_ARRAY: Arrays with length attributes */
int GTY((length("my_array_len"))) my_array[10];
struct my_struct * GTY((length("struct_array_len"))) struct_array[5];
char * GTY((length("str_array_len"))) string_array[3];

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void *p;
        tree t;
    } u;
    gimple stmt;  /* Use dummy GCC type */
};

/* TYPE_SCALAR: Scalar typedef with user marker */
typedef int GTY((user)) my_scalar_t;
typedef long GTY((user)) my_long_scalar_t;

/* TYPE_STRING: String pointers */
const char * GTY((length)) my_string;
char * GTY((length)) mutable_string;

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (*GTY((user)) my_callback_fn)(int, char*);
typedef int (*GTY((user)) tree_callback_fn)(tree, rtx);

/* Additional complex types to ensure full traversal */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct * GTY((skip)) ptr_field;
    union my_union union_field;
    int GTY((length("nested_len"))) nested_array[8];
};

/* Struct containing callback */
struct GTY((tag("with_callback"))) struct_with_callback {
    my_callback_fn callback;
    int data;
};

/* Union with nested array */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    int GTY((length("union_array_len"))) union_array[4];
    my_callback_fn func_ptr;
};

/* Global variables with GTY markers for additional coverage */
extern struct my_struct GTY(()) *global_struct_ptr;
extern union my_union GTY(()) *global_union_ptr;
extern int GTY((user)) global_scalar;

#endif /* TEST_STATE_GTY_H */

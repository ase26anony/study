/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined (for standalone testing) */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid parsing errors */
typedef int tree;
typedef void* rtx;
typedef int gimple;

/* 
 * TYPE_UNDEFINED: Forward declaration without definition
 * This should trigger write_state_undefined_type()
 */
struct GTY(()) my_undefined_struct;

/* 
 * TYPE_SCALAR: Scalar type with user annotation  
 * This should trigger write_state_scalar_type()
 */
typedef int GTY((user)) my_scalar_t;

/* 
 * TYPE_STRUCT: Regular struct with tag
 * This should trigger write_state_struct_type()
 */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    my_scalar_t field2;
    struct my_undefined_struct* next; /* Forward reference */
};

/* 
 * TYPE_USER_STRUCT: Typedef with user annotation
 * This should trigger write_state_user_struct_type()
 */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* 
 * TYPE_UNION: Union with descriminator
 * This should trigger write_state_union_type()
 */
union GTY((desc("0"))) my_union {
    int a;
    char* GTY((skip)) b;  /* Skip this pointer field */
    struct my_struct* c;
};

/* 
 * TYPE_POINTER: Pointer type with skip annotation
 * This should trigger write_state_pointer_type()
 */
struct my_struct* GTY((skip)) my_pointer;

/* 
 * TYPE_ARRAY: Array with length attribute
 * This should trigger write_state_array_type()
 */
int GTY((length("my_array_length"))) my_array[10];
extern int my_array_length; /* Declaration for length function */

/* 
 * TYPE_LANG_STRUCT: Language-specific struct
 * This should trigger write_state_lang_struct_type()
 */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        struct my_struct* s;
    } u;
    tree dummy_tree; /* GCC internal type */
};

/* 
 * TYPE_STRING: String pointer with length
 * This should trigger write_state_string_type()
 */
const char* GTY((length("my_string_length"))) my_string;
extern int my_string_length;

/* 
 * TYPE_CALLBACK: Function pointer with user annotation
 * This should trigger write_state_callback_type()
 */
typedef void (*GTY((user)) my_callback_fn)(int, struct my_struct*);

/* Additional complex types to ensure thorough coverage */

/* Nested struct with multiple pointer types */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((tag("my_struct"))) ptr1;
    union my_union* GTY((skip)) ptr2;
    my_callback_fn callback;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_array_len"))) ptr_array[5];
extern int ptr_array_len;

/* Struct containing arrays */
struct GTY((tag("array_container"))) array_container {
    int GTY((length("container_len"))) data[20];
    char* GTY((length("str_len"))) strings[3];
};
extern int container_len;
extern int str_len;

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct my_struct s;
    struct nested_struct ns;
    my_scalar_t scalar;
};

/* Forward declaration for mutual reference */
struct GTY(()) forward_decl;

/* Struct with mutual reference */
struct GTY((tag("mutual_struct"))) mutual_struct {
    int id;
    struct forward_decl* GTY((skip)) partner;
};

/* Definition of forward_decl */
struct GTY(()) forward_decl {
    int value;
    struct mutual_struct* GTY((tag("mutual_struct"))) owner;
};

#endif /* TEST_STATE_GTY_H */

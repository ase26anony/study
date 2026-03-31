/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependencies */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_SCALAR: Scalar type with user annotation
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {  /* TYPE_STRUCT */
    int field1;
    my_scalar_t field2;
    struct my_undefined_struct* next;  /* Reference to undefined type */
};

/* ============================================
   TYPE_USER_STRUCT: Typedef of struct with user annotation
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {  /* TYPE_UNION */
    int a;
    char* GTY((skip)) b;  /* Skip annotation on pointer field */
    struct my_struct* c;
};

/* ============================================
   TYPE_POINTER: Pointer type declaration
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */

/* Helper for array length */
extern int my_array_length;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("my_string_length"))) my_string;  /* TYPE_STRING */

/* Helper for string length */
extern int my_string_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {  /* TYPE_LANG_STRUCT */
    int lang_specific;
    union {
        int a;
        void* p;
        struct my_struct* s;
    } u;
    tree dummy_tree;  /* GCC internal type */
    rtx dummy_rtx;    /* GCC internal type */
};

/* ============================================
   TYPE_CALLBACK: Function pointer type
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, struct my_struct*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure thorough traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
    struct my_struct* GTY((skip)) ptr1;
    union my_union data;
    struct nested_struct* GTY((skip)) next;
};

/* Array of pointers */
struct my_struct* GTY((length("ptr_array_len"))) ptr_array[5];

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
    struct {
        int x;
        int y;
    } GTY((tag("point"))) point;
    struct my_struct* GTY((skip)) sptr;
    my_callback_fn callback;
};

/* Struct with multiple pointer types */
struct GTY((tag("multi_ptr_struct"))) multi_ptr_struct {
    struct my_struct* GTY((skip)) regular_ptr;
    const char* GTY((length("str_len"))) string_ptr;
    void* GTY((skip)) void_ptr;
    int* GTY((skip)) int_ptr;
};

/* Global variables with various GTY annotations */
extern struct my_struct GTY((tag("global_struct"))) global_struct_instance;
extern union my_union GTY((desc("2"))) global_union_instance;
extern struct my_lang_struct GTY((special("global_lang"))) global_lang_instance;

/* Function declarations using callback type */
void register_callback(my_callback_fn GTY((skip)) cb);
void process_struct(struct my_struct* GTY((skip)) s);

#endif /* TEST_STATE_GTY_H */

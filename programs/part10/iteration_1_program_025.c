/* test_state_gty.h - Comprehensive GTY annotations for all type categories */

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
   TYPE_UNDEFINED: Forward declaration without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Regular struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
    int field1;
    tree field2;  /* Using dummy GCC type */
    void* GTY((skip)) skip_field;  /* Skip this pointer field */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag
   ============================================ */
union GTY((desc("0"))) my_union {
    int a;
    char* GTY((skip)) b;  /* Skip pointer in union */
    struct my_struct* GTY((tag("1"))) c;
    double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer type
   ============================================ */
struct my_struct* GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* Array of pointers */
struct my_struct* GTY((length("10"))) pointer_array[10];

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_len"))) my_array[10];  /* TYPE_ARRAY */

/* Variable to hold array length */
extern int my_array_len;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
    int lang_specific;
    union {
        int a;
        void* p;
        tree t;  /* GCC type */
    } u;
    rtx insn;  /* Another GCC type */
};  /* TYPE_LANG_STRUCT */

/* Alternative lang struct pattern */
struct GTY(()) lang_struct_base {
    int base_field;
};

struct GTY(()) my_other_lang_struct {
    struct lang_struct_base base;
    void* lang_data;
};

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* Another scalar type */
typedef double GTY((user)) my_double_t;

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char* GTY((length("str_len"))) my_string;  /* TYPE_STRING */

/* Variable for string length */
extern int str_len;

/* Array of strings */
const char* GTY((length("str_array_len"))) string_array[5];

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, void*);  /* TYPE_CALLBACK */

/* Callback with GCC types */
typedef tree (*GTY((user)) tree_callback_fn)(tree, rtx);

/* ============================================
   Complex nested structure to exercise more paths
   ============================================ */
struct GTY((tag("complex_struct"))) complex_struct {
    my_scalar_t scalar_field;
    my_user_struct_t user_struct_field;
    union my_union union_field;
    struct my_lang_struct* GTY((skip)) lang_struct_ptr;
    my_callback_fn callback_field;
    const char* GTY((length("name_len"))) name;
    int name_len;
};

/* Global variables for testing */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct my_lang_struct GTY(()) global_lang_struct;

/* Function declarations */
void register_callback(my_callback_fn GTY((user)) cb);
struct my_struct* GTY((returns_nonnull)) create_struct(void);

#endif /* TEST_STATE_GTY_H */

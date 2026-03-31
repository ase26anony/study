/* test-coverage.gt - Comprehensive type definitions to cover all gengtype switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char*, float);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
typedef char char_array[20];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    char d[4];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GTY struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* TYPE_SCALAR */
    scalar_int count;
    enum my_enum state;
    
    /* Nested TYPE_UNION */
    union {
        int int_val;
        float float_val;
    } value;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct *user_data;
    
    /* Flexible array member (variable-length array) */
    char flexible_array[];
};

/* Another GTY structure with nested struct */
struct GTY(()) outer_struct {
    struct complex_struct inner;
    
    /* Array of pointers */
    struct plain_struct *ptr_array[8];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Union with struct */
    union {
        struct plain_struct ps;
        struct user_struct us;
    } choice;
};

/* TYPE_ARRAY with pointer elements */
typedef struct outer_struct *outer_ptr_array[4];

/* Callback that uses multiple types */
typedef struct complex_struct* (*factory_callback)(int, const char*);

/* Union containing callback */
union callback_union {
    callback_type func1;
    factory_callback func2;
};

/* Opaque pointer type (triggers TYPE_UNDEFINED when referenced) */
typedef struct opaque_struct *opaque_ptr;

/* Struct with opaque pointer */
struct GTY(()) has_opaque {
    opaque_ptr unknown;
    int known_field;
};

/* Enumeration type (TYPE_SCALAR) */
typedef enum {
    RED,
    GREEN,
    BLUE
} color_enum;

/* Array of strings */
const char *string_array[] = {
    "first",
    "second",
    "third"
};

/* Struct with array of callbacks */
struct GTY(()) callback_container {
    callback_type handlers[5];
    int handler_count;
};

/* Language struct with nested arrays */
struct GTY((tag("LANG"))) nested_lang_struct {
    struct lang_specific items[10];
    int (*processor[3])(struct lang_specific*);
};

/* Union with array */
union array_union {
    int ints[10];
    float floats[10];
    void* pointers[10];
};

/* Final comprehensive type that references many others */
struct GTY(()) master_type {
    struct complex_struct cs;
    struct outer_struct os;
    struct user_struct *us_ptr;
    union my_union mu;
    struct lang_specific ls;
    callback_type cb;
    string_type str;
    scalar_int num;
    color_enum color;
    int_ptr int_pointer;
    char_array chars;
    struct has_opaque ho;
    struct callback_container cc;
    struct nested_lang_struct nls;
    union array_union au;
    factory_callback factory;
};

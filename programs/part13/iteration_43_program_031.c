/* test-coverage.gt - Comprehensive type definitions to cover all gengtype TYPE_* cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef double scalar_double;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char *string_array[5];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    double field2;
    enum my_enum field3;
};

/* TYPE_USER_STRUCT: GTY-marked user-defined structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
    callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    double c;
    struct plain_struct *d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    union my_union lang_union;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER field */
    struct opaque_struct *opaque_ptr;
    
    /* TYPE_ARRAY field */
    int numbers[20];
    
    /* TYPE_STRING field */
    const char *name;
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* TYPE_UNION field */
    union my_union data_union;
    
    /* TYPE_STRUCT field (nested) */
    struct plain_struct plain;
    
    /* TYPE_USER_STRUCT field */
    struct user_struct *user_data GTY((skip));
    
    /* TYPE_LANG_STRUCT field */
    struct lang_specific *lang_ptr;
    
    /* Flexible array member (TYPE_ARRAY variant) */
    int flexible_array[];
};

/* Another GTY structure with pointers to various types */
struct GTY(()) container {
    /* TYPE_POINTER to scalar */
    scalar_int *int_ptr;
    
    /* TYPE_POINTER to string */
    string_type *strings;
    
    /* TYPE_POINTER to array */
    fixed_array *array_ptr;
    
    /* TYPE_POINTER to callback */
    callback_type *callbacks;
    
    /* TYPE_POINTER to union */
    union my_union *union_ptr;
    
    /* Nested structure containing another structure */
    struct complex_nested nested;
    
    /* Pointer to language-specific structure */
    struct lang_specific *lang_struct;
};

/* Union containing GTY structures */
union GTY((desc ("%0.type"))) tagged_union {
    int type;
    struct container *container_ptr;
    struct complex_nested *nested_ptr;
    struct lang_specific *lang_ptr;
};

/* Root structure that references everything */
struct GTY(()) root_struct {
    /* Various scalar types (TYPE_SCALAR) */
    scalar_int count;
    scalar_double value;
    enum my_enum choice;
    
    /* String (TYPE_STRING) */
    string_type description;
    
    /* Callback (TYPE_CALLBACK) */
    callback_type notify;
    
    /* Arrays (TYPE_ARRAY) */
    fixed_array data;
    string_array messages;
    
    /* Pointers to different types (TYPE_POINTER) */
    struct opaque_struct *unknown;
    struct container *items;
    struct user_struct *user_info;
    
    /* Union (TYPE_UNION) */
    union my_union variant;
    
    /* Tagged union */
    union tagged_union tagged;
    
    /* Language-specific structure */
    struct lang_specific lang_data;
};

/* Additional test cases for edge scenarios */

/* Structure with function pointer array */
struct GTY(()) func_table {
    callback_type functions[10];
    another_callback more_funcs[5];
};

/* Structure with nested anonymous struct/union */
struct GTY(()) anonymous_members {
    union {
        int x;
        void *y;
    } data;
    
    struct {
        int a;
        int b;
    } coord;
};

/* Pointer to pointer chain */
typedef struct container **container_ptr_ptr;

/* Const pointer types */
typedef const struct root_struct *const_root_ptr;
typedef int *const const_int_ptr;

/* Array of pointers */
typedef struct container *container_array[10];

/* Structure with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
    unsigned int flag:1;
    unsigned int value:8;
    unsigned int :3;  /* padding */
    unsigned int mode:4;
};

/* Void pointer type */
typedef void *generic_ptr;

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Structure with variable-length array at end */
struct GTY(()) var_len_struct {
    int length;
    int data[1];  /* Actually variable length */
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int defined_now;
    void *some_data;
};

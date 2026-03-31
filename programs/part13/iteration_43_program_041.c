/* test-coverage.gt - Comprehensive type definitions to cover all TYPE_* cases */

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
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[][20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
    callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    callback_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    string_type lang_name;
    enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER within struct */
    struct plain_struct *plain_ptr;
    
    /* TYPE_USER_STRUCT reference */
    struct user_struct *user_ptr;
    
    /* TYPE_UNION field */
    union my_union data_union;
    
    /* TYPE_ARRAY within struct */
    int number_array[5];
    
    /* Flexible array member */
    char flexible_array[];
};

/* Another GC-tracked structure with various type combinations */
struct GTY(()) mixed_types {
    /* TYPE_SCALAR fields */
    scalar_int count;
    enum my_enum current_state;
    
    /* TYPE_STRING field */
    string_type name;
    
    /* TYPE_POINTER fields */
    int_ptr numbers;
    generic_ptr user_data;
    
    /* TYPE_ARRAY field */
    fixed_array values;
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* TYPE_STRUCT field */
    struct plain_struct plain;
    
    /* TYPE_UNION field */
    union my_union variant;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_specific *lang_ptr;
    
    /* Nested TYPE_ARRAY of pointers */
    struct complex_nested *nested_array[3];
};

/* Union with GTY marker */
union GTY(()) tagged_union {
    struct mixed_types *as_mixed;
    struct complex_nested *as_complex;
    string_type as_string;
};

/* Callback structure using function pointers */
struct GTY(()) callback_container {
    callback_type on_start;
    callback_type on_data;
    callback_type on_end;
    another_callback on_error;
};

/* Array of structures */
typedef struct mixed_types mixed_array[4];

/* Pointer to array */
typedef mixed_array *mixed_array_ptr;

/* Structure with nested anonymous struct/union */
struct GTY(()) anonymous_member {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } value;
};

/* Opaque pointer type (TYPE_POINTER to TYPE_UNDEFINED) */
typedef struct opaque_struct *opaque_ptr;

/* Structure with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int count : 8;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Structure with array of function pointers */
struct GTY(()) callback_array {
    const char *name;
    callback_type callbacks[5];
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
    void *raw;
    int_ptr ints;
    string_type str;
    struct mixed_types *mixed;
    callback_type func;
};

/* Final structure that references everything */
struct GTY(()) master_container {
    struct mixed_types main_data;
    union tagged_union optional_data;
    struct callback_container callbacks;
    struct anonymous_member position;
    struct bitfield_struct flags;
    struct tree_node *tree_root;
    struct callback_array callback_set;
    union pointer_union current_ptr;
    opaque_ptr hidden_data;
    
    /* Array of strings */
    string_type string_list[10];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

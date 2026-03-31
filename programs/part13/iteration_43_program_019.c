/* test-coverage.gt - Comprehensive type definitions for gengtype coverage */

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
typedef int (*another_callback)(const char *, float);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char *string_array[5];

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
    string_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    enum my_enum lang_enum;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    int_ptr ptr_field;
    
    /* TYPE_ARRAY */
    fixed_array array_field;
    
    /* TYPE_STRING */
    string_type name;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_SCALAR */
    scalar_int count;
    enum my_enum status;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* Nested struct pointer */
    struct plain_struct *plain_ptr;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Another GC-tracked struct with nested structures */
struct GTY(()) container_struct {
    /* TYPE_USER_STRUCT */
    struct user_struct user_data;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_specific *lang_ptr;
    
    /* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
    struct complex_struct *ptr_array[8];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Union containing different types */
    union {
        int int_val;
        float float_val;
        struct plain_struct *struct_ptr;
    } variant;
    
    /* Callback array */
    callback_type callbacks[4];
    
    /* String array */
    string_type strings[10];
};

/* Union with GTY marker */
union GTY(()) tagged_union {
    int tag;
    struct complex_struct *complex;
    struct container_struct *container;
    string_type str;
};

/* Type with bitfields (still TYPE_SCALAR) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int value : 16;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int type;
    string_type name;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *parent;
};

/* Structure with chain of pointers */
struct GTY(()) linked_item {
    int data;
    struct linked_item *GTY((skip)) next;
    struct linked_item *prev;
};

/* Mixed structure with all type kinds */
struct GTY(()) all_in_one {
    /* TYPE_SCALAR */
    int id;
    enum my_enum kind;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_POINTER */
    void *user_data;
    int *number_ptr;
    
    /* TYPE_ARRAY */
    float values[7];
    callback_type hooks[3];
    
    /* TYPE_STRUCT (embedded) */
    struct plain_struct embedded;
    
    /* TYPE_UNION */
    union my_union choice;
    
    /* TYPE_CALLBACK */
    int (*processor)(struct all_in_one *);
    
    /* Flexible array of strings */
    string_type tags[];
};

/* Additional pointer types for coverage */
typedef struct all_in_one *all_in_one_ptr;
typedef union tagged_union *union_ptr_t;

/* Array of function pointers */
typedef int (*func_array[5])(void);

/* Const pointer types */
typedef const struct complex_struct *const_complex_ptr;
typedef const int *const_int_ptr;

/* Opaque pointer type (TYPE_POINTER to TYPE_UNDEFINED) */
typedef struct opaque_struct *unknown_ptr;

/* Void pointer type */
typedef void *generic_ptr;

/* Structure with nested anonymous struct/union */
struct GTY(()) nested_anon {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int int_val;
        float float_val;
    } value;
    
    struct {
        const char *key;
        void *data;
    } entry;
};

/* Template-like structure with parameterized callback */
struct GTY(()) callback_container {
    void *context;
    void (*execute)(void *context, int param);
    int (*validate)(void *context, const char *input);
};

/* Ensure we have TYPE_NONE case covered (gcc_unreachable) */
/* This is the default case in the switch, should not be reachable */
/* with proper type classification */

/* Final structure that references everything */
struct GTY(()) master_struct {
    struct user_struct user;
    struct lang_specific lang;
    struct complex_struct complex;
    struct container_struct container;
    union tagged_union utag;
    struct bitfield_struct bits;
    struct tree_node tree;
    struct linked_item list;
    struct all_in_one all;
    struct nested_anon anon;
    struct callback_container callbacks;
    
    /* Array of various pointers */
    void *void_ptrs[5];
    struct opaque_struct *opaque_ptrs[3];
    callback_type func_ptrs[4];
    
    /* Multi-type union */
    union {
        int as_int;
        float as_float;
        void *as_ptr;
        struct plain_struct as_struct;
    } multi;
};

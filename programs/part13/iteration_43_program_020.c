/* test-coverage.gt - Comprehensive test file for gengtype coverage */

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
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

/* TYPE_STRUCT: Plain C struct without GTY markers */
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

/* A GC-tracked structure containing various types */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum status;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* Nested TYPE_STRUCT */
    struct plain_struct nested_struct;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct * GTY((skip)) user_data;
    
    /* Pointer to TYPE_LANG_STRUCT */
    struct lang_specific *lang_ptr;
};

/* Another structure with flexible array member */
struct GTY(()) flexible_struct {
    int length;
    /* TYPE_ARRAY (flexible/variable length) */
    char data[];
};

/* Union with GTY marker */
union GTY((desc ("%0.type"))) tagged_union {
    int type;
    struct complex_struct * GTY((tag ("0"))) cs;
    struct flexible_struct * GTY((tag ("1"))) fs;
};

/* Structure containing array of pointers */
struct GTY(()) pointer_array_struct {
    /* Array of TYPE_POINTER */
    void *ptr_array[8];
    
    /* Pointer to TYPE_ARRAY */
    int (*matrix_ptr)[4][4];
    
    /* Array of TYPE_CALLBACK */
    callback_type callbacks[3];
};

/* Structure for testing nested arrays */
struct GTY(()) nested_array_struct {
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Array of strings */
    const char *names[5];
    
    /* Array of structures */
    struct plain_struct items[10];
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct undefined_type *undefined_ptr;

/* Mix of typedefs for various types */
typedef union my_union union_alias;
typedef struct plain_struct struct_alias;
typedef int (*complex_callback)(struct complex_struct *, union my_union);

/* Global variable declarations with GC roots */
struct complex_struct * GTY((root)) global_complex;
struct flexible_struct * GTY((root)) global_flexible;

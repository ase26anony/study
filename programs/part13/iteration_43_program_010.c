/* test-coverage.gt - Test file to cover all TYPE_* cases in gengtype-state.cc */

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
typedef int (*another_callback)(const char*, void*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char char_array[256];

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
    void* ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char* name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int inner_data;
        float inner_float;
    } inner;
    
    /* Pointer to TYPE_UNION */
    union my_union* union_ptr;
    
    /* Array of TYPE_POINTER */
    void* ptr_array[3];
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another GTY structure with different combinations */
struct GTY(()) another_complex {
    /* Reference to TYPE_USER_STRUCT */
    struct user_struct* user_data;
    
    /* TYPE_ARRAY of strings */
    const char* strings[4];
    
    /* Nested anonymous union (TYPE_UNION) */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } variant;
    
    /* Pointer to callback (TYPE_POINTER to TYPE_CALLBACK) */
    callback_type* callback_ptr;
    
    /* Multi-dimensional array (TYPE_ARRAY) */
    int matrix[3][3];
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
    int* int_ptr;
    void** void_ptr_ptr;
    struct complex_struct* struct_ptr;
    callback_type func_ptr;
};

/* Structure with language-specific tag variations */
struct GTY((tag("GTY_ggc"))) ggc_struct {
    void* ggc_data;
    int ggc_counter;
};

struct GTY((tag("GTY_pch_none"))) pch_struct {
    int pch_value;
    char pch_name[32];
};

/* Array of structures */
struct GTY(()) array_element {
    int index;
    void* data;
};

typedef struct array_element element_array[10];

/* Structure with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int value : 8;
    unsigned int : 5; /* padding */
    unsigned int last_flag : 16;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Typedef for void (TYPE_UNDEFINED) */
typedef void void_type;

/* Structure containing a pointer to undefined type */
struct GTY(()) has_undefined {
    struct opaque_struct* opaque_ptr;
    struct never_defined* never_ptr;
};

/* Complex callback type with multiple parameters */
typedef int (*complex_callback)(struct complex_struct*, union my_union, int, ...);

/* Structure using the complex callback */
struct GTY(()) uses_complex_callback {
    complex_callback handler;
    void* context;
};

/* Union with array member */
union GTY(()) array_union {
    int as_ints[4];
    float as_floats[4];
    void* as_ptrs[4];
};

/* Structure with nested anonymous struct */
struct GTY(()) nested_anonymous {
    struct {
        int x;
        int y;
    } point;
    
    struct {
        int width;
        int height;
    } size;
};

/* Final catch-all structure containing references to all types */
struct GTY(()) master_struct {
    /* TYPE_STRUCT */
    struct plain_struct plain;
    
    /* TYPE_USER_STRUCT */
    struct user_struct* user;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* TYPE_POINTER */
    void* generic_pointer;
    
    /* TYPE_ARRAY */
    int numbers[100];
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific* lang;
    
    /* TYPE_SCALAR */
    int counter;
    float value;
    enum my_enum state;
    
    /* TYPE_STRING */
    const char* description;
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_UNDEFINED reference */
    struct opaque_struct* opaque;
    
    /* Complex nested types */
    struct complex_struct complex;
    struct another_complex* another;
    union pointer_union pointers;
    struct ggc_struct* ggc;
    struct pch_struct pch;
    struct array_element elements[5];
    struct bitfield_struct flags;
    struct has_undefined* undefined_ref;
    struct uses_complex_callback callback_user;
    union array_union array_data;
    struct nested_anonymous nested;
    
    /* Flexible array of strings */
    const char* tags[];
};

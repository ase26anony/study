/* Test coverage file for gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char* string_array[5];

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    float c;
};

/* TYPE_STRUCT: Plain C struct (no GTY marker) */
struct plain_struct {
    int field1;
    char field2;
    float field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
    callback_type callback;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* lang_data;
    enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER field */
    struct opaque_struct* opaque_ptr_field;
    
    /* TYPE_ARRAY field */
    int int_array[20];
    
    /* TYPE_UNION field */
    union my_union union_field;
    
    /* TYPE_STRING field */
    const char* name;
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int inner_field;
        float inner_float;
    } inner;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct* user_data;
    
    /* Array of TYPE_POINTER */
    void* ptr_array[8];
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another union with GTY marker */
union GTY(()) tagged_union {
    int as_int;
    float as_float;
    void* as_ptr;
    struct complex_nested* as_complex;
};

/* Structure containing array of function pointers */
struct GTY(()) callback_container {
    /* TYPE_ARRAY of TYPE_CALLBACK */
    callback_type callbacks[4];
    
    /* TYPE_SCALAR */
    enum my_enum current_state;
    
    /* TYPE_STRING */
    const char* description;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Void pointer typedef (could be considered TYPE_UNDEFINED in some contexts) */
typedef void generic_void;

/* Structure with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 8;
};

/* Array of unions */
union GTY(()) small_union {
    char byte;
    int word;
} union_array[100];

/* Structure with nested anonymous struct/union */
struct GTY(()) anonymous_members {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } data;
};

/* Const pointer types */
typedef int* const const_int_ptr;
typedef const char* const const_string_ptr;

/* Multi-dimensional array */
int matrix[3][4];

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Complex callback with structure parameter */
typedef void (*complex_callback)(struct complex_nested*, int);

/* Final structure that references many types */
struct GTY(()) master_type {
    /* Reference to TYPE_LANG_STRUCT */
    struct lang_specific* lang_data;
    
    /* TYPE_USER_STRUCT */
    struct user_struct user_instance;
    
    /* TYPE_UNION */
    union my_union current_union;
    
    /* TYPE_ARRAY of TYPE_STRUCT */
    struct plain_struct plain_array[5];
    
    /* TYPE_POINTER to TYPE_CALLBACK */
    complex_callback* callback_ptr;
    
    /* TYPE_STRING array */
    const char* messages[3];
    
    /* TYPE_SCALAR bitfield */
    unsigned int options : 4;
    
    /* Padding to ensure alignment */
    char padding[4];
};

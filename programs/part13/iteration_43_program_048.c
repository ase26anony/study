/* test-coverage.gt - Test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* void_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    callback_type callback;
};

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    string_type lang_name;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_struct {
    /* TYPE_POINTER within struct */
    void_ptr ptr_field;
    
    /* TYPE_ARRAY within struct */
    fixed_array array_field;
    
    /* TYPE_UNION within struct */
    union my_union union_field;
    
    /* TYPE_STRING within struct */
    string_type name;
    
    /* TYPE_CALLBACK within struct */
    callback_type handler;
    
    /* TYPE_SCALAR within struct */
    scalar_int id;
    
    /* Flexible array member */
    char GTY((length("strlen(%h.data) + 1"))) flexible_array[];
};

/* Another GTY-marked struct with nested types */
struct GTY(()) container {
    /* Pointer to another GTY type */
    struct complex_struct* GTY((skip)) complex_ptr;
    
    /* Array of pointers */
    void* GTY((length("10"))) ptr_array[10];
    
    /* Nested struct */
    struct GTY(()) nested {
        int x;
        int y;
    } nested_field;
    
    /* Union with GTY marker */
    union GTY(()) tagged_union {
        int as_int;
        void* as_ptr;
        struct complex_struct* as_struct;
    } tag_union;
};

/* TYPE_NONE should not be directly triggerable as it's for internal error */
/* gcc_unreachable() will be called if TYPE_NONE is encountered */

/* Additional test cases to ensure edge cases are covered */

/* Enum type (scalar) */
typedef enum color { RED, GREEN, BLUE } color_t;

/* Pointer to callback */
typedef callback_type* callback_ptr;

/* Array of structs */
struct plain_struct struct_array[5];

/* Struct containing array of unions */
struct GTY(()) union_container {
    union my_union unions[3];
    int count;
};

/* Void pointer typedef */
typedef void* generic_pointer;

/* Const pointer */
typedef const int* const_int_ptr;

/* Struct with bitfield (scalar) */
struct with_bitfield {
    unsigned int flag:1;
    unsigned int value:7;
};

/* Opaque pointer type */
typedef struct opaque_struct* opaque_ptr;

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
};

/* Union containing struct */
union container_union {
    struct complex_struct cs;
    struct container c;
    int data[20];
};

/* Typedef for a function type (different from function pointer) */
typedef int func_type(double);

/* Multiple levels of pointer indirection */
typedef int*** triple_ptr;

/* Array of function pointers */
callback_type callback_array[5];

/* Struct with anonymous union */
struct GTY(()) with_anon_union {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    };
};

/* Complete the forward declaration with actual definition */
struct opaque_struct {
    void* data;
    int refcount;
};

/* Ensure all basic types are used in GTY contexts */
struct GTY(()) master_container {
    /* Include one of each type */
    struct plain_struct plain;          /* TYPE_STRUCT */
    struct user_struct* user;           /* TYPE_POINTER to TYPE_USER_STRUCT */
    union my_union union_data;          /* TYPE_UNION */
    fixed_array numbers;                /* TYPE_ARRAY */
    struct lang_specific* lang;         /* TYPE_POINTER to TYPE_LANG_STRUCT */
    scalar_int counter;                 /* TYPE_SCALAR */
    string_type message;                /* TYPE_STRING */
    callback_type notify;               /* TYPE_CALLBACK */
    struct opaque_struct* opaque;       /* TYPE_POINTER to TYPE_UNDEFINED? */
    
    /* Nested container */
    struct container* contents;
};

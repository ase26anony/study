/* Test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char*, float);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
typedef char char_array[20];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    float field2;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    float c;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* lang_ptr;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_POINTER */
    void* GTY((skip)) ptr;
    
    /* TYPE_STRING */
    const char* GTY((length("strlen(%h.name) + 1"))) name;
    
    /* TYPE_ARRAY (fixed size) */
    int scores[5];
    
    /* TYPE_UNION */
    union {
        int int_val;
        float float_val;
    } value;
    
    /* TYPE_CALLBACK */
    callback_type GTY((skip)) callback;
    
    /* Pointer to another GC-tracked struct */
    struct GTY(()) inner_struct* inner;
    
    /* Flexible array member */
    char GTY((length("flex_len"))) flexible_array[];
};

/* Another GC-tracked struct for TYPE_STRUCT case */
struct GTY(()) another_struct {
    scalar_int id;
    string_type description;
    
    /* Nested union */
    union nested_union {
        int x;
        struct GTY(()) complex_struct* cs;
    } data;
    
    /* Array of pointers */
    void* GTY((length("ptr_count"))) *ptr_array;
};

/* Union containing various types */
union GTY(()) complex_union {
    struct GTY(()) complex_struct* cs;
    struct GTY(()) another_struct* as;
    int number;
    const char* text;
};

/* Type with callback field */
struct GTY(()) callback_container {
    callback_type handler;
    void* GTY((skip)) user_data;
};

/* Array of structs */
struct GTY(()) array_element {
    int id;
    float value;
};

typedef struct GTY(()) array_element element_array[50];

/* Forward declaration for TYPE_UNDEFINED in a different context */
struct undefined_type;

/* Struct that references undefined type */
struct GTY(()) references_undefined {
    /* This will be TYPE_UNDEFINED */
    struct undefined_type* GTY((skip)) undefined_ptr;
};

/* Enum as scalar */
typedef enum color { RED, GREEN, BLUE } color_t;

/* Struct with enum field */
struct GTY(()) enum_container {
    color_t current_color;
    enum my_enum status;
};

/* Multiple levels of pointer indirection */
typedef struct GTY(()) complex_struct*** triple_ptr;

/* Void pointer type */
typedef void* void_ptr;

/* Const pointer */
typedef const int* const_int_ptr;

/* Struct with array of strings */
struct GTY(()) string_array_container {
    const char* GTY((length("count"))) strings[10];
    int count;
};

/* Union with array */
union array_union {
    int ints[4];
    float floats[4];
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int defined_now;
    void* data;
};

/* Ensure all types are referenced to prevent optimization */
void GTY((user)) reference_all_types() {
    /* This function doesn't need a body, it just ensures types are processed */
}

/* test-coverage.gt - Comprehensive test file for gengtype coverage */

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
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char* string_array[5];

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    double c;
};

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    double field2;
    char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GTY struct containing various type combinations */
struct GTY(()) complex_type {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_STRING */
    const char* GTY((skip)) name;
    
    /* TYPE_POINTER */
    struct opaque_struct* GTY((skip)) opaque_ref;
    
    /* TYPE_ARRAY */
    int scores[5];
    
    /* TYPE_UNION (embedded) */
    union {
        int int_val;
        void* ptr_val;
    } GTY((desc("0"))) value;
    
    /* TYPE_CALLBACK */
    callback_type GTY((skip)) handler;
    
    /* TYPE_POINTER to another GTY type */
    struct user_struct* GTY((tag("USER"))) user_data;
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another GTY union with nested structures */
union GTY((desc("%1.type"))) tagged_union {
    struct GTY((tag("0"))) {
        int type;
        int int_value;
    } int_data;
    
    struct GTY((tag("1"))) {
        int type;
        const char* string_value;
    } string_data;
    
    struct GTY((tag("2"))) {
        int type;
        void* GTY((skip)) ptr_value;
        int array[3];
    } complex_data;
};

/* TYPE_ARRAY of pointers */
typedef struct complex_type* ptr_array[10];

/* TYPE_STRUCT with callback array */
struct GTY(()) callback_container {
    callback_type handlers[3];
    int priorities[3];
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* A structure that uses the never-defined type (pointer only) */
struct GTY(()) uses_undefined {
    struct never_defined* GTY((skip)) undefined_ptr;
    int valid;
};

/* Mixed GTY and non-GTY types in a union */
union mixed_union {
    struct GTY((tag("0"))) {
        int tag;
        void* data;
    } gty_part;
    
    struct {
        int tag;
        long data;
    } non_gty_part;
};

/* TYPE_ARRAY with variable length (in a struct) */
struct GTY(()) var_array_struct {
    int length;
    int elements[1];  /* Variable length array */
};

/* String constants that might be processed */
extern const char* const string_constants[] = {
    "test1",
    "test2",
    "test3"
};

/* Global variables with various types */
struct complex_type* GTY((root)) global_complex = NULL;
callback_type global_callback = NULL;
int global_array[5] = {1, 2, 3, 4, 5};

/* Nested pointer structure */
struct GTY(()) nested_pointers {
    struct nested_pointers* GTY((skip)) next;
    struct nested_pointers** GTY((skip)) prev_ptr;
    void* data;
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int defined_now;
    void* data;
};

/* A final structure that references many of the above types */
struct GTY(()) master_type {
    /* TYPE_STRUCT reference */
    struct plain_struct plain;
    
    /* TYPE_USER_STRUCT */
    struct user_struct* user;
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific* lang;
    
    /* TYPE_UNION */
    union my_union u;
    
    /* TYPE_ARRAY of different types */
    int ints[10];
    struct complex_type* objects[5];
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_POINTER chain */
    struct nested_pointers* chain;
    
    /* TYPE_STRING array */
    const char* messages[3];
    
    /* TYPE_SCALAR bitfield */
    unsigned int flags : 4;
    
    /* Flexible array of strings */
    const char* extra_strings[];
};

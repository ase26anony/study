/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
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
typedef int fixed_array_type[10];
typedef char string_array[5][20];

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    callback_type func_ptr;
};

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user-defined structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
    string_type name;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* lang_data;
    enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER within struct */
    struct opaque_struct* GTY((skip)) opaque_ptr;
    
    /* TYPE_ARRAY within struct */
    fixed_array_type numbers;
    
    /* TYPE_UNION within struct */
    union my_union data_union;
    
    /* TYPE_STRING within struct */
    string_type description;
    
    /* TYPE_CALLBACK within struct */
    callback_type notify;
    
    /* TYPE_SCALAR within struct */
    scalar_int count;
    enum my_enum status;
    
    /* Flexible array member (variable-length array) */
    int GTY((length("flex_count"))) flexible_array[];
    
    /* For tracking flexible array length */
    int flex_count;
};

/* Another GTY structure with pointer chain */
struct GTY(()) pointer_chain {
    struct complex_nested* GTY((tag("0"))) nested;
    struct pointer_chain* GTY((tag("1"))) next;
    user_struct* GTY((tag("2"))) user_data;
};

/* Union with GTY marker */
union GTY((desc("%0.type"))) tagged_union {
    int type;
    struct plain_struct plain;
    struct complex_nested* complex;
    lang_specific* lang;
};

/* Array of pointers */
typedef struct complex_nested* GTY((length("array_len"))) nested_ptr_array[];
extern int array_len;

/* Callback structure */
struct GTY(()) callback_container {
    callback_type pre_process;
    callback_type post_process;
    void* GTY((skip)) user_context;
};

/* Mixed structure with all types */
struct GTY(()) all_types_mix {
    /* SCALAR */
    int id;
    float value;
    enum my_enum mode;
    
    /* POINTER */
    void* data;
    int_ptr int_pointer;
    
    /* ARRAY */
    char name[50];
    fixed_array_type scores;
    
    /* STRING */
    string_type message;
    
    /* CALLBACK */
    another_callback processor;
    
    /* STRUCT */
    struct plain_struct plain;
    
    /* UNION */
    union my_union variant;
    
    /* USER_STRUCT */
    user_struct* user;
    
    /* LANG_STRUCT */
    lang_specific* lang;
    
    /* Nested complex type */
    struct complex_nested* nested;
    
    /* Pointer to opaque (UNDEFINED) */
    struct opaque_struct* GTY((skip)) opaque;
};

/* Global variables for testing */
extern struct all_types_mix GTY((tag("GLOBAL"))) global_mix;
extern nested_ptr_array GTY((tag("PTR_ARRAY"))) global_ptr_array;
extern string_type GTY((tag("STRINGS"))) global_strings[];
extern int global_string_count;

/* Template-like structure for parameterized types */
struct GTY(()) template_container {
    void* GTY((skip)) data;
    size_t GTY((skip)) size;
    callback_type GTY((skip)) destructor;
};

/* End of test-coverage.gt */

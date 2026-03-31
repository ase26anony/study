/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, void*);
typedef int (*another_callback)(const char*);

/* TYPE_POINTER: Various pointer types */
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

/* TYPE_USER_STRUCT: User-defined GC structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
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
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_UNION */
    union my_union union_field;
    
    /* Pointer to another GC struct (TYPE_POINTER to TYPE_STRUCT) */
    struct GTY(()) inner_struct *inner;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Another GC struct for more coverage */
struct GTY(()) inner_struct {
    int value;
    string_type description;
    
    /* Nested union */
    union {
        int int_val;
        float float_val;
    } nested_union;
    
    /* Array of pointers */
    void * GTY((length("%0.value"))) *ptr_array;
};

/* Union with GTY marker */
union GTY((desc("%1.type"))) tagged_union {
    int type;
    struct complex_struct * GTY((tag("0"))) cs;
    struct inner_struct * GTY((tag("1"))) is;
};

/* Array of structures */
typedef struct GTY(()) complex_struct complex_array[3];

/* Pointer to array */
typedef complex_array *array_ptr;

/* Struct with callback array */
struct GTY(()) callback_container {
    /* Array of callbacks */
    callback_type GTY((length("%0.callback_count"))) callbacks[];
    int callback_count;
};

/* Opaque pointer type */
typedef struct opaque_struct * GTY((atomic)) atomic_opaque_ptr;

/* Struct with string array */
struct GTY(()) string_container {
    /* Array of strings */
    const char * GTY((length("%0.str_count"))) *strings;
    int str_count;
};

/* Test TYPE_NONE - This should not appear in normal parsing,
   but we include various edge cases */

/* Void pointer - special case */
typedef void *generic_pointer;

/* Const pointer to const */
typedef const int * const const_int_ptr;

/* Struct with bitfield (scalar) */
struct bitfield_struct {
    unsigned int flag:1;
    unsigned int value:7;
};

/* Anonymous struct/union */
struct GTY(()) anonymous_container {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } data;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int code;
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
};

/* Chain of structures for testing traversal */
struct GTY(()) list_node {
    void *data;
    struct list_node *next;
};

/* Union with array */
union array_union {
    int ints[4];
    float floats[4];
    char chars[16];
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int defined_now;
    void *data;
};

/* Additional scalar typedefs */
typedef unsigned long long uint64;
typedef signed char int8;

/* Enum with explicit values */
enum error_codes {
    ERR_NONE = 0,
    ERR_INVALID = 1,
    ERR_MEMORY = 2,
    ERR_LAST
};

/* Mixed declarations in a single struct */
struct GTY(()) mixed_types {
    /* All in one place */
    scalar_int s_int;          /* TYPE_SCALAR */
    string_type s_string;      /* TYPE_STRING */
    callback_type s_callback;  /* TYPE_CALLBACK */
    int_ptr s_pointer;         /* TYPE_POINTER */
    fixed_array s_array;       /* TYPE_ARRAY */
    struct plain_struct s_struct; /* TYPE_STRUCT */
    union my_union s_union;    /* TYPE_UNION */
    
    /* For variable length array */
    int * GTY((length("%0.vla_length"))) vla;
    int vla_length;
};

/* Final check: ensure we have at least one instance of each type kind */
/*
  TYPE_UNDEFINED: struct opaque_struct (forward declaration)
  TYPE_SCALAR: int, float, enum, typedefs
  TYPE_STRING: const char*
  TYPE_CALLBACK: function pointer typedefs
  TYPE_POINTER: various pointer types
  TYPE_ARRAY: fixed arrays, flexible arrays
  TYPE_STRUCT: plain_struct
  TYPE_USER_STRUCT: user_struct
  TYPE_UNION: my_union, anonymous unions
  TYPE_LANG_STRUCT: lang_specific
*/

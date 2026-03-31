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
typedef int (*another_callback)(const char*, float);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char* string_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void * GTY((skip)) data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void* c;
    const char* d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void* GTY((tag("LANG"))) lang_pointer;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER within struct */
    struct plain_struct* GTY((skip)) plain_ptr;
    
    /* TYPE_ARRAY within struct */
    int GTY((skip)) numbers[5];
    
    /* TYPE_UNION within struct */
    union my_union GTY((skip)) data_union;
    
    /* TYPE_CALLBACK within struct */
    callback_type GTY((skip)) callback;
    
    /* TYPE_STRING within struct */
    const char* GTY((skip)) name;
    
    /* TYPE_SCALAR within struct */
    enum my_enum GTY((skip)) status;
    
    /* Flexible array member (TYPE_ARRAY) */
    int GTY((length ("%0.flex_length"))) flexible_array[];
    
    /* For tracking flexible array length */
    size_t flex_length;
};

/* Another GC-tracked structure with various field types */
struct GTY(()) another_gc_struct {
    /* Pointer to another GC structure */
    struct complex_nested* GTY((skip)) nested;
    
    /* Array of pointers */
    void* GTY((skip)) ptr_array[8];
    
    /* String field */
    const char* GTY((skip)) description;
    
    /* Scalar fields */
    int GTY((skip)) count;
    float GTY((skip)) value;
    
    /* Union field */
    union my_union GTY((skip)) choice;
    
    /* Callback field */
    another_callback GTY((skip)) handler;
};

/* Union containing GC pointers */
union GTY(()) gc_union {
    struct another_gc_struct* GTY((skip)) gc_ptr;
    struct user_struct* GTY((skip)) user_ptr;
    int GTY((skip)) scalar;
    const char* GTY((skip)) str;
};

/* Structure with nested anonymous union */
struct GTY(()) with_anonymous_union {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
        const char* str_val;
    } GTY((skip)) data;
};

/* Structure containing array of structures */
struct GTY(()) array_of_structs {
    struct plain_struct GTY((skip)) items[3];
    int count;
};

/* Typedef for a pointer to function returning pointer */
typedef struct complex_nested* (*factory_func)(int, const char*);

/* Structure using the factory function type */
struct GTY(()) with_factory {
    factory_func GTY((skip)) create;
    void (* GTY((skip)) destroy)(struct complex_nested*);
};

/* Test case for TYPE_NONE - this should not appear in normal parsing,
   but we include various edge cases to ensure thorough coverage */

/* Multiple levels of indirection */
typedef int*** triple_ptr;

/* Const pointer to const data */
typedef const int* const const_int_ptr;

/* Array of function pointers */
typedef int (*func_ptr_array[5])(void);

/* Structure with bitfield (scalar type) */
struct with_bitfield {
    unsigned int flag:1;
    unsigned int value:7;
    unsigned int padding:24;
};

/* Volatile and restrict qualified pointers */
typedef volatile int* volatile_ptr;
typedef int* restrict restrict_ptr;

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int hidden_data;
    void* secret_ptr;
};

/* Final structure that references almost everything */
struct GTY(()) master_struct {
    /* Reference to user struct */
    struct user_struct* GTY((skip)) user;
    
    /* Reference to lang struct */
    struct lang_specific* GTY((skip)) lang;
    
    /* Array of unions */
    union my_union GTY((skip)) union_array[4];
    
    /* Matrix (2D array) */
    int GTY((skip)) matrix[3][3];
    
    /* Pointer to array */
    int (* GTY((skip)) ptr_to_array)[10];
    
    /* Reference to self (recursive pointer) */
    struct master_struct* GTY((skip)) next;
    
    /* For callback testing */
    callback_type GTY((skip)) on_event;
    
    /* String literal pointer */
    const char* GTY((skip)) message;
    
    /* Scalar types */
    scalar_int GTY((skip)) id;
    scalar_float GTY((skip)) weight;
    enum my_enum GTY((skip)) state;
};

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
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char *string_array[5];

/* TYPE_UNION: Plain union */
union my_union {
    int a;
    void *b;
    float c;
};

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* GTY struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_ARRAY (fixed size) */
    int numbers[5];
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* TYPE_POINTER to another GTY struct */
    struct GTY(()) inner_struct *inner;
    
    /* TYPE_ARRAY of pointers */
    struct plain_struct *struct_array[3];
};

/* Another GTY struct for additional coverage */
struct GTY(()) inner_struct {
    int value;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_POINTER to callback */
    callback_type *callback_ptr;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Union with GTY marker */
union GTY(()) tagged_union {
    int as_int;
    void *as_ptr;
    struct complex_struct *as_struct;
};

/* Array of unions */
union my_union union_array[10];

/* Struct containing array of function pointers */
struct GTY(()) callback_container {
    /* TYPE_ARRAY of TYPE_CALLBACK */
    callback_type callbacks[5];
    
    /* TYPE_SCALAR */
    enum my_enum current_state;
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct undefined_type *undefined_ptr;

/* Additional scalar typedefs */
typedef unsigned long long uint64;
typedef signed char int8;

/* Struct with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int count : 4;
};

/* Nested struct/union combination */
struct GTY(()) outer_container {
    /* Anonymous union */
    union {
        int x;
        float y;
    } anonymous_data;
    
    /* Pointer to array */
    int (*matrix_ptr)[4][4];
    
    /* Multi-dimensional array */
    char buffer[10][20];
};

/* For TYPE_LANG_STRUCT variations */
struct GTY((tag("LANG"), chain_next("%h.next"), chain_prev("%h.prev"))) linked_lang_struct {
    int data;
    struct linked_lang_struct *next;
    struct linked_lang_struct *prev;
};

/* Callback with complex signature */
typedef struct complex_struct* (*factory_callback)(int, const char*, callback_type);

/* Struct using the complex callback */
struct GTY(()) factory {
    factory_callback create;
    void (*destroy)(struct complex_struct*);
};

/* Additional coverage for edge cases */

/* Const pointer to const */
typedef const struct complex_struct * const const_complex_ptr;

/* Array of const pointers */
typedef const char * const const_string_array[5];

/* Volatile scalar */
typedef volatile int volatile_int;

/* Struct with all basic types */
struct GTY(()) all_types {
    /* SCALAR types */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    enum my_enum e;
    
    /* POINTER types */
    void *vp;
    int *ip;
    const char *cp;
    
    /* ARRAY types */
    int ai[5];
    char as[10];
    
    /* Nested struct */
    struct inner_struct nested;
    
    /* Union */
    union my_union u;
    
    /* Callback */
    callback_type cb;
};

/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc */

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

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

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
    callback_type callback;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    enum my_enum lang_enum;
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
    enum my_enum state;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_POINTER to another GTY struct */
    struct GTY(()) inner_struct *inner;
    
    /* TYPE_ARRAY of pointers */
    void *ptr_array[8];
};

/* Another GTY struct for TYPE_POINTER references */
struct GTY(()) inner_struct {
    int value;
    string_type description;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Union with GTY marker */
union GTY(()) tagged_union {
    struct complex_struct *as_struct;
    struct inner_struct *as_inner;
    int as_int;
    void *as_ptr;
};

/* Struct containing array of structs */
struct GTY(()) array_container {
    /* TYPE_ARRAY of structs */
    struct inner_struct elements[4];
    
    /* Variable length array of pointers */
    struct complex_struct *var_array[0];
};

/* Opaque pointer type for TYPE_POINTER */
typedef struct complex_struct *complex_ptr;

/* Mixed struct with nested anonymous union */
struct GTY(()) mixed_types {
    int type;
    
    union {
        /* TYPE_STRING */
        const char *str;
        
        /* TYPE_POINTER */
        void *ptr;
        
        /* TYPE_SCALAR */
        int num;
        
        /* TYPE_CALLBACK */
        callback_type cb;
    } GTY((desc ("%0.type"))) data;
};

/* Struct with callback field that takes another callback */
struct GTY(()) callback_container {
    /* TYPE_CALLBACK that returns a callback */
    callback_type (*get_callback)(int);
    
    /* TYPE_CALLBACK that uses string type */
    int (*process_string)(string_type);
};

/* Additional undefined type reference */
struct another_undefined;

/* Pointer to undefined type */
typedef struct another_undefined *undefined_ptr;

/* Array of undefined pointers */
extern struct another_undefined *undefined_array[10];

/* Struct with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Complete the forward declaration to avoid warnings */
struct opaque_struct {
    int defined_later;
    void *some_pointer;
};

/* Another language-specific struct with different tag */
struct GTY((tag("OTHER_LANG"))) other_lang_struct {
    int other_field;
    union my_union other_union;
};

/* Typedef for a function type (TYPE_CALLBACK) */
typedef void (*simple_func)(void);

/* Struct containing all basic types */
struct GTY(()) all_types_struct {
    /* TYPE_SCALAR */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    enum my_enum e;
    
    /* TYPE_POINTER */
    char *cp;
    int *ip;
    void *vp;
    
    /* TYPE_STRING */
    const char *str;
    
    /* TYPE_ARRAY */
    int arr[7];
    char strarr[3][50];
    
    /* TYPE_CALLBACK */
    callback_type cb1;
    simple_func cb2;
    
    /* TYPE_UNION */
    union my_union u;
    
    /* Nested structs */
    struct plain_struct ps;
    struct inner_struct *isp;
};

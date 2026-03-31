/* Test coverage file for gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef unsigned int scalar_uint;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char*, void*);

/* TYPE_ENUM (handled as TYPE_SCALAR) */
enum my_enum {
    E1,
    E2,
    E3
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
    int id;
    enum my_enum kind;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    const char *d;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct plain_struct* struct_ptr;
typedef union my_union* union_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char* string_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    callback_type lang_callback;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER field */
    struct plain_struct *plain_ptr;
    
    /* TYPE_UNION field */
    union my_union u;
    
    /* TYPE_ARRAY field */
    int numbers[20];
    
    /* TYPE_STRING field */
    const char *name;
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* TYPE_USER_STRUCT field */
    struct user_struct *user_data;
    
    /* TYPE_LANG_STRUCT field */
    struct lang_specific *lang_data;
    
    /* Flexible array member (variable-length array) */
    int flexible_array[];
};

/* Another GTY structure with pointer chain */
struct GTY(()) pointer_chain {
    struct pointer_chain *next;  /* TYPE_POINTER to same type */
    struct complex_nested *data; /* TYPE_POINTER to another GTY struct */
    int value;
};

/* Union containing various types */
union GTY(()) mixed_union {
    struct plain_struct ps;
    struct user_struct *us;  /* TYPE_POINTER */
    int numbers[5];          /* TYPE_ARRAY */
    callback_type cb;        /* TYPE_CALLBACK */
};

/* Structure with array of pointers */
struct GTY(()) array_of_pointers {
    struct plain_struct *items[10];  /* TYPE_ARRAY of TYPE_POINTER */
    callback_type callbacks[3];      /* TYPE_ARRAY of TYPE_CALLBACK */
    string_type strings[5];          /* TYPE_ARRAY of TYPE_STRING */
};

/* Opaque pointer type */
typedef struct opaque_struct *opaque_ptr;

/* Structure with nested anonymous union */
struct GTY(()) with_anon_union {
    int type;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
        const char *str_val;
    } value;
};

/* Template-like structure (C doesn't have templates, but we can simulate) */
#define DECLARE_CONTAINER(TYPE, NAME) \
struct GTY(()) NAME { \
    TYPE *items; \
    int count; \
    int capacity; \
}

/* Instantiate for different types */
DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(struct plain_struct, struct_container);
DECLARE_CONTAINER(callback_type, callback_container);

/* Structure with bitfields (scalar type) */
struct GTY(()) with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int value : 16;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int code;
    union {
        int int_val;
        float float_val;
        const char *str_val;
        struct tree_node *child;  /* TYPE_POINTER to same type */
    } GTY((desc("code"))) u;
    struct tree_node *next;  /* TYPE_POINTER to same type */
};

/* Structure with function pointer table */
struct GTY(()) vtable {
    callback_type func1;
    int (*func2)(float, void*);
    void (*func3)(struct complex_nested*);
};

/* Global variable declarations (not types, but might be processed) */
extern int global_scalar;
extern const char *global_string;
extern struct plain_struct *global_struct_ptr;
extern callback_type global_callback;

/* Typedef chain */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 my_int3;

/* Void pointer type */
typedef void *generic_ptr;

/* Constant pointer types */
typedef int *const const_int_ptr;
typedef const struct plain_struct *const const_struct_ptr;

/* Array of arrays */
typedef int matrix[3][4];
typedef const char *string_matrix[2][3];

/* Structure with all basic scalar types */
struct GTY(()) all_scalars {
    char c;
    signed char sc;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    _Bool b;
};

/* Final catch-all structure that references everything */
struct GTY(()) master_type {
    /* TYPE_STRUCT */
    struct plain_struct plain;
    
    /* TYPE_USER_STRUCT */
    struct user_struct *user;
    
    /* TYPE_UNION */
    union my_union u;
    
    /* TYPE_POINTER */
    void *ptr;
    
    /* TYPE_ARRAY */
    int array[10];
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific *lang;
    
    /* TYPE_SCALAR */
    int scalar;
    
    /* TYPE_STRING */
    const char *string;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* Chain of pointers */
    struct master_type *next;
    
    /* Array of various types */
    struct plain_struct *struct_array[5];
    callback_type callback_array[3];
    const char *string_array[4];
    
    /* Flexible array */
    int flexible[];
};

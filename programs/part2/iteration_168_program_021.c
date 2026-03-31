#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
void undefined_function_prototype(void*);  /* void pointer parameter */

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* Function with scalar parameters */
int scalar_function(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals and string pointers */
const char* global_string_ptr = "global string literal";
char string_array[] = "string array initializer";
const char* const_string_array[] = {"string1", "string2", "string3"};

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct types with mixed members */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    char name[50];
    struct simple_struct* nested;
    double values[10];
    void* data;
};

struct packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedefs for struct types */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct complex_struct complex_t;

typedef struct {
    point_t start;
    point_t end;
    char label[20];
} line_t;

/* ==================== TYPE_UNION ==================== */
/* Union types */
union simple_union {
    int i;
    float f;
    char c;
    void* ptr;
};

union data_union {
    long long_value;
    double double_value;
    char string_value[16];
    struct simple_struct struct_value;
};

/* Transparent union (GCC-specific) */
typedef union {
    int* int_ptr;
    float* float_ptr;
} transparent_union_t __attribute__((transparent_union));

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types */
int* int_ptr;
float* float_ptr;
struct simple_struct* struct_ptr;
union simple_union* union_ptr;
point_t* user_struct_ptr;
void* void_ptr;
int** double_ptr;
char* string_ptr_array[5];

/* Function pointer declaration */
int (*func_ptr)(int, int);

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
point_t user_struct_array[3][4];
int* pointer_array[8];
char* string_array_2d[3][10];
int three_d_array[2][3][4];

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (callbacks) */
typedef int (*int_callback)(int);
typedef void (*void_callback)(void);
typedef struct simple_struct* (*struct_callback)(int, char*);
typedef void (*complex_callback)(int, float, struct complex_struct*, union data_union);

/* Struct containing callback members */
struct callback_container {
    int_callback int_handler;
    void_callback void_handler;
    complex_callback complex_handler;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;

/* Dummy struct that might be recognized by gengtype */
struct lang_type {
    struct tree_node* base;
    int lang_specific_data;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */
typedef struct nested_container {
    /* Struct containing array of pointers to unions */
    union data_union* union_ptr_array[8];
    
    /* Pointer to struct containing callback */
    struct callback_container* cb_container;
    
    /* Multi-dimensional array of structs */
    point_t point_grid[5][5];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container* (*get_cb_container)(int id);
    
    /* Nested anonymous struct */
    struct {
        int depth;
        struct nested_container* next_level;
    } nested_info;
} nested_container_t;

/* Function pointer returning pointer to struct containing callback members */
typedef struct callback_container* (*container_factory)(int, char*);

/* Union containing struct with array of function pointers */
union mega_union {
    struct {
        int count;
        int_callback callbacks[10];
        nested_container_t* containers[5];
    } callback_suite;
    
    struct {
        char* names[20];
        point_t* points;
        void (*operations[3])(void);
    } data_suite;
};

/* ==================== GCC ATTRIBUTES ==================== */
/* Types with GCC-specific attributes */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

struct packed_with_attributes {
    unsigned char type;
    unsigned int data;
    char name[8];
} __attribute__((packed, aligned(4)));

/* Transparent union attribute */
typedef union {
    int* as_int;
    float* as_float;
    struct simple_struct* as_struct;
} multi_ptr_t __attribute__((transparent_union));

/* ==================== COMPLEX FUNCTION SIGNATURES ==================== */
/* Complex function pointer with nested types */
typedef nested_container_t* (*complex_factory)(
    int mode,
    const char* name,
    point_t* points,
    int count,
    void (*progress_callback)(int, const char*)
);

/* Callback that takes another callback as parameter */
typedef void (*callback_dispatcher)(
    int event_type,
    void* data,
    void (*event_handler)(int, void*)
);

/* ==================== FINAL COMPLEX TYPE ==================== */
/* Ultimate nested type combining everything */
typedef struct ultimate_type {
    /* Scalar members */
    int id;
    float priority;
    
    /* String member */
    char description[256];
    
    /* Struct member */
    struct complex_struct base;
    
    /* User struct member */
    point_t position;
    
    /* Union member */
    union mega_union data;
    
    /* Pointer members */
    nested_container_t* container;
    complex_factory factory;
    
    /* Array members */
    int_callback handlers[5];
    point_t trajectory[100];
    
    /* Callback member */
    callback_dispatcher dispatcher;
    
    /* Language struct pointer */
    struct lang_type* lang_info;
    
    /* Attribute for alignment */
    unsigned char raw_data[64];
} __attribute__((aligned(32))) ultimate_type_t;

#endif /* TEST_TYPES_H */

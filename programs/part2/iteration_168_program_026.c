#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct undefined_t;
void undefined_function(void* param); /* void pointer parameter */

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
const char* global_string = "Hello, World!";
char string_array[] = "Test string";
const char* const_string_array[] = {"str1", "str2", "str3"};

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct definitions */
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

struct nested_struct {
    struct {
        int a;
        int b;
    } inner;
    struct complex_struct* complex_ptr;
};

/* Struct with GCC attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef structs */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct complex_struct complex_t;

typedef struct {
    point_t start;
    point_t end;
    double length;
} line_t;

/* ==================== TYPE_UNION ==================== */
/* Union definitions */
union simple_union {
    int i;
    float f;
    char c;
    void* p;
};

union data_union {
    struct {
        int type;
        char name[20];
    } metadata;
    struct {
        double x;
        double y;
        double z;
    } coordinates;
    unsigned char raw_data[32];
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    float* float_ptr;
    void* void_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
int* int_ptr;
float* float_ptr;
double* double_ptr;
char** string_ptr_ptr;
struct simple_struct* struct_ptr;
union simple_union* union_ptr;
point_t* typedef_ptr;
void (*func_ptr)(void);
int (*array_of_func_ptrs[5])(void);

/* Pointer to pointer chain */
int*** triple_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of various types and dimensions */
int int_array[10];
float float_array[5][5];
double double_3d_array[3][3][3];
struct simple_struct struct_array[20];
point_t typedef_array[15][15];
union data_union union_array[8];

/* Array of pointers */
int* pointer_array[10];
struct complex_struct* struct_ptr_array[5];
void (*func_ptr_array[3])(int, float);

/* Flexible array member */
struct flexible_array {
    int count;
    double data[];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (callbacks) */
typedef int (*int_callback)(void);
typedef void (*void_callback)(int, float, char*);
typedef struct simple_struct* (*struct_callback)(int param);
typedef void (*complex_callback)(int (*nested_callback)(void), void* data);

/* Struct with callback members */
struct callback_container {
    int_callback get_value;
    void_callback process_data;
    complex_callback complex_operation;
};

/* Union with callback */
union callback_union {
    int (*int_func)(int);
    float (*float_func)(float);
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal structure patterns */
struct tree_common;
struct tree_decl_common;
struct tree_type_common;

/* Dummy structs with GCC internal naming patterns */
struct lang_type {
    int dummy_field;
};

struct lang_decl {
    void* lang_specific;
};

/* Struct that might be recognized by gengtype */
struct GTY(()) gcc_internal_struct {
    struct tree_common* common;
    struct lang_type* lang_type;
    int flags;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Deeply nested type hierarchy */
typedef struct node {
    int value;
    struct node** children; /* Array of pointers to nodes */
    int (*compare)(struct node*, struct node*);
    union {
        int int_data;
        double double_data;
        char* string_data;
    } data;
} node_t;

/* Struct containing array of pointers to unions */
struct container {
    int count;
    union data_union* items[10];
    node_t* (*create_node)(int value);
};

/* Function pointer returning pointer to struct containing callbacks */
typedef struct callback_container* (*factory_callback)(int type);

/* Complex typedef chain */
typedef node_t* node_ptr_t;
typedef node_ptr_t (*node_factory_t)(int, float);
typedef node_factory_t factory_array_t[5];

/* Struct with all type categories combined */
struct master_type {
    /* SCALAR */
    int id;
    
    /* STRING */
    const char* name;
    
    /* STRUCT (nested) */
    struct {
        int x;
        int y;
    } position;
    
    /* UNION */
    union {
        int int_val;
        float float_val;
    } value;
    
    /* POINTER */
    void* data;
    
    /* ARRAY */
    int scores[5];
    
    /* CALLBACK */
    int (*validator)(struct master_type*);
    
    /* USER_STRUCT (via typedef) */
    point_t origin;
    
    /* ARRAY of POINTERS to UNIONS */
    union data_union* data_ptrs[3];
    
    /* CALLBACK returning POINTER to STRUCT */
    struct callback_container* (*get_callbacks)(void);
};

/* ==================== FUNCTION DECLARATIONS ==================== */
/* Functions using all the defined types */
struct master_type* create_master_type(int id, const char* name);
int process_container(struct container* cont, int_callback cb);
void traverse_nodes(node_t* root, void_callback visitor);
transparent_union_t make_transparent_union(void* ptr);

#endif /* TEST_TYPES_H */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct *undefined_ptr_t;

/* Void in pointer context without full definition */
extern void *global_void_ptr;
typedef void (*void_func_ptr)(void);

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
long global_long = 100L;
unsigned int global_uint = 100U;
_Bool global_bool = 1;

/* Function parameters using scalars */
extern int process_scalars(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals in initializations */
char str_array[] = "test_string";
const char *const_string_ptr = "constant_string";
char *dynamic_string = "dynamic";

/* String in struct */
struct string_holder {
    const char *name;
    char buffer[256];
};

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct types with mixed members */
struct simple_struct {
    int id;
    float value;
    char tag;
};

struct complex_struct {
    int counter;
    double data[10];
    struct simple_struct *nested;
    char name[50];
};

struct nested_members {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
    struct complex_struct *complex_ptr;
};

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_member;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedefs for struct types */
typedef struct {
    int x;
    int y;
    int z;
} vector3d_t;

typedef struct node {
    int data;
    struct node *next;
    vector3d_t position;
} node_t;

typedef struct {
    char *title;
    int year;
    float rating;
} movie_t;

/* Complex typedef chain */
typedef struct complex_struct complex_t;
typedef complex_t *complex_ptr_t;

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union simple_union {
    int i;
    float f;
    char c;
    void *ptr;
};

/* Union with struct members */
union data_container {
    struct {
        int type;
        char name[20];
    } info;
    struct {
        double x;
        double y;
    } point;
    long long big_value;
};

/* Typedef union */
typedef union {
    int int_val;
    float float_val;
    double double_val;
} numeric_union_t;

/* Union with GCC attributes */
union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
    double fp_value;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char **string_ptr_ptr;

/* Struct pointers */
struct simple_struct *simple_ptr;
struct complex_struct **complex_ptr_ptr;
node_t *node_ptr;

/* Function pointers (also TYPE_CALLBACK) */
typedef int (*int_func_ptr)(int, int);
typedef void (*void_callback)(void *data, int status);

/* Void pointer */
void *generic_ptr;

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to pointer chain */
void ****quad_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and types */
int int_array[100];
float float_array[10][20];
double multi_dim_array[5][10][15];

/* Struct arrays */
struct simple_struct struct_array[50];
vector3d_t vector_array[100][3];

/* Pointer arrays */
void *ptr_array[25];
int_func_ptr callback_array[10];

/* String arrays */
const char *string_array[] = {"one", "two", "three", NULL};

/* Flexible array member in struct */
struct flex_array_struct {
    int count;
    double data[];  /* Flexible array member */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */
typedef int (*binary_op)(int a, int b);
typedef void (*event_handler)(int event_id, void *user_data);
typedef char *(*string_transformer)(const char *input, int flags);
typedef struct complex_struct *(*factory_func)(int param1, float param2);

/* Callback in struct */
struct processor {
    const char *name;
    binary_op operation;
    event_handler on_complete;
};

/* Nested callback */
typedef void (*outer_callback)(int (*inner_callback)(int), void *context);

/* Callback returning callback */
typedef int (*generator_func)(void);
typedef generator_func (*factory_generator)(int seed);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy structs that might be recognized by gengtype */
struct __gcc_internal_tree_node {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

typedef struct __gcc_internal_tree_node *gcc_tree_ptr;

/* Another potential internal type */
struct __gcc_dummy_lang_struct {
    int lang_flag;
    void *lang_data;
    struct __gcc_dummy_lang_struct *next;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */
typedef struct container {
    /* Struct containing array of pointers to unions */
    union data_container *union_array[20];
    
    /* Pointer to function returning pointer to struct with callback */
    struct processor *(*get_processor)(int id);
    
    /* Nested struct with complex members */
    struct {
        int depth;
        struct container *parent;
        void (*traverse)(struct container *, 
                        void (*visit)(struct container *, void *), 
                        void *context);
    } metadata;
    
    /* Array of callbacks */
    event_handler handlers[5];
    
    /* Pointer to array of structs */
    vector3d_t (*get_vectors)[];
    
    /* Self-referential pointer */
    struct container *next;
} container_t;

/* Function pointer returning pointer to struct containing callback members */
typedef container_t *(*container_factory)(
    int size, 
    void (*init)(container_t *),
    int (*validate)(const container_t *)
);

/* Union containing struct with array of pointers to functions */
union mega_union {
    struct {
        int type;
        int (*operations[10])(int, int);
        union mega_union *(*clone)(const union mega_union *);
    } complex;
    container_t *container;
    void *raw_data[8];
};

/* ==================== GCC ATTRIBUTES ==================== */
/* Types with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
    char padding;
};

struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
    double d;
};

/* Transparent union */
typedef union __attribute__((transparent_union)) trans_union {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
} trans_union_t;

/* Struct with multiple attributes */
struct __attribute__((aligned(8), packed)) multi_attr_struct {
    int id;
    char name[32];
    float values[4];
};

/* ==================== FINAL COMPLEX TYPEDEF ==================== */
/* Ultimate nested type definition */
typedef union mega_union *(*complex_operation)(
    container_t *(*factory)(int, 
                           int (*)(int), 
                           void (*)(void)),
    trans_union_t param,
    struct __gcc_dummy_lang_struct *lang_info
);

#endif /* TEST_TYPES_H */

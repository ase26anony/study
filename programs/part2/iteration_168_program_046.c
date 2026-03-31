#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* TYPE_STRING: String-related types */
const char *global_string_ptr = "test_string";
char global_string_array[] = "array_string";
const char *const global_const_string_ptr = "const_string";

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    double values[10];
    struct simple_struct nested;
    void *data;
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int counter;
    float ratio;
    char name[32];
} user_struct_t;

typedef struct complex_struct complex_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            double double_val;
            void *ptr_val;
        } data;
    } tagged;
    unsigned char raw[16];
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char *char_ptr;
void *void_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
user_struct_t *user_struct_ptr;
int **double_ptr_to_int;
void (*func_ptr)(void);

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[50][20];
double double_3d_array[10][10][10];
char char_array[] = {'a', 'b', 'c', '\0'};
struct simple_struct struct_array[25];
union simple_union union_array[15];
user_struct_t user_struct_array[5];
int *pointer_array[30];
void (*func_ptr_array[10])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(void);
typedef float (*float_callback_t)(int, float);
typedef void (*void_callback_t)(const char *, int);
typedef struct simple_struct *(*struct_callback_t)(int, void *);
typedef union simple_union (*union_callback_t)(double, int);

/* Complex nested callback type */
typedef int (*complex_callback_t)(int (*)(float), void **);

/* TYPE_LANG_STRUCT: GCC internal structure (simulated) */
struct tree_node;
struct tree_common {
    struct tree_node *chain;
    struct tree_node *type;
    int uid;
};

/* Complex nested type definitions to ensure deep traversal */

/* Struct containing array of pointers to unions */
struct container_struct {
    int id;
    union simple_union *union_ptrs[20];
    complex_callback_t callback;
    struct {
        int depth;
        struct container_struct *next;
    } nested;
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*meta_callback_t)(int, void (*)(void));

struct callback_container {
    int id;
    void (*start_callback)(void);
    int (*process_callback)(int, float);
    void (*end_callback)(const char *);
    meta_callback_t meta;
};

/* Typedef for complex nested type hierarchy */
typedef struct {
    struct {
        int level;
        struct container_struct *containers[5];
        union {
            int mode;
            float precision;
        } config;
    } settings;
    
    user_struct_t users[10];
    void (*handlers[3])(struct callback_container *);
    
    struct {
        int (*comparator)(const void *, const void *);
        void (*allocator)(size_t);
        void (*deallocator)(void *);
    } utilities;
} super_complex_t;

/* GCC-specific attributed types */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(64)));

typedef struct __attribute__((packed)) {
    unsigned char byte;
    unsigned short word;
    unsigned long dword;
} packed_typedef_t;

/* Mixed attribute struct */
struct multi_attr_struct {
    int x __attribute__((aligned(8)));
    char y __attribute__((packed));
    double z;
} __attribute__((aligned(32)));

#endif /* TEST_TYPES_H */

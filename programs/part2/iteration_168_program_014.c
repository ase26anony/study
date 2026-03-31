#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined;
struct another_undefined;
void undefined_function(void*); /* void pointer parameter */

/* TYPE_SCALAR: Fundamental types */
int scalar_int;
float scalar_float;
double scalar_double;
char scalar_char;
long scalar_long;
unsigned int scalar_unsigned;

/* TYPE_STRING: String types */
const char* string_pointer = "test_string";
char string_array[] = "another_string";
const char* const string_const_pointer = "const_string";

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int a;
    float b;
    char c;
};

struct complex_struct {
    int id;
    double values[10];
    struct simple_struct nested;
    char name[50];
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
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

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            char* string_value;
        } data;
    } tag;
    long long raw_data;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
int* int_pointer;
struct simple_struct* struct_pointer;
union simple_union* union_pointer;
point_t* user_struct_pointer;
void* void_pointer;
const void* const_void_pointer;
volatile int* volatile_pointer;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
point_t user_struct_array[25];
int* pointer_array[50];
char* string_array_2d[10][30];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback)(int, int);
typedef void (*void_callback)(void);
typedef char* (*string_callback)(const char*, int);
typedef struct simple_struct* (*struct_callback)(int id);
typedef void (*complex_callback)(int (*)(float), void*);

/* Struct containing callback members */
struct callback_container {
    int_callback int_func;
    string_callback string_func;
    void_callback cleanup_func;
};

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node) */
struct tree_node;
struct tree_common;
struct tree_type;

/* Complex nested type hierarchy */
typedef union {
    struct {
        int type_tag;
        union {
            int int_val;
            double double_val;
            struct {
                char* data;
                int length;
            } string_val;
            struct nested_in_union {
                point_t points[4];
                int count;
            } struct_val;
        } value;
    } tagged;
    unsigned char raw[64];
} mega_union_t;

/* Function pointer returning pointer to struct containing callbacks */
typedef struct callback_container* (*factory_callback)(int, const char*);

/* Struct with array of pointers to unions */
struct union_container {
    union simple_union* unions[20];
    int count;
    factory_callback create;
};

/* Even more complex nesting */
typedef struct {
    mega_union_t data;
    struct union_container* containers;
    int_callback validators[5];
    struct {
        point_t* points;
        int size;
    } dynamic_array;
} super_complex_t;

/* GCC attributes on various types */
struct aligned_struct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((aligned(32)));

typedef struct __attribute__((packed)) {
    unsigned char flags;
    int value;
} packed_user_t;

/* Mixed declarations with attributes */
volatile const int* volatile const_volatile_array[10] 
    __attribute__((aligned(64)));

#endif /* TEST_TYPES_H */

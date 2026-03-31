/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long longlong_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string_t;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    float z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    double area;
};

typedef struct {
    int id;
    string_t name;
    float score;
} Student;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    char char_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Fixed-size arrays */
int global_array[100];
struct Point point_array[50];
union Data data_array[20];
char string_array[10][256];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef string_t (*StringGenerator)(void);
typedef int (*BinaryOp)(int, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    int (*processor)(struct ComplexType*);
    void* user_data;
    char name[64];
} ComplexType;

/* Function prototypes using various types */
void process_point(struct Point* p);
union Data generate_data(int seed);
int compare_strings(const void* a, const void* b);
ComplexType* create_complex_type(string_t name);

#endif /* TEST_TYPES_H */

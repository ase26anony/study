/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
struct forward_declared;

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
    long long as_int;
    double as_float;
    void* as_ptr;
    struct Point as_point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*FuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef float Matrix[3][3];
typedef char StringArray[20][50];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);
typedef int (*BinaryOp)(int, int);
typedef void (*ErrorHandler)(const char*, int);

/* TYPE_LANG_STRUCT: Structures with GCC attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    IntArray counts;
    Comparator compare;
    struct ComplexType* next;
    char name[50];
} ComplexType;

/* Function declarations using these types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data_int(int value);
ComplexType* create_complex_type(const char* name);

#endif /* TEST_TYPES_H */

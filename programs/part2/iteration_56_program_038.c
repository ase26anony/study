#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_SCALAR: Basic scalar types */
typedef int MyInt;
typedef float MyFloat;
typedef double MyDouble;
typedef char MyChar;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char *s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef void* VoidPtr;
typedef struct Point* PointPtr;

/* TYPE_ARRAY: Array type declarations */
typedef int IntArray[10];
typedef float Matrix[5][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryFunc)(int, int);
typedef void (*Callback)(void*);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* Complex nested type for thorough traversal */
typedef struct ComplexType {
    union Data *data_array[5];          /* Array of pointers to union */
    int (*operations[3])(int, int);     /* Array of function pointers */
    struct Point points[10];            /* Array of structs */
    void **generic_ptrs;                /* Pointer to void pointers */
} ComplexType;

/* External declarations for multi-file testing */
extern struct ExternalStruct {
    int external_id;
    char external_name[50];
} external_var;

extern void process_types(void);

#endif /* TEST_TYPES_H */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

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
    char c;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef void* VoidPtr;
typedef struct Point* PointPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef float Matrix[5][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidCallback)(void);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* Complex nested type declarations */
typedef struct Container {
    /* Nested: Structure containing array of pointers */
    int *items[10];
    
    /* Nested: Pointer to function pointer */
    int (**func_table)(void);
    
    /* Nested: Union within struct */
    union {
        int int_val;
        float float_val;
    } value;
} Container;

/* Cross-file type reference */
extern struct ExternalType {
    int id;
    char *description;
} external_var;

/* Function declarations using various types */
int process_data(struct Point *points, int count);
MyStruct* create_struct(int data, const char *name);
void register_callback(VoidCallback cb);

#endif /* TEST_TYPES_H */

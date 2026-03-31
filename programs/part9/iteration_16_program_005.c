#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char* mutable_string;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
extern int int_array[10];
extern char char_array[20];
extern struct Point point_array[5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);

/* Complex nested type combinations */
struct ComplexType {
    /* Nested struct containing array of pointers to unions */
    union Value* value_ptrs[8];
    
    /* Struct with callback member */
    Callback notify;
    
    /* Pointer to array of structs */
    struct Point (*point_matrix)[4];
    
    /* Qualifiers */
    volatile const int* volatile* complex_ptr;
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((deprecated));

/* Aligned union */
union __attribute__((aligned(16))) AlignedUnion {
    long long data[2];
    char bytes[16];
};

#endif /* TYPES_H */

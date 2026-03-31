#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile const int volatile_const_int;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char global_char_array[50];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
} __attribute__((packed));

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
typedef int IntArray[10];
typedef char StringArray[5][50];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type for deep traversal */
struct ComplexType {
    union Value values[5];           /* Array of unions */
    MyStruct* struct_ptrs[8];        /* Array of pointers to user structs */
    Comparator compare_func;         /* Callback function pointer */
    volatile const int* volatile* complex_ptr; /* Complex pointer with qualifiers */
};

/* GCC-specific attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    double precise;
};

struct __attribute__((deprecated)) DeprecatedStruct {
    int old_data;
};

/* Packed struct with pragma */
#pragma pack(push, 1)
struct PackedData {
    char type;
    int value;
    short count;
};
#pragma pack(pop)

#endif /* TYPES_H */

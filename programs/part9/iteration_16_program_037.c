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
extern char global_string_array[];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;
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
typedef char CharMatrix[5][20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char type;
    int value;
    short count;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int flags;
};

/* Deprecated typedef */
typedef int OldIntType __attribute__((deprecated));

/* Complex nested type */
struct ComplexType {
    union Value* values[5];          /* Array of pointers to union */
    Comparator compare_func;         /* Callback member */
    struct Point points[10];         /* Array of structs */
    volatile const int* volatile* ppi; /* Complex pointer */
};

/* For C++ specific types */
#ifdef __cplusplus
namespace TestTypes {
    class SimpleClass {
    private:
        int private_data;
    public:
        SimpleClass() : private_data(0) {}
        int getData() const { return private_data; }
        void setData(int value) { private_data = value; }
    };
    
    template<typename T>
    class Box {
        T value;
    public:
        Box(const T& v) : value(v) {}
        T get() const { return value; }
    };
}
#endif

#endif /* TYPES_H */

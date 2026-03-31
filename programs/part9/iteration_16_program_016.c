#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char* name;  /* TYPE_STRING */
} MyStruct;

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator)(const void*, const void*);

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array type */
typedef int IntArray[10];

/* Complex nested type */
struct ComplexType {
    union Value values[5];  /* TYPE_ARRAY of TYPE_UNION */
    MyStruct* items[20];    /* TYPE_ARRAY of TYPE_POINTER to TYPE_USER_STRUCT */
    comparator cmp;         /* TYPE_CALLBACK */
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    short c;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* Volatile and const qualified pointers */
typedef volatile const int* volatile* ComplexPtr;

/* External declarations */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Value global_union;

#endif /* TYPES_H */

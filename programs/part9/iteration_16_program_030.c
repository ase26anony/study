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
    float value;
} MyStruct;

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef char StringArray[5][20];

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data;
    int count;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* External declarations */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Value global_union;

#endif /* TYPES_H */

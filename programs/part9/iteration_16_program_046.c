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
    char name[32];
} MyStruct;

/* TYPE_UNION */
union Value {
    int i;
    float f;
    double d;
    void* p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct Point* PointPtr;
typedef void (*func_ptr)(void);

/* Complex nested type */
typedef struct ComplexType ComplexType;

/* GCC attributes for edge cases */
struct PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((packed));

struct AlignedStruct {
    double data;
    int counter;
} __attribute__((aligned(16)));

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* Extern declarations */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Value global_union;

#endif /* TYPES_H */

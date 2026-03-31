#ifndef TYPES_COMMON_H
#define TYPES_COMMON_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator)(const void*, const void*);

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef MyStruct* MyStructPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array type */
typedef int IntArray[10];

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char c;
    int i;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double d;
    int i;
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* Extern declarations for cross-file usage */
extern struct Point global_point;
extern union Value global_value;

#endif /* TYPES_COMMON_H */

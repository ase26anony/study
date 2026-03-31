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
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*func_ptr)(void);

/* Complex nested type */
typedef union Value* (*complex_callback)(struct Point**, int);

/* GCC attributes for edge cases */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int flags;
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct_t;

/* For TYPE_LANG_STRUCT - using visibility attribute */
struct __attribute__((visibility("hidden"))) HiddenStruct {
    int secret;
    char key[16];
};

/* External declarations */
extern struct Point global_point;
extern MyStruct global_mystruct;
extern union Value global_value;

#endif /* TYPES_H */

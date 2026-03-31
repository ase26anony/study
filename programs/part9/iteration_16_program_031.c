#ifndef TEST_TYPES_H
#define TEST_TYPES_H

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

/* TYPE_UNION: Union definition */
union Value {
    int i;
    float f;
    double d;
    char c;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_LANG_STRUCT: Use GCC attribute to potentially trigger this category */
struct __attribute__((visibility("hidden"))) HiddenStruct {
    int secret;
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char flag;
    int value;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

/* Deprecated typedef */
typedef int OldInt __attribute__((deprecated));

/* External declarations for multiple translation units */
extern struct Point global_point;
extern MyStruct global_struct;
extern union Value global_union;

#endif /* TEST_TYPES_H */

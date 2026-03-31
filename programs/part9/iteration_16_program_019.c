#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_USER_STRUCT: Typedef for struct */
typedef struct {
    int id;
    const char* name;
} UserStruct;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_UNION: Plain union */
union Value {
    int i;
    float f;
    double d;
};

/* Compiler attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct_t;

/* External declarations for multiple translation units */
extern struct Point global_point;
extern const char* global_string;
extern int global_array[50];

#endif /* TEST_TYPES_H */

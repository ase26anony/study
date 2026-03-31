#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int data;
    char tag;
} MyStruct;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    char c;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern const char *global_string;

#endif /* TEST_TYPES_H */

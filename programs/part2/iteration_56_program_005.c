#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct NestedContainer {
    struct Point points[5];
    void *metadata;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
    float score;
} Student;

typedef Student *StudentPtr;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char *str;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char flag;
    int value;
    double data;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long a;
    long long b;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *message, int severity);

/* Complex nested callback */
typedef struct Node *(*NodeAllocator)(void);
typedef void (*NodeProcessor)(struct Node *, NodeAllocator);

/* External declarations for multi-file testing */
extern struct IncompleteStruct *global_incomplete_ptr;
extern void process_student(Student *s);

#endif /* TEST_TYPES_H */

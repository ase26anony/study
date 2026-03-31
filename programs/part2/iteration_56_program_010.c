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
    double precision;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long data[4];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern Student global_student;
extern union Data global_data;

/* Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef struct Point* (*PointFactory)(int, int);

#endif /* TEST_TYPES_H */

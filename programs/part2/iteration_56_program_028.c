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

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    char c;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[50];
} UserStruct;

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long data[4];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Callback)(int, void *);

/* External declarations for multi-file testing */
extern struct Point global_point;
extern union Data global_data;
extern void process_types(void);

#endif /* TEST_TYPES_H */

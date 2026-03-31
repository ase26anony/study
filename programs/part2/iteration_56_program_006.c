/* test_types.h - Header file with forward declarations and opaque types */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;           /* Forward declaration */
union IncompleteUnion;             /* Forward declaration */
typedef struct OpaqueType OpaqueType; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structure */
struct GlobalStruct {
    int id;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    double x;
    double y;
    double z;
} Vector3D;

/* TYPE_UNION: Global union */
union DataContainer {
    int int_val;
    float float_val;
    double double_val;
    char char_val;
    void *ptr_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *message);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedData {
    char flag;
    int counter;
    double value;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
};

/* External declarations for multi-file testing */
extern struct GlobalStruct global_instance;
extern Vector3D global_vector;

#endif /* TEST_TYPES_H */

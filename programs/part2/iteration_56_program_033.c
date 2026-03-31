#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_STRUCT: Plain C structure */
struct GlobalStruct {
    int id;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    double x, y, z;
} Vector3D;

/* TYPE_UNION: Global union */
union DataContainer {
    long int_val;
    double float_val;
    void *ptr_val;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedData {
    unsigned char flags;
    unsigned int value;
};

/* External declarations for cross-file references */
extern struct GlobalStruct global_instance;
extern Vector3D global_vector;

#endif /* TEST_TYPES_H */

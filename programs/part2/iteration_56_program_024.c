#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

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
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedData {
    char flag;
    int value;
    double precision;
};

/* External declarations for cross-file references */
extern struct GlobalStruct global_instance;
extern Vector3D *create_vector(double x, double y, double z);

#endif /* TEST_TYPES_H */

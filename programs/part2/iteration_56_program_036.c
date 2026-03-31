#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
void undefined_function(void); /* void return type */

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
    char *string_val;
};

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*Comparator)(const void *, const void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedData {
    unsigned char flags;
    unsigned int value;
};

/* Opaque pointer type for cross-file testing */
typedef struct OpaqueType *OpaqueHandle;

/* External declarations for multi-file testing */
extern struct GlobalStruct global_instance;
extern Vector3D global_vector;

#endif /* TEST_TYPES_H */

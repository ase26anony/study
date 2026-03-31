#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct GlobalStruct {
    int id;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    double x;
    double y;
    double z;
} Vector3D;

/* TYPE_UNION: Union declarations */
union DataContainer {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Logger)(const char*);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedStruct {
    char flag;
    int counter;
    double value;
};

/* Complex nested type for thorough traversal */
struct ComplexType {
    union DataContainer* variants;      /* Pointer to union */
    Vector3D* vectors;                  /* Pointer to typedef struct */
    Comparator compare_func;            /* Function pointer */
    struct GlobalStruct metadata;       /* Nested plain struct */
};

/* External declarations for multi-file testing */
extern struct GlobalStruct global_instance;
extern Vector3D* create_vector(double x, double y, double z);

#endif /* TEST_TYPES_H */

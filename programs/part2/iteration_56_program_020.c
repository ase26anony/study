/* test_types.h - Header file with forward declarations and opaque types */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_STRUCT: Plain structure declaration */
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

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedData {
    unsigned char flags;
    unsigned int value;
};

/* External declarations for multi-file testing */
extern void process_types(void);
extern int compute_checksum(void);

#endif /* TEST_TYPES_H */

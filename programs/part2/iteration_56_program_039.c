/* test_types.h - Header file with forward declarations and opaque types */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;           /* Forward declaration */
union IncompleteUnion;             /* Forward declaration */
typedef struct OpaqueType OpaqueType;  /* Opaque pointer type */

/* TYPE_STRUCT: Plain C structure declaration */
struct GlobalStruct {
    int id;
    float value;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    char name[32];
    int count;
} UserStruct;

/* TYPE_UNION: Union declaration */
union DataUnion {
    int i;
    float f;
    double d;
    void *p;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedAttrStruct {
    char flag;
    int number;
    double value;
};

/* TYPE_ARRAY: Array type in header */
extern int global_array[100];

/* External function declarations */
extern void process_data(void *data, int size);
extern OpaqueType *create_opaque(int value);

#endif /* TEST_TYPES_H */

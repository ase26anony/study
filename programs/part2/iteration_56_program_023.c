/* test_types.h - Header file with incomplete types and declarations */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;          /* Forward declaration */
union IncompleteUnion;            /* Forward declaration */
typedef struct OpaqueType Opaque; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structure */
struct GlobalStruct {
    int id;
    float value;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    char name[32];
    int count;
} UserDefinedStruct;

/* TYPE_UNION */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char char_val;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedStruct {
    char flag;
    int number;
    double value;
};

/* External declarations for multi-file testing */
extern struct GlobalStruct global_instance;
extern void process_types(void);

#endif /* TEST_TYPES_H */

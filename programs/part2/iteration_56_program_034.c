/* test_types.h - Header file for multi-file type declarations */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_SCALAR: Basic scalar types */
typedef int MyInt;
typedef float MyFloat;
typedef double MyDouble;
typedef char MyChar;

/* TYPE_STRUCT: Plain C structure */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct {
    int data;
    char *name;
} MyStruct;

/* TYPE_UNION: Union declaration */
union Data {
    int i;
    float f;
    double d;
    char *str;
};

/* TYPE_ARRAY: Array type in header */
extern int global_array[100];

/* TYPE_POINTER: Function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* TYPE_CALLBACK: Callback function type */
typedef void (*EventHandler)(int event_id, void *user_data);

/* TYPE_LANG_STRUCT: GCC attribute extension */
struct __attribute__((packed, aligned(1))) PackedStruct {
    char flag;
    int value;
    double precision;
};

/* External declarations for multi-file testing */
extern void process_types(void);
extern int compute_checksum(void);

#endif /* TEST_TYPES_H */

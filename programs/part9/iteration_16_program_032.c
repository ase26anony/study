#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* Forward declarations for TYPE_UNDEFINED */
struct Opaque;           /* Incomplete struct - TYPE_UNDEFINED */
struct AnotherOpaque;    /* Another incomplete type */

/* TYPE_CALLBACK - Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);
typedef size_t (*strlen_func)(const char*);

/* TYPE_USER_STRUCT - Typedef structs */
typedef struct {
    int id;
    char name[32];
} UserStruct;

typedef struct Point {
    int x;
    int y;
} Point_t;  /* TYPE_USER_STRUCT when referenced via typedef */

/* TYPE_STRUCT - Plain struct */
struct PlainStruct {
    int counter;
    double value;
};

/* TYPE_UNION */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char char_val;
};

/* Complex nested type definitions */
typedef union DataUnion* DataUnionPtr;

/* Array type for use in other files */
extern int global_array[100];

/* String type */
extern const char* global_string;

/* Function declarations */
extern void process_data(struct PlainStruct* ps);
extern comparator get_comparator(void);

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char flag;
    int value;
    double data;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
};

/* Deprecated typedef */
typedef int OldIntType __attribute__((deprecated("Use int instead")));

#endif /* TYPES_H */

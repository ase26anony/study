/* type_zoo.h */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types - TYPE_SCALAR */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* String type - TYPE_STRING */
extern char *global_string;

/* Struct types - TYPE_STRUCT */
struct SimpleStruct {
    int x;
    float y;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    char tag;
};

/* User struct types - TYPE_USER_STRUCT */
typedef struct {
    int id;
    char name[32];
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types - TYPE_UNION */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long big;
    double precise;
} TypedefUnion;

/* Language-specific struct - TYPE_LANG_STRUCT */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Callback types - TYPE_CALLBACK */
typedef int (*Comparator)(const void*, const void*);
typedef void (*VoidCallback)(int, char*);

/* Function declarations using various types */
void process_scalars(int a, float b, double c, char d);
struct ComplexStruct create_complex(int seed);
UserStruct* create_user_struct(const char* name);
void use_union(union SimpleUnion u);
int compare_values(const void* a, const void* b);
void array_operations(void);

#endif /* TYPE_ZOO_H */

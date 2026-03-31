#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int id;
    const char* name;
} UserStruct;

/* GCC-specific attributes */
#define PACKED __attribute__((packed))
#define ALIGNED __attribute__((aligned(16)))
#define DEPRECATED __attribute__((deprecated))

#endif /* TEST_TYPES_H */

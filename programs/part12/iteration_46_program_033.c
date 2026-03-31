// Create a pointer to int type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *ptr_type = malloc(sizeof(struct type));
ptr_type->kind = TYPE_POINTER;
ptr_type->u.pointer_to = int_type;  // Pointer to int

// Create an array of pointers to int
struct type *array_type = malloc(sizeof(struct type));
array_type->kind = TYPE_ARRAY;
array_type->u.element_type = ptr_type;  // Array of (pointer to int)

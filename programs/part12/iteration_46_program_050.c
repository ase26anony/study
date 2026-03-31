// Create an integer pointer type
struct type *int_type = create_type(TYPE_SCALAR);
struct type *int_ptr_type = create_type(TYPE_POINTER);
int_ptr_type->u.pointer_to = int_type;

// Create an array of integers
struct type *int_array_type = create_type(TYPE_ARRAY);
int_array_type->u.element_type = int_type;

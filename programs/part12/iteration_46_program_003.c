// Create a pointer to int
struct type *int_type = create_scalar_type();
struct type *ptr_type = create_pointer_type(int_type);

// Create an array of pointers
struct type *arr_type = create_array_type(ptr_type, 10);

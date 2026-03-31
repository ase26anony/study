   // Allocate memory for struct + data
   size_t data_size = 100;
   struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
   
   // Set the length
   ts->length_bits = data_size * 8; // Convert bytes to bits
   
   // Use the data
   strncpy(ts->data, "Hello", data_size);

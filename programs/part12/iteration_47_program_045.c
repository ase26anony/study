   size_t data_size = 100; // Desired array size
   struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
   ts->length_bits = data_size * 8; // Example: bits = bytes * 8

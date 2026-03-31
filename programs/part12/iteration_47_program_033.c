   struct tagged_string *ts = malloc(sizeof(struct tagged_string) + desired_data_size);
   ts->length_bits = some_value;
   // Use ts->data[0...desired_data_size-1]

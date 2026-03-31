DW_TAG_structure_type (Message)
  DW_AT_name: "Message"
  DW_TAG_member (id)
  DW_TAG_member (data)
    DW_AT_type: reference to encrypted_string type
    DW_AT_data_bit_offset: 32  // After the 32-bit 'id'

DW_TAG_typedef (encrypted_string)
  DW_AT_name: "encrypted_string"
  DW_AT_type: reference to base type

DW_TAG_base_type
  DW_AT_name: "__BitInt(128)"
  DW_AT_encoding: DW_ATE_signed/unsigned
  DW_AT_byte_size: 16
  DW_AT_bit_size: 128

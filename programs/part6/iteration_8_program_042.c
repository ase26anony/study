   unsigned int size = RTX_CODE_SIZE (REG);
   rtx new_reg = (rtx) alloca (size);
   memset (new_reg, 0, size);

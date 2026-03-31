   rtx new_reg = (rtx) alloca (size);
   memset (new_reg, 0, size);
   PUT_CODE (new_reg, REG);

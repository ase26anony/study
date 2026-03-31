// Consider using ggc_alloc or persistent allocation if needed beyond scope
rtx new_reg = gen_rtx_REG(GET_MODE(*loc), 
                         LAST_VIRTUAL_REGISTER + 1);

// Or if manual allocation is necessary:
rtx new_reg = (rtx) ggc_alloc(size);
PUT_CODE(new_reg, REG);
set_mode_and_regno(new_reg, GET_MODE(*loc),
                   LAST_VIRTUAL_REGISTER + 1);

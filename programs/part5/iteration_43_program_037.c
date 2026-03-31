case PLUGIN_PASS_MANAGER_SETUP:
    gcc_assert (!callback);
    register_pass ((struct register_pass_info *) user_data);
    break;

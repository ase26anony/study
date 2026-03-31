int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
    // Register passes
    struct register_pass_info pass_info = {...};
    register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
    
    // Register plugin info
    struct plugin_info my_info = {...};
    register_callback(plugin_name, PLUGIN_INFO, NULL, &my_info);
    
    return 0;
}

/* test_plugin.c - GCC plugin to test specific plugin infrastructure */
#include <stdio.h>
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible;

/* Dummy pass structure */
static struct opt_pass my_dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &my_dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    { NULL, 0, sizeof(void*), NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    struct plugin_pass pass_data;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin version mismatch\n");
        return 1;
    }
    
    printf("Plugin initialized: %s\n", plugin_info_args->base_name);
    
    /* Register for PLUGIN_INFO event */
    if (plugin_event(plugin_info_args->base_name, PLUGIN_INFO, &plugin_info) != PLUGIN_SUCCESS) {
        fprintf(stderr, "Failed to register PLUGIN_INFO\n");
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    if (plugin_event(plugin_info_args->base_name, PLUGIN_PASS_MANAGER_SETUP, &pass_info) != PLUGIN_SUCCESS) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    if (plugin_event(plugin_info_args->base_name, PLUGIN_REGISTER_GGC_ROOTS, dummy_ggc_root_tab) != PLUGIN_SUCCESS) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    
    printf("All plugin events registered successfully\n");
    return PLUGIN_SUCCESS;
}

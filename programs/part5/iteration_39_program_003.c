/* test_plugin.c - GCC plugin to test specific plugin event callbacks */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible = 1;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-test-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage testing of plugin infrastructure"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = NULL,
        .nelt = 0,
        .stride = 0,
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator entry */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    struct plugin_pass pass_data;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_INFO event */
    if (plugin_event(plugin_info_args->base_name,
                     PLUGIN_INFO,
                     &plugin_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    if (plugin_event(plugin_info_args->base_name,
                     PLUGIN_PASS_MANAGER_SETUP,
                     &pass_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    if (plugin_event(plugin_info_args->base_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     dummy_ggc_root_tab) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    return PLUGIN_SUCCESS;
}

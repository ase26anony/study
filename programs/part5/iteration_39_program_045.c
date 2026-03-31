/* test_plugin.c - GCC plugin to test uncovered event cases */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"

int plugin_is_GPL_compatible;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass my_dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &my_dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab my_ggc_root_tab[] = {
    {
        .base = NULL,
        .nelt = 0,
        .stride = 0,
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    struct plugin_pass pass_data;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP */
    if (plugin_event(plugin_info->base_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     &pass_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_INFO */
    if (plugin_event(plugin_info->base_name, 
                     PLUGIN_INFO, 
                     &my_plugin_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS */
    if (plugin_event(plugin_info->base_name, 
                     PLUGIN_REGISTER_GGC_ROOTS, 
                     my_ggc_root_tab) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    return PLUGIN_SUCCESS;
}

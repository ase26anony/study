/* test_plugin.c - GCC plugin to test specific plugin events */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible;

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

/* Pass info structure for registration */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    { NULL, 0, sizeof(void *), NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info_args->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP */
    if (plugin_event(plugin_name, PLUGIN_PASS_MANAGER_SETUP, &dummy_pass_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_INFO */
    if (plugin_event(plugin_name, PLUGIN_INFO, &plugin_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS */
    if (plugin_event(plugin_name, PLUGIN_REGISTER_GGC_ROOTS, dummy_ggc_roots) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    return PLUGIN_SUCCESS;
}

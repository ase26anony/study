/* test_plugin.c - GCC plugin to test specific plugin infrastructure events */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible;

/* Dummy pass definition */
static unsigned int dummy_pass_execute(void)
{
    return 0;
}

static bool dummy_pass_gate(void)
{
    return true;
}

static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
};

/* Register pass info structure */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin infrastructure events"
};

/* Minimal GGC root table (empty terminator) */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    { NULL, 0, sizeof(void *), NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    if (register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, 
                          NULL, (void *)&dummy_pass_info) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_INFO event */
    if (register_callback(plugin_name, PLUGIN_INFO, 
                          NULL, (void *)&plugin_info_data) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    if (register_callback(plugin_name, PLUGIN_REGISTER_GGC_ROOTS, 
                          NULL, (void *)dummy_ggc_root_tab) != PLUGIN_SUCCESS) {
        return 1;
    }
    
    return PLUGIN_SUCCESS;
}

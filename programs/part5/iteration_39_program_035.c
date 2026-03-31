/* test_plugin.c - GCC plugin to test specific plugin events */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"

int plugin_is_GPL_compatible;

/* Dummy pass structure */
static struct opt_pass my_dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "my-dummy-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

/* Register pass info structure */
static struct register_pass_info my_pass_info = {
    .pass = &my_dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* Minimal GGC root table - empty terminator */
static const struct ggc_root_tab my_ggc_root_tab[] = {
    { NULL, 0, NULL, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* callback is NULL as required */
                      (void *)&my_pass_info);
    
    /* Register for PLUGIN_INFO event */
    register_callback(plugin_info->base_name,
                      PLUGIN_INFO,
                      NULL,  /* callback is NULL as required */
                      (void *)&my_plugin_info);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_info->base_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* callback is NULL as required */
                      (void *)my_ggc_root_tab);
    
    return 0;  /* PLUGIN_SUCCESS */
}

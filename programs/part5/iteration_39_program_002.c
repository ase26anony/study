/* test_plugin.c - GCC plugin to test uncovered plugin infrastructure code */

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

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
    .pass = &my_dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Test plugin for covering GCC plugin infrastructure code coverage"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab my_ggc_roots[] = {
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
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    struct plugin_pass pass_data;
    int event;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_INFO event */
    event = PLUGIN_INFO;
    register_callback(plugin_info->base_name,
                      event,
                      NULL,  /* callback is NULL as required by gcc_assert */
                      (void *)&my_plugin_info);
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    event = PLUGIN_PASS_MANAGER_SETUP;
    register_callback(plugin_info->base_name,
                      event,
                      NULL,  /* callback is NULL as required by gcc_assert */
                      (void *)&my_pass_info);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    event = PLUGIN_REGISTER_GGC_ROOTS;
    register_callback(plugin_info->base_name,
                      event,
                      NULL,  /* callback is NULL as required by gcc_assert */
                      (void *)my_ggc_roots);
    
    return PLUGIN_SUCCESS;
}

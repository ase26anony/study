/* test_plugin.c - GCC plugin to test uncovered plugin infrastructure code */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"

/* Required plugin metadata */
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
    .todo_flags_finish = 0,
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
    .help = "Test plugin for covering GCC plugin infrastructure code coverage"
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
    const char *plugin_name = plugin_info_args->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s: Initializing...\n", plugin_name);
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* callback is NULL as required by gcc_assert */
                     (void *)&pass_info);
    
    /* Register for PLUGIN_INFO event */
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* callback is NULL as required by gcc_assert */
                     (void *)&plugin_info);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* callback is NULL as required by gcc_assert */
                     (void *)dummy_ggc_root_tab);
    
    printf("Plugin %s: Registered for target events\n", plugin_name);
    
    return PLUGIN_SUCCESS;
}

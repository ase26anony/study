/* test_plugin.c - GCC plugin to trigger uncovered lines in plugin.cc */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "pass_manager.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "test_coverage_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_var = 0;

/* Minimal dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = NULL,
    .execute = NULL,
    .sub = NULL,
    .next = NULL,
    .static_pass_number = 0,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Dummy pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin.cc lines 458-470"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_var,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_var),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL } /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1; /* Version mismatch */
    }
    
    /* Set plugin name from arguments */
    plugin_name = plugin_info_args->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP with NULL callback */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     NULL, /* callback must be NULL as per uncovered code */
                     &dummy_pass_info);
    
    /* Register for PLUGIN_INFO with NULL callback */
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL, /* callback must be NULL as per uncovered code */
                     &plugin_info_data);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS with NULL callback */
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL, /* callback must be NULL as per uncovered code */
                     dummy_ggc_roots);
    
    /* Optional: Register for finish event to confirm execution */
    register_callback(plugin_name, PLUGIN_FINISH, NULL, NULL);
    
    return 0; /* Success */
}

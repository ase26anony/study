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
static int dummy_ggc_root = 0;

/* Minimal pass structure for PLUGIN_PASS_MANAGER_SETUP */
static bool dummy_gate(void)
{
    /* Always return false so the pass doesn't actually run */
    return false;
}

static unsigned int dummy_execute(void)
{
    /* This should never be called since gate returns false */
    return 0;
}

/* Define a minimal pass */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Data structure for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Data structure for PLUGIN_INFO */
static struct plugin_info plugin_info_struct = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin.cc lines 458-470"
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(int),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_arg,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_arg->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP - triggers line 458-460 */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        &pass_info
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", 
                plugin_name);
        return result;
    }
    
    /* Register for PLUGIN_INFO - triggers line 461-463 */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        &plugin_info_struct
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return result;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS - triggers line 464-466 */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        dummy_roots
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", 
                plugin_name);
        return result;
    }
    
    /* Optional: Register finish callback to confirm plugin executed */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,  /* No callback needed for finish */
        NULL
    );
    
    return 0;  /* Success */
}

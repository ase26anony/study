/*
 * GCC plugin to trigger uncovered lines in plugin.cc
 * Specifically targets lines 458-470 covering:
 *   PLUGIN_PASS_MANAGER_SETUP
 *   PLUGIN_INFO  
 *   PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "context.h"
#include "pass_manager.h"

/* Mandatory plugin declarations */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
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

static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
    .next = NULL,
    .static_pass_number = 0,
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
    .help = "Plugin to trigger coverage of specific plugin.cc lines\n"
            "Registers for PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS"
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
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
    
    printf("Coverage trigger plugin initializing: %s\n", plugin_name);
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP
     * This triggers lines 458-460 in plugin.cc
     * Note: callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL as asserted in plugin.cc */
        (void *)&pass_info
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    
    printf("Registered PLUGIN_PASS_MANAGER_SETUP\n");
    
    /* 
     * Register for PLUGIN_INFO  
     * This triggers lines 461-463 in plugin.cc
     * Note: callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL as asserted in plugin.cc */
        (void *)&plugin_info_struct
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_INFO\n");
        return 1;
    }
    
    printf("Registered PLUGIN_INFO\n");
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers lines 464-466 in plugin.cc
     * Note: callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL as asserted in plugin.cc */
        (void *)dummy_ggc_roots
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    
    printf("Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    /* Optional: Register finish callback to confirm execution */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    printf("Coverage trigger plugin initialized successfully\n");
    
    return 0;  /* Return 0 for success */
}

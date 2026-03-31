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

/* Mandatory plugin declarations */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "test_coverage_plugin";

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
    .name = "dummy-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
    .next = NULL,
    .static_pass_number = 0
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
    .help = "Test plugin for coverage analysis"
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_root_tab[] = {
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
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1; /* Return error */
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_arg->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers lines 458-460 in plugin.cc */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* callback must be NULL as per uncovered code */
                      &pass_info);
    
    /* Register for PLUGIN_INFO event
     * This triggers lines 461-463 in plugin.cc */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* callback must be NULL as per uncovered code */
                      &plugin_info_struct);
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers lines 464-466 in plugin.cc */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* callback must be NULL as per uncovered code */
                      dummy_root_tab);
    
    /* Optional: Register for finish event to confirm execution */
    register_callback(plugin_name,
                      PLUGIN_FINISH,
                      NULL,
                      NULL);
    
    return 0; /* Success */
}

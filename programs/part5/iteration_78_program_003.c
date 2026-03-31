/* test_plugin.c - GCC plugin to trigger specific uncovered lines in plugin.cc */
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
    /* This pass does nothing */
    return 0;
}

static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
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
    .help = "Test plugin for coverage analysis. Triggers PLUGIN_INFO event."
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_ggc_roots[] = {
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
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1; /* Return non-zero to indicate failure */
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_args->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)&pass_info
    );
    
    if (ret != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Register for PLUGIN_INFO event */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)&plugin_info_struct
    );
    
    if (ret != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)dummy_ggc_roots
    );
    
    if (ret != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Optional: Register for PLUGIN_FINISH to confirm execution */
    ret = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,  /* No callback needed for finish */
        NULL
    );
    
    /* Return 0 to indicate successful initialization */
    return 0;
}

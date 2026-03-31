/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "ggc.h"
#include "plugin-version.h"
#include "c-family/c-common.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PART 1: Data structures for the three events
   ============================================ */

/* 1. For PLUGIN_PASS_MANAGER_SETUP: A simple dummy pass */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder pass */
    return 0;
}

static struct gimple_opt_pass dummy_pass = {
    .pass = {
        .type = GIMPLE_PASS,
        .name = "dummy-coverage-pass",
        .optinfo_flags = OPTGROUP_NONE,
        .tv_id = TV_NONE,
        .properties_required = 0,
        .properties_provided = 0,
        .properties_destroyed = 0,
        .todo_flags_start = 0,
        .todo_flags_finish = 0,
        .execute = dummy_pass_execute,
    }
};

static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* 2. For PLUGIN_INFO: Plugin information structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "This plugin triggers uncovered code in GCC's plugin infrastructure"
};

/* 3. For PLUGIN_REGISTER_GGC_ROOTS: GGC root table */
static const struct ggc_root_tab dummy_roots[] = {
    {
        .base = (void *)&dummy_pass,
        .nelt = sizeof(dummy_pass) / sizeof(void *),
        .stride = sizeof(void *),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PART 2: Plugin initialization function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: Incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s: Initializing to trigger uncovered code...\n", plugin_name);
    
    /* ============================================
       Register callbacks for the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback function needed - infrastructure handles it */
                     &pass_info);
    
    /* 2. Register PLUGIN_INFO event */
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,
                     &plugin_info_data);
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,
                     dummy_roots);
    
    /* Additional callback to verify plugin is active during compilation */
    register_callback(plugin_name,
                     PLUGIN_PRE_GENERICIZE,
                     NULL,  /* We could add a simple callback here if needed */
                     NULL);
    
    printf("Plugin %s: All three target events registered successfully\n", plugin_name);
    
    return 0;  /* Success */
}

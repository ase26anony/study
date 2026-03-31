/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets:
 * - PLUGIN_PASS_MANAGER_SETUP
 * - PLUGIN_INFO
 * - PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass my_pass;
static struct register_pass_info my_pass_info;
static struct plugin_info my_plugin_info;
static struct ggc_root_tab my_ggc_roots[2];

/* ============================================
 * 1. Dummy GIMPLE Pass for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Dummy execute function for our pass */
static unsigned int
my_pass_execute (void)
{
    /* This pass does nothing, just exists to be registered */
    return 0;
}

/* Define the pass structure */
static struct opt_pass my_pass = 
{
    .type = GIMPLE_PASS,           /* GIMPLE pass */
    .name = "my-dummy-pass",       /* Pass name */
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .execute = my_pass_execute,    /* Execution function */
};

/* Pass registration info */
static struct register_pass_info my_pass_info = 
{
    .pass = &my_pass,              /* Pointer to our pass */
    .reference_pass_name = "cfg",  /* Insert after CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER  /* Insert after reference pass */
};

/* ============================================
 * 2. Plugin Info for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = 
{
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, "
            "and PLUGIN_REGISTER_GGC_ROOTS events"
};

/* ============================================
 * 3. GGC Root Table for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy structure for GGC roots */
static struct dummy_ggc_struct 
{
    int x;
    tree t;
} dummy_ggc_data;

/* GGC root table with one dummy entry */
static struct ggc_root_tab my_ggc_roots[2] = 
{
    {
        .base = (void *)&dummy_ggc_data,  /* Base pointer */
        .nelt = 1,                        /* Number of elements */
        .stride = sizeof(struct dummy_ggc_struct), /* Size of each element */
        .cb = NULL,                       /* No callback */
        .pchw = NULL                      /* No PCH handling */
    },
    {
        NULL, 0, 0, NULL, NULL  /* Terminator entry */
    }
};

/* ============================================
 * Main Plugin Initialization Function
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
    {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
     * Register callbacks for the three target events
     * ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    register_callback (
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback needed - infrastructure handles it */
        &my_pass_info
    );
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP\n");
    
    /* 2. Register PLUGIN_INFO event */
    register_callback (
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback needed - infrastructure handles it */
        &my_plugin_info
    );
    printf("  Registered PLUGIN_INFO\n");
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback (
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback needed - infrastructure handles it */
        my_ggc_roots
    );
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    /* Optional: Register additional events to ensure plugin is called */
    register_callback (
        plugin_name,
        PLUGIN_START_PARSE_FUNCTION,
        NULL,  /* Dummy callback */
        NULL
    );
    
    printf("Plugin %s initialization complete\n", plugin_name);
    return 0;
}

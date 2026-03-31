/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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

/* Required for GCC plugin compatibility */
int plugin_is_GPL_compatible = 1;

/*---------------------------------------------------------------------
 * 1. For PLUGIN_PASS_MANAGER_SETUP: Create a dummy pass
 *---------------------------------------------------------------------*/

/* Simple dummy pass structure */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder */
    return 0;
}

static struct gimple_opt_pass dummy_pass = 
{
    {
        GIMPLE_PASS,
        "dummy-coverage-pass",           /* name */
        OPTGROUP_NONE,                   /* optinfo_flags */
        NULL,                            /* gate */
        dummy_pass_execute,              /* execute */
        NULL,                            /* sub */
        NULL,                            /* next */
        0,                               /* static_pass_number */
        TV_NONE,                         /* tv_id */
        0,                               /* properties_required */
        0,                               /* properties_provided */
        0,                               /* properties_destroyed */
        0,                               /* todo_flags_start */
        0                                /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info dummy_pass_info = 
{
    .pass = &dummy_pass.pass,           /* Reference to the pass */
    .reference_pass_name = "cfg",       /* Insert after CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER     /* Position: after reference pass */
};

/*---------------------------------------------------------------------
 * 2. For PLUGIN_INFO: Create plugin info structure
 *---------------------------------------------------------------------*/
static struct plugin_info plugin_metadata = 
{
    .version = "1.0-coverage",
    .help = "GCC plugin to trigger uncovered code in plugin.cc\n"
            "Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS"
};

/*---------------------------------------------------------------------
 * 3. For PLUGIN_REGISTER_GGC_ROOTS: Create GGC root table
 *---------------------------------------------------------------------*/

/* Dummy type for GGC roots */
struct dummy_ggc_type {
    int id;
    void *data;
};

/* Static instance for GGC root */
static struct dummy_ggc_type dummy_ggc_instance = {0, NULL};

/* GGC root table - must be NULL-terminated */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_instance,
        .nelt = 1,
        .stride = sizeof(struct dummy_ggc_type),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
    { NULL, 0, 0, NULL, NULL }
};

/*---------------------------------------------------------------------
 * Plugin initialization function
 *---------------------------------------------------------------------*/
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: Incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Coverage Plugin: Initializing %s\n", plugin_name);
    
    /*-----------------------------------------------------------------
     * Register callback for PLUGIN_PASS_MANAGER_SETUP
     * This triggers: case PLUGIN_PASS_MANAGER_SETUP in plugin.cc
     *-----------------------------------------------------------------*/
    register_callback(plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback function needed for registration */
                     &dummy_pass_info);
    
    /*-----------------------------------------------------------------
     * Register callback for PLUGIN_INFO
     * This triggers: case PLUGIN_INFO in plugin.cc
     *-----------------------------------------------------------------*/
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* No callback function needed */
                     &plugin_metadata);
    
    /*-----------------------------------------------------------------
     * Register callback for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers: case PLUGIN_REGISTER_GGC_ROOTS in plugin.cc
     *-----------------------------------------------------------------*/
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* No callback function needed */
                     (void *)dummy_ggc_roots);
    
    printf("Coverage Plugin: All three target events registered\n");
    
    return 0; /* Success */
}

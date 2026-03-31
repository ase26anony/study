/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing, just return */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always run this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = 
{
    {
        GIMPLE_PASS,
        "dummy-coverage-pass",           /* name */
        OPTGROUP_NONE,                   /* optinfo_flags */
        dummy_pass_gate,                 /* gate */
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
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,           /* Reference to our dummy pass */
    .reference_pass_name = "cfg",       /* Insert after the CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER     /* Position: insert after reference pass */
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info plugin_info_data = {
    .version = "1.0-coverage",
    .help = "GCC plugin to trigger coverage of plugin.cc lines 458-470\n"
            "This plugin registers dummy components to exercise:\n"
            "1. PLUGIN_PASS_MANAGER_SETUP\n"
            "2. PLUGIN_INFO\n"
            "3. PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree dummy_ggc_tree = NULL_TREE;
static GTY(()) int dummy_ggc_int_array[10];

/* GGC root table with one dummy entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_tree,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_tree),
        .cb = NULL,
        .pchw = NULL
    },
    {
        .base = (void *)&dummy_ggc_int_array,
        .nelt = sizeof(dummy_ggc_int_array) / sizeof(dummy_ggc_int_array[0]),
        .stride = sizeof(dummy_ggc_int_array[0]),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage Plugin: Initializing plugin '%s'\n", plugin_name);
    
    /* ============================================
       Register PLUGIN_PASS_MANAGER_SETUP event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_PASS_MANAGER_SETUP...\n");
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback needed - infrastructure handles it */
                      &pass_info);
    
    /* ============================================
       Register PLUGIN_INFO event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_INFO...\n");
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback needed - infrastructure handles it */
                      &plugin_info_data);
    
    /* ============================================
       Register PLUGIN_REGISTER_GGC_ROOTS event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_REGISTER_GGC_ROOTS...\n");
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback needed - infrastructure handles it */
                      dummy_ggc_roots);
    
    printf("Coverage Plugin: All events registered successfully\n");
    
    return 0;  /* Success */
}

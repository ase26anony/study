/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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
int plugin_is_GPL_compatible;

/* Global plugin name */
static const char *plugin_name = "coverage_plugin";

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* This pass does nothing, just for demonstration */
    return 0;
}

static struct gimple_opt_pass dummy_pass = 
{
    {
        GIMPLE_PASS,
        "dummy_pass",               /* name */
        OPTGROUP_NONE,              /* optinfo_flags */
        NULL,                       /* gate */
        dummy_pass_execute,         /* execute */
        NULL,                       /* sub */
        NULL,                       /* next */
        0,                          /* static_pass_number */
        TV_NONE,                    /* tv_id */
        0,                          /* properties_required */
        0,                          /* properties_provided */
        0,                          /* properties_destroyed */
        0,                          /* todo_flags_start */
        0                           /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,       /* Reference to our dummy pass */
    .reference_pass_name = "cfg",   /* Insert after CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

/* Plugin info structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "GCC plugin for coverage testing of plugin infrastructure\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO,\n"
            "and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root structure for testing */
static tree dummy_tree = NULL_TREE;

/* GGC root table with one dummy entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminating NULL entry as required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int ret = 0;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    /* Store plugin name from plugin_info_args */
    plugin_name = plugin_info_args->base_name;
    
    /* ============================================
       Register PLUGIN_PASS_MANAGER_SETUP callback
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback function needed for registration */
        &pass_info
    );
    
    if (ret != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", 
                plugin_name);
        return ret;
    }
    
    /* ============================================
       Register PLUGIN_INFO callback
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback function needed for registration */
        &plugin_info_data
    );
    
    if (ret != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return ret;
    }
    
    /* ============================================
       Register PLUGIN_REGISTER_GGC_ROOTS callback
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback function needed for registration */
        dummy_ggc_roots
    );
    
    if (ret != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", 
                plugin_name);
        return ret;
    }
    
    /* Optional: Register additional callbacks to verify plugin is working */
    ret = register_callback(
        plugin_name,
        PLUGIN_START_PARSE_FUNCTION,
        NULL,  /* Dummy callback */
        NULL
    );
    
    fprintf(stderr, "%s: Successfully initialized and registered all target events\n",
            plugin_name);
    
    return 0;
}

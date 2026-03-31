/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Required for GCC plugin compatibility */
int plugin_is_GPL_compatible = 1;

/* ============================================
 * 1. Data for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Simple dummy pass structure */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder */
    return 0;
}

static struct gimple_opt_pass dummy_pass = 
{
    {
        GIMPLE_PASS,          /* type */
        "dummy-pass",         /* name */
        OPTGROUP_NONE,        /* optinfo_flags */
        NULL,                 /* gate */
        dummy_pass_execute,   /* execute */
        NULL,                 /* sub */
        NULL,                 /* next */
        0,                    /* static_pass_number */
        TV_NONE,              /* tv_id */
        0,                    /* properties_required */
        0,                    /* properties_provided */
        0,                    /* properties_destroyed */
        0,                    /* todo_flags_start */
        0                     /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,           /* Pointer to our pass */
    .reference_pass_name = "cfg",       /* Insert after "cfg" pass */
    .ref_pass_instance_number = 1,      /* First instance */
    .pos_op = PASS_POS_INSERT_AFTER     /* Insert after reference pass */
};

/* ============================================
 * 2. Data for PLUGIN_INFO
 * ============================================ */

static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
};

/* ============================================
 * 3. Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_pass,    /* Base pointer */
        .nelt = 1,                      /* Number of elements */
        .stride = sizeof(dummy_pass),   /* Size of each element */
        .cb = NULL,                     /* No callback */
        .pchw = NULL                    /* No PCH handling */
    },
    { NULL, 0, 0, NULL, NULL }          /* Terminator */
};

/* ============================================
 * Plugin Initialization Function
 * ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
     * Register callbacks for the three target events
     * ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback needed - infrastructure handles it */
                      &pass_info);
    
    /* 2. Register PLUGIN_INFO */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback needed */
                      &plugin_info_data);
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback needed */
                      dummy_ggc_root_tab);
    
    printf("Plugin %s registered all three target events\n", plugin_name);
    
    return 0; /* Success */
}

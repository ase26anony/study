/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Define a simple dummy pass */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always run this pass */
    return true;
}

/* Create the dummy pass structure */
static struct gimple_opt_pass dummy_pass = 
{
    {
        GIMPLE_PASS,
        "dummy-pass",           /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        dummy_pass_gate,        /* gate */
        dummy_pass_execute,     /* execute */
        NULL,                   /* sub */
        NULL,                   /* next */
        0,                      /* static_pass_number */
        TV_NONE,                /* tv_id */
        0,                      /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    }
};

/* Create register_pass_info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,           /* Pointer to our pass */
    .reference_pass_name = "cfg",       /* Insert after the 'cfg' pass */
    .ref_pass_instance_number = 1,      /* First instance */
    .pos_op = PASS_POS_INSERT_AFTER     /* Insert after the reference pass */
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

/* Create plugin_info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Coverage plugin for testing GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Define a dummy GGC root table */
static const struct ggc_root_tab dummy_ggc_roots[] = {
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
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage plugin initializing: %s\n", plugin_name);
    
    /* ============================================
       Register callbacks for the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP callback */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,                    /* No callback function needed */
        &pass_info               /* User data for registration */
    );
    
    /* 2. Register PLUGIN_INFO callback */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,                    /* No callback function needed */
        &plugin_info_data        /* User data for registration */
    );
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS callback */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,                    /* No callback function needed */
        dummy_ggc_roots          /* User data for registration */
    );
    
    /* Additional callback to verify plugin is working */
    register_callback(
        plugin_name,
        PLUGIN_START_UNIT,
        NULL,                    /* Simple callback to print message */
        NULL
    );
    
    printf("Coverage plugin successfully registered all callbacks\n");
    
    return 0;  /* Success */
}

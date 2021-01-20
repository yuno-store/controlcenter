/****************************************************************************
 *          MAIN_CONTROLCENTER.C
 *          controlcenter main
 *
 *          Control Center of Yuneta Systems
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ****************************************************************************/
#include <yuneta_tls.h>
#include "c_controlcenter.h"
#include "yuno_controlcenter.h"

/***************************************************************************
 *                      Names
 ***************************************************************************/
#define APP_NAME        ROLE_CONTROLCENTER
#define APP_DOC         "Control Center of Yuneta Systems"

#define APP_VERSION     "4.6.11"
#define APP_SUPPORT     "<niyamaka at yuneta.io>"
#define APP_DATETIME    __DATE__ " " __TIME__

/***************************************************************************
 *                      Default config
 ***************************************************************************/
PRIVATE char fixed_config[]= "\
{                                                                   \n\
    'yuno': {                                                       \n\
        'yuno_role': '"ROLE_CONTROLCENTER"',                        \n\
        'tags': ['yuneta', 'utils']                                 \n\
    }                                                               \n\
}                                                                   \n\
";
PRIVATE char variable_config[]= "\
{                                                                   \n\
    'environment': {                                                \n\
        'use_system_memory': true,                                  \n\
        'log_gbmem_info': true,                                     \n\
        'MEM_MIN_BLOCK': 512,                                       \n\
        'MEM_MAX_BLOCK': 209715200,             #^^  200*M          \n\
        'MEM_SUPERBLOCK': 209715200,            #^^  200*M          \n\
        'MEM_MAX_SYSTEM_MEMORY': 2147483648,    #^^ 2*G             \n\
        'console_log_handlers': {                                   \n\
            'to_stdout': {                                          \n\
                'handler_type': 'stdout',                           \n\
                'handler_options': 255                              \n\
            }                                                       \n\
        },                                                          \n\
        'daemon_log_handlers': {                                    \n\
            'to_file': {                                            \n\
                'handler_type': 'file',                             \n\
                'filename_mask': 'controlcenter-MM-DD.log',         \n\
                'handler_options': 255                              \n\
            },                                                      \n\
            'to_udp': {                                             \n\
                'handler_type': 'udp',                              \n\
                'url': 'udp://127.0.0.1:1992',                      \n\
                'handler_options': 255                              \n\
            }                                                       \n\
        }                                                           \n\
    },                                                              \n\
    'yuno': {                                                       \n\
        'required_services': [],                                    \n\
        'public_services': [],                                      \n\
        'service_descriptor': {                                     \n\
        },                                                          \n\
        'trace_levels': {                                           \n\
            'Tcp0': ['connections'],                                \n\
            'TcpS0': ['listen', 'not-accepted', 'accepted'],        \n\
            'Tcp1': ['connections'],                                \n\
            'TcpS1': ['listen', 'not-accepted', 'accepted']         \n\
        }                                                           \n\
    },                                                              \n\
    'global': {                                                     \n\
        'Authz.max_sessions_per_user': 4,                           \n\
        'Authz.initial_load': {                                     \n\
            'roles': [                                              \n\
                {                                                   \n\
                    'id': 'root',                                   \n\
                    'disabled': false,                              \n\
                    'description': 'Super-Owner of system',         \n\
                    'realm_id': '*',                                \n\
                    'service': '*'                                  \n\
                },                                                  \n\
                {                                                   \n\
                    'id': 'owner',                                  \n\
                    'disabled': false,                              \n\
                    'description': 'Owner of system',               \n\
                    'realm_id': '(^^__realm_id__^^)',               \n\
                    'service': '*'                                  \n\
                },                                                  \n\
                {                                                   \n\
                    'id': 'manage-authz',                           \n\
                    'disabled': false,                              \n\
                    'description': 'Management of Authz',           \n\
                    'realm_id': '(^^__realm_id__^^)',               \n\
                    'service': 'treedb_authz'                       \n\
                },                                                  \n\
                {                                                   \n\
                    'id': 'manage-controlcenter',                   \n\
                    'disabled': false,                              \n\
                    'description': 'Management of Controlcenter',   \n\
                    'realm_id': '(^^__realm_id__^^)',               \n\
                    'service': 'treedb_controlcenter'               \n\
                }                                                   \n\
            ],                                                      \n\
            'users': [                                              \n\
                {                                                   \n\
                    'id': 'yuneta',                                 \n\
                    'roles': [                                      \n\
                        'roles^root^users',                         \n\
                        'roles^owner^users'                         \n\
                    ]                                               \n\
                },                                                  \n\
                {                                                   \n\
                    'id': 'ginsmar@mulesol.es',                     \n\
                    'roles': [                                      \n\
                        'roles^manage-controlcenter^users',         \n\
                        'roles^manage-authz^users',                 \n\
                        'roles^owner^users'                         \n\
                    ]                                               \n\
                },                                                  \n\
                {                                                   \n\
                    'id': 'cochoa@mulesol.es',                      \n\
                    'roles': [                                      \n\
                        'roles^manage-controlcenter^users',         \n\
                        'roles^manage-authz^users',                 \n\
                        'roles^owner^users'                         \n\
                    ]                                               \n\
                }                                                   \n\
            ]                                                       \n\
        },                                                          \n\
        'Authz.jwt_public_key': '-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAj0ZkOtmWlsDLJiJWXTEJ\\ntyXxlVY7iLseG982eaFgDaAdtE3Z5J+WDvzni7v4MPR55oMyi/oeAvTKIsVOv3aw\\nobRJ/Mr45Qh6I0j4Hn+rFfPW4wbmxRmFeyRrfMzYAZZoibZ3m7EFlj2RINvJFIgE\\npIoTf4UneXmlSDbUU9MTZe1mULfCfEZVa5V9W86BluAAib1mYJU7aJ20KPkbQAvW\\nXqC82AE9ga66HnQ2n56mv1kPyvNGKvvM6vD2IXQeLIYgudYT2KlGKd8isrOEkrno\\nXtPKMSaRhVccO73Wbo7krhjGV5MTpMvvOM+wDprslFkODm0MORsHORVxfcVGWSpU\\ngQIDAQAB\\n-----END PUBLIC KEY-----\\n'  \n\
    },                                                              \n\
    'services': [                                                   \n\
        {                                                           \n\
            'name': 'controlcenter',                                \n\
            'gclass': 'Controlcenter',                              \n\
            'default_service': true,                                \n\
            'autostart': true,                                      \n\
            'autoplay': false,                                      \n\
            'kw': {                                                 \n\
            },                                                      \n\
            'zchilds': [                                            \n\
            ]                                                       \n\
        },                                                          \n\
        {                                                           \n\
            'name': 'authz',                                        \n\
            'gclass': 'Authz',                                      \n\
            'default_service': false,                               \n\
            'autostart': true,                                      \n\
            'autoplay': true,                                       \n\
            'kw': {                                                 \n\
            },                                                      \n\
            'zchilds': [                                            \n\
            ]                                                       \n\
        }                                                           \n\
    ]                                                               \n\
}                                                                   \n\
";

/***************************************************************************
 *                      Register
 ***************************************************************************/
static void register_yuno_and_more(void)
{
    /*------------------------*
     *  Register yuneta-tls
     *------------------------*/
    yuneta_register_c_tls();

    /*-------------------*
     *  Register yuno
     *-------------------*/
    register_yuno_controlcenter();

    /*--------------------*
     *  Register service
     *--------------------*/
    gobj_register_gclass(GCLASS_CONTROLCENTER);
}

/***************************************************************************
 *                      Main
 ***************************************************************************/
int main(int argc, char *argv[])
{
    /*------------------------------------------------*
     *  To trace memory
     *------------------------------------------------*/
#ifdef DEBUG
    static uint32_t mem_list[] = {16336, 0};
    gbmem_trace_alloc_free(0, mem_list);
#endif

    /*------------------------------------------------*
     *          Traces
     *------------------------------------------------*/
    gobj_set_gclass_no_trace(GCLASS_TIMER, "machine", TRUE);  // Avoid timer trace, too much information

    gobj_set_gclass_trace(GCLASS_IEVENT_SRV, "identity-card", TRUE);
    gobj_set_gclass_trace(GCLASS_IEVENT_CLI, "identity-card", TRUE);

// Samples
//     gobj_set_gclass_trace(GCLASS_IEVENT_CLI, "ievents2", TRUE);
    gobj_set_gclass_trace(GCLASS_IEVENT_SRV, "ievents2", TRUE); // TODO TEST
//     gobj_set_gclass_trace(GCLASS_CONTROLCENTER, "messages", TRUE);

//     gobj_set_gobj_trace(0, "create_delete", TRUE, 0);
//     gobj_set_gobj_trace(0, "create_delete2", TRUE, 0);
//     gobj_set_gobj_trace(0, "start_stop", TRUE, 0);
//     gobj_set_gobj_trace(0, "subscriptions", TRUE, 0);
//     gobj_set_gobj_trace(0, "machine", TRUE, 0);
//     gobj_set_gobj_trace(0, "ev_kw", TRUE, 0);
//     gobj_set_gobj_trace(0, "libuv", TRUE, 0);

    //set_auto_kill_time(5);

    /*------------------------------------------------*
     *          Start yuneta
     *------------------------------------------------*/
    helper_quote2doublequote(fixed_config);
    helper_quote2doublequote(variable_config);
    yuneta_setup(
        dbattrs_startup,            // dbsimple2
        dbattrs_end,                // dbsimple2
        dbattrs_load_persistent,    // dbsimple2
        dbattrs_save_persistent,    // dbsimple2
        dbattrs_remove_persistent,  // dbsimple2
        dbattrs_list_persistent,    // dbsimple2
        command_parser,
        stats_parser,
        authz_checker,              // Monoclass Authz
        authenticate_parser         // Monoclass Authz
    );
    return yuneta_entry_point(
        argc, argv,
        APP_NAME, APP_VERSION, APP_SUPPORT, APP_DOC, APP_DATETIME,
        fixed_config,
        variable_config,
        register_yuno_and_more
    );
}

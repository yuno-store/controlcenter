/***********************************************************************
 *          C_CONTROLCENTER.C
 *          Controlcenter GClass.
 *
 *          Control Center of Yuneta Systems
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ***********************************************************************/
#include <grp.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "c_controlcenter.h"

#include "treedb_schema_controlcenter.c"

/***************************************************************************
 *              Constants
 ***************************************************************************/

/***************************************************************************
 *              Structures
 ***************************************************************************/

/***************************************************************************
 *              Prototypes
 ***************************************************************************/

/***************************************************************************
 *          Data: config, public data, private data
 ***************************************************************************/
PRIVATE json_t *cmd_help(hgobj gobj, const char *cmd, json_t *kw, hgobj src);
PRIVATE json_t *cmd_authzs(hgobj gobj, const char *cmd, json_t *kw, hgobj src);
PRIVATE json_t *cmd_list_agents(hgobj gobj, const char *cmd, json_t *kw, hgobj src);
PRIVATE json_t *cmd_command_agent(hgobj gobj, const char *cmd, json_t *kw, hgobj src);

PRIVATE sdata_desc_t pm_help[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "cmd",          0,              0,          "command about you want help."),
SDATAPM (ASN_UNSIGNED,  "level",        0,              0,          "command search level in childs"),
SDATA_END()
};
PRIVATE sdata_desc_t pm_authzs[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "authz",        0,              0,          "permission to search"),
SDATAPM (ASN_OCTET_STR, "service",      0,              0,          "Service where to search the permission. If empty print all service's permissions"),
SDATA_END()
};
PRIVATE sdata_desc_t pm_list_agents[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_UNSIGNED,  "expand",        0,              0,          "Expand details"),
SDATA_END()
};
PRIVATE sdata_desc_t pm_command_agent[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "agent_id",     0,              0,          "agent id"),
SDATAPM (ASN_OCTET_STR, "cmd2agent",    0,              0,          "command to agent"),
SDATA_END()
};
PRIVATE sdata_desc_t pm_write_tty[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "agent_id",     0,              0,          "agent id"),
SDATAPM (ASN_OCTET_STR, "content64",    0,              0,          "Content64 data to write to tty"),
SDATA_END()
};

PRIVATE const char *a_help[] = {"h", "?", 0};
PRIVATE const char *a_write_tty[] = {"EV_WRITE_TTY", 0};

PRIVATE sdata_desc_t command_table[] = {
/*-CMD---type-----------name----------------alias---------------items-----------json_fn---------description---------- */
SDATACM (ASN_SCHEMA,    "help",             a_help,             pm_help,        cmd_help,       "Command's help"),
SDATACM2 (ASN_SCHEMA,   "authzs",           0,                  0,              pm_authzs,      cmd_authzs,     "Authorization's help"),
SDATACM (ASN_SCHEMA,    "list-agents",      0,                  pm_list_agents, cmd_list_agents, "List connected agents"),
SDATACM2 (ASN_SCHEMA,   "command-agent",    SDF_WILD_CMD,       0,                  pm_command_agent,cmd_command_agent,"Command to agent. WARNING: parameter's keys are not checked"),
SDATACM2 (ASN_SCHEMA,   "write-tty",        0,                  a_write_tty,    pm_write_tty,   0,              "Write data to tty (internal use)"),

SDATA_END()
};


/*---------------------------------------------*
 *      Attributes - order affect to oid's
 *---------------------------------------------*/
PRIVATE sdata_desc_t tattr_desc[] = {
/*-ATTR-type------------name----------------flag----------------default-----description---------- */
SDATA (ASN_OCTET_STR,   "__username__",     SDF_RD,             "",              "Username"),
SDATA (ASN_COUNTER64,   "txMsgs",           SDF_RD|SDF_PSTATS,  0,          "Messages transmitted"),
SDATA (ASN_COUNTER64,   "rxMsgs",           SDF_RD|SDF_RSTATS,  0,          "Messages receiveds"),

SDATA (ASN_COUNTER64,   "txMsgsec",         SDF_RD|SDF_RSTATS,  0,          "Messages by second"),
SDATA (ASN_COUNTER64,   "rxMsgsec",         SDF_RD|SDF_RSTATS,  0,          "Messages by second"),
SDATA (ASN_COUNTER64,   "maxtxMsgsec",      SDF_WR|SDF_RSTATS,  0,          "Max Tx Messages by second"),
SDATA (ASN_COUNTER64,   "maxrxMsgsec",      SDF_WR|SDF_RSTATS,  0,          "Max Rx Messages by second"),

SDATA (ASN_INTEGER,     "timeout",          SDF_RD,             1*1000,     "Timeout"),
SDATA (ASN_POINTER,     "user_data",        0,                  0,          "user data"),
SDATA (ASN_POINTER,     "user_data2",       0,                  0,          "more user data"),
SDATA_END()
};

/*---------------------------------------------*
 *      GClass trace levels
 *---------------------------------------------*/
enum {
    TRACE_MESSAGES = 0x0001,
};
PRIVATE const trace_level_t s_user_trace_level[16] = {
{"messages",        "Trace messages"},
{0, 0},
};

/*---------------------------------------------*
 *      GClass authz levels
 *---------------------------------------------*/

PRIVATE sdata_desc_t authz_table[] = {
/*-AUTHZ-- type---------name----------------flag----alias---items---description--*/
SDATAAUTHZ (ASN_SCHEMA, "list-agents",      0,      0,      0,      "Permission to list remote agents"),
SDATAAUTHZ (ASN_SCHEMA, "command-agent",    0,      0,      0,      "Permission to send command to remote agent"),
SDATAAUTHZ (ASN_SCHEMA, "write-tty",        0,      0,      0,      "Internal use. Feed remote consola from local keyboard"),
SDATA_END()
};

/*---------------------------------------------*
 *              Private data
 *---------------------------------------------*/
typedef struct _PRIVATE_DATA {
    hgobj timer;
    int32_t timeout;

    hgobj gobj_top_side;
    hgobj gobj_input_side;
    hgobj gobj_treedbs;
    hgobj gobj_treedb_controlcenter;

    uint64_t *ptxMsgs;
    uint64_t *prxMsgs;
    uint64_t txMsgsec;
    uint64_t rxMsgsec;
} PRIVATE_DATA;




            /******************************
             *      Framework Methods
             ******************************/




/***************************************************************************
 *      Framework Method create
 ***************************************************************************/
PRIVATE void mt_create(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    priv->timer = gobj_create(gobj_name(gobj), GCLASS_TIMER, 0, gobj);
    priv->ptxMsgs = gobj_danger_attr_ptr(gobj, "txMsgs");
    priv->prxMsgs = gobj_danger_attr_ptr(gobj, "rxMsgs");

    /*----------------------------------------*
     *  Check AUTHZS
     *----------------------------------------*/
    BOOL is_yuneta = FALSE;
    struct passwd *pw = getpwuid(getuid());
    if(strcmp(pw->pw_name, "yuneta")==0) {
        gobj_write_str_attr(gobj, "__username__", "yuneta");
        is_yuneta = TRUE;
    } else {
        static gid_t groups[30]; // HACK to use outside
        int ngroups = sizeof(groups)/sizeof(groups[0]);

        getgrouplist(pw->pw_name, 0, groups, &ngroups);
        for(int i=0; i<ngroups; i++) {
            struct group *gr = getgrgid(groups[i]);
            if(strcmp(gr->gr_name, "yuneta")==0) {
                gobj_write_str_attr(gobj, "__username__", "yuneta");
                is_yuneta = TRUE;
                break;
            }
        }
    }
    if(!is_yuneta) {
        trace_msg("User or group 'yuneta' is needed to run %s", gobj_yuno_role());
        printf("User or group 'yuneta' is needed to run %s\n", gobj_yuno_role());
        exit(0);
    }

    /*-------------------------------------------*
     *          Create Treedb System
     *-------------------------------------------*/
    char path[PATH_MAX];
    yuneta_realm_store_dir(
        path,
        sizeof(path),
        gobj_yuno_role(),
        gobj_yuno_realm_owner(),
        gobj_yuno_realm_id(),
        "",  // gclass-treedb controls the directories
        TRUE
    );
    json_t *kw_treedbs = json_pack("{s:s, s:s, s:b, s:i, s:i, s:i}",
        "path", path,
        "filename_mask", "%Y",  // to management treedbs we don't need multifiles (per day)
        "master", 1,
        "xpermission", 02770,
        "rpermission", 0660,
        "exit_on_error", LOG_OPT_EXIT_ZERO
    );
    priv->gobj_treedbs = gobj_create_service(
        "treedbs",
        GCLASS_TREEDB,
        kw_treedbs,
        gobj
    );

    /*
     *  HACK pipe inheritance
     */
    gobj_set_bottom_gobj(gobj, priv->gobj_treedbs);

    /*
     *  Do copy of heavy used parameters, for quick access.
     *  HACK The writable attributes must be repeated in mt_writing method.
     */
    SET_PRIV(timeout,               gobj_read_int32_attr)
}

/***************************************************************************
 *      Framework Method writing
 ***************************************************************************/
PRIVATE void mt_writing(hgobj gobj, const char *path)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    IF_EQ_SET_PRIV(timeout,             gobj_read_int32_attr)
    END_EQ_SET_PRIV()
}

/***************************************************************************
 *      Framework Method destroy
 ***************************************************************************/
PRIVATE void mt_destroy(hgobj gobj)
{
}

/***************************************************************************
 *      Framework Method start
 ***************************************************************************/
PRIVATE int mt_start(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*
     *  Start treedbs
     */
    gobj_subscribe_event(priv->gobj_treedbs, 0, 0, gobj);
    gobj_start_tree(priv->gobj_treedbs);

    gobj_start(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method stop
 ***************************************************************************/
PRIVATE int mt_stop(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*
     *  Stop treedbs
     */
    if(priv->gobj_treedbs) {
        gobj_unsubscribe_event(priv->gobj_treedbs, 0, 0, gobj);
        gobj_stop_tree(priv->gobj_treedbs);
    }

    gobj_stop(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method play
 *  Yuneta rule:
 *  If service has mt_play then start only the service gobj.
 *      (Let mt_play be responsible to start their tree)
 *  If service has not mt_play then start the tree with gobj_start_tree().
 ***************************************************************************/
PRIVATE int mt_play(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*---------------------------------------*
     *      Open treedb controlcenter
     *---------------------------------------*/
    helper_quote2doublequote(treedb_schema_controlcenter);
    json_t *jn_treedb_schema_controlcenter;
    jn_treedb_schema_controlcenter = legalstring2json(treedb_schema_controlcenter, TRUE);
    if(!jn_treedb_schema_controlcenter) {
        /*
         *  Exit if schema fails
         */
        exit(-1);
    }

    if(parse_schema(jn_treedb_schema_controlcenter)<0) {
        /*
         *  Exit if schema fails
         */
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_APP_ERROR,
            "msg",          "%s", "Parse schema fails",
            NULL
        );
        exit(-1);
    }

    const char *treedb_name = kw_get_str(
        jn_treedb_schema_controlcenter,
        "id",
        "treedb_controlcenter",
        KW_REQUIRED
    );

    json_t *kw_treedb = json_pack("{s:s, s:i, s:s, s:o}",
        "filename_mask", "%Y",
        "exit_on_error", 0,
        "treedb_name", treedb_name,
        "treedb_schema", jn_treedb_schema_controlcenter
    );
    json_t *jn_resp = gobj_command(priv->gobj_treedbs,
        "open-treedb",
        kw_treedb,
        gobj
    );
    int result = kw_get_int(jn_resp, "result", -1, KW_REQUIRED);
    if(result < 0) {
        const char *comment = kw_get_str(jn_resp, "comment", "", KW_REQUIRED);
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_APP_ERROR,
            "msg",          "%s", comment,
            NULL
        );
    }
    json_decref(jn_resp);

    priv->gobj_treedb_controlcenter = gobj_find_service("treedb_controlcenter", TRUE);

    /*---------------------------------------*
     *      Start __top_side__
     *---------------------------------------*/
    priv->gobj_top_side = gobj_find_service("__top_side__", TRUE);
    gobj_subscribe_event(priv->gobj_top_side, 0, 0, gobj);
    gobj_start_tree(priv->gobj_top_side);

    /*---------------------------------------*
     *      Start __input_side__
     *---------------------------------------*/
    priv->gobj_input_side = gobj_find_service("__input_side__", TRUE);
    gobj_subscribe_event(priv->gobj_input_side, 0, 0, gobj);
    gobj_start_tree(priv->gobj_input_side);

    return 0;
}

/***************************************************************************
 *      Framework Method pause
 ***************************************************************************/
PRIVATE int mt_pause(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*---------------------------------------*
     *      Stop __top_side__
     *---------------------------------------*/
    if(priv->gobj_top_side) {
        gobj_unsubscribe_event(priv->gobj_top_side, 0, 0, gobj);
        EXEC_AND_RESET(gobj_stop_tree, priv->gobj_top_side);
    }

    /*---------------------------------------*
     *      Input __input_side__
     *---------------------------------------*/
    if(priv->gobj_input_side) {
        gobj_unsubscribe_event(priv->gobj_input_side, 0, 0, gobj);
        EXEC_AND_RESET(gobj_stop_tree, priv->gobj_input_side);
    }

    /*---------------------------------------*
     *      Close treedb controlcenter
     *---------------------------------------*/
    json_decref(gobj_command(priv->gobj_treedbs,
        "close-treedb",
        json_pack("{s:s}",
            "treedb_name", "treedb_controlcenter"
        ),
        gobj
    ));
    priv->gobj_treedb_controlcenter = 0;

    clear_timeout(priv->timer);
    return 0;
}




            /***************************
             *      Commands
             ***************************/




/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *cmd_help(hgobj gobj, const char *cmd, json_t *kw, hgobj src)
{
    KW_INCREF(kw);
    json_t *jn_resp = gobj_build_cmds_doc(gobj, kw);
    return msg_iev_build_webix(
        gobj,
        0,
        jn_resp,
        0,
        0,
        kw  // owned
    );
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *cmd_authzs(hgobj gobj, const char *cmd, json_t *kw, hgobj src)
{
    return gobj_build_authzs_doc(gobj, cmd, kw, src);
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *cmd_list_agents(hgobj gobj, const char *cmd, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);
    int expand = kw_get_int(kw, "expand", 0, KW_WILD_NUMBER);

    /*----------------------------------------*
     *  Check AUTHZS
     *----------------------------------------*/
    const char *permission = "list-agents";
    if(!gobj_user_has_authz(gobj, permission, kw_incref(kw), src)) {
        return msg_iev_build_webix(
            gobj,
            -1,
            json_sprintf("No permission to '%s'", permission),
            0,
            0,
            kw  // owned
        );
    }

    /*----------------------------------------*
     *  Job
     *----------------------------------------*/
    json_t *jn_data = json_array();

    json_t *jn_filter = json_pack("{s:s, s:s}",
        "__gclass_name__", GCLASS_IEVENT_SRV_NAME,
        "__state__", "ST_SESSION"
    );
    dl_list_t *dl_childs = gobj_match_childs_tree(priv->gobj_input_side, 0, jn_filter);

    hgobj child; rc_instance_t *i_hs;
    i_hs = rc_first_instance(dl_childs, (rc_resource_t **)&child);
    while(i_hs) {
        json_t *jn_attrs = gobj_read_json_attr(child, "attrs");
        if(expand) {
            json_array_append(jn_data, jn_attrs);
        } else {
            json_array_append_new(jn_data,
                json_sprintf("UUID:%s, HOSTNAME:'%s'",
                    kw_get_str(jn_attrs, "id", "", 0),
                    kw_get_str(jn_attrs, "__md_iev__`ievent_gate_stack`0`host", "", 0)
                )
            );
        }
        i_hs = rc_next_instance(i_hs, (rc_resource_t **)&child);
    }

    rc_free_iter(dl_childs, TRUE, 0);

    return msg_iev_build_webix(gobj,
        0,
        0,
        0,
        jn_data,
        kw  // owned
    );
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *cmd_command_agent(hgobj gobj, const char *cmd, json_t *kw_, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);
    json_t *kw = json_deep_copy(kw_);
    KW_DECREF(kw_);

    /*----------------------------------------*
     *  Check AUTHZS
     *----------------------------------------*/
    const char *permission = "command-agent";
    if(!gobj_user_has_authz(gobj, permission, kw_incref(kw), src)) {
        return msg_iev_build_webix(
            gobj,
            -1,
            json_sprintf("No permission to '%s'", permission),
            0,
            0,
            kw  // owned
        );
    }

    /*----------------------------------------*
     *  Job
     *----------------------------------------*/
    const char *keys2delete[] = { // WARNING parameters of command-yuno command of agent
        "id",
        "command",
        "service",
        "realm_id",
        "yuno_role",
        "yuno_name",
        "yuno_release",
        "yuno_tag",
        "yuno_disabled",
        "yuno_running",
        0
    };
    for(int i=0; keys2delete[i]!=0; i++) {
        json_object_del(kw, keys2delete[i]);
    }

    const char *agent_id = kw_get_str(kw, "agent_id", "", 0);
    const char *cmd2agent = kw_get_str(kw, "cmd2agent", "", 0);

    if(empty_string(cmd2agent)) {
        return msg_iev_build_webix(gobj,
            -1,
            json_string("What cmd2agent?"),
            0,
            0,
            kw  // owned
        );
    }

    json_t *jn_filter = json_pack("{s:s, s:s}",
        "__gclass_name__", GCLASS_IEVENT_SRV_NAME,
        "__state__", "ST_SESSION"
    );
    dl_list_t *dl_childs = gobj_match_childs_tree(priv->gobj_input_side, 0, jn_filter);

    int some = 0;
    hgobj child; rc_instance_t *i_hs;
    i_hs = rc_first_instance(dl_childs, (rc_resource_t **)&child);
    while(i_hs) {
        json_t *jn_attrs = gobj_read_json_attr(child, "attrs");
        if(!empty_string(agent_id)) {
            const char *id_ = kw_get_str(jn_attrs, "id", "", 0);
            const char *host_ = kw_get_str(jn_attrs, "__md_iev__`ievent_gate_stack`0`host", "", 0);
            if(strcmp(id_, agent_id)!=0 || strcmp(id_, host_)!=0) {
                i_hs = rc_next_instance(i_hs, (rc_resource_t **)&child);
                continue;
            }
        }

        json_t *webix = gobj_command( // debe retornar siempre 0.
            child,
            cmd2agent,
            json_incref(kw),
            src
        );
        JSON_DECREF(webix);
        some++;

        i_hs = rc_next_instance(i_hs, (rc_resource_t **)&child);
    }

    rc_free_iter(dl_childs, TRUE, 0);


    return msg_iev_build_webix(gobj, // Asynchronous response too
        some?0:-1,
        json_sprintf("Command sent to %d nodes", some),
        0,
        0,
        kw  // owned
    );
}




            /***************************
             *      Local Methods
             ***************************/




            /***************************
             *      Actions
             ***************************/




/***************************************************************************
 *  Identity_card on from
 *      Web clients (__top_side__)
 ***************************************************************************/
PRIVATE int ac_on_open(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    if(src != priv->gobj_top_side && src != priv->gobj_input_side) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "on_open NOT from GOBJ_TOP_SIDE",
            "src",          "%s", gobj_full_name(src),
            NULL
        );
    }

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *  Identity_card off from
 *      Web clients (__top_side__)
 *      agent clients (__input_side__)
 ***************************************************************************/
PRIVATE int ac_on_close(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *  HACK nodo intermedio
 ***************************************************************************/
PRIVATE int ac_stats_yuno_answer(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    json_t *jn_ievent_id = msg_iev_pop_stack(kw, IEVENT_MESSAGE_AREA_ID);
    const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);

    hgobj gobj_requester = gobj_child_by_name(
        gobj_find_service("__top_side__", TRUE),
        dst_service,
        0
    );
    JSON_DECREF(jn_ievent_id);

    if(!gobj_requester) {
        // Debe venir del agent
        jn_ievent_id = msg_iev_get_stack(kw, IEVENT_MESSAGE_AREA_ID, 0);
        JSON_INCREF(jn_ievent_id);
        const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);
        gobj_requester = gobj_find_service(dst_service, TRUE);
    }

    if(!gobj_requester) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "service not found",
            "service",      "%s", dst_service,
            NULL
        );
        JSON_DECREF(jn_ievent_id);
        KW_DECREF(kw);
        return 0;
    }
    JSON_DECREF(jn_ievent_id);

    KW_INCREF(kw);
    json_t *kw_redirect = msg_iev_answer(gobj, kw, kw, 0); // "__answer__"

    return gobj_send_event(
        gobj_requester,
        event,
        kw_redirect,
        gobj
    );
}

/***************************************************************************
 *  HACK nodo intermedio
 ***************************************************************************/
PRIVATE int ac_command_yuno_answer(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    json_t *jn_ievent_id = msg_iev_pop_stack(kw, IEVENT_MESSAGE_AREA_ID);
    const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);

    hgobj gobj_requester = gobj_child_by_name(
        gobj_find_service("__top_side__", TRUE),
        dst_service,
        0
    );
    JSON_DECREF(jn_ievent_id);

    if(!gobj_requester) {
        // Debe venir del agent
        jn_ievent_id = msg_iev_get_stack(kw, IEVENT_MESSAGE_AREA_ID, 0);
        JSON_INCREF(jn_ievent_id);
        const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);
        gobj_requester = gobj_find_service(dst_service, TRUE);
    }

    if(!gobj_requester) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "service not found",
            "service",      "%s", dst_service,
            NULL
        );
        JSON_DECREF(jn_ievent_id);
        KW_DECREF(kw);
        return 0;
    }
    JSON_DECREF(jn_ievent_id);

    KW_INCREF(kw);
    json_t *kw_redirect = msg_iev_answer(gobj, kw, kw, 0); // "__answer__"

    return gobj_send_event(
        gobj_requester,
        event,
        kw_redirect,
        gobj
    );
}

/***************************************************************************
 *  HACK nodo intermedio
 ***************************************************************************/
PRIVATE int ac_tty_mirror_open(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    json_t *jn_ievent_id = msg_iev_pop_stack(kw, IEVENT_MESSAGE_AREA_ID);
    const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);

    hgobj gobj_requester = gobj_child_by_name(
        gobj_find_service("__top_side__", TRUE),
        dst_service,
        0
    );
    JSON_DECREF(jn_ievent_id);

    if(!gobj_requester) {
        // Debe venir del agent
        jn_ievent_id = msg_iev_get_stack(kw, IEVENT_MESSAGE_AREA_ID, 0);
        JSON_INCREF(jn_ievent_id);
        const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);
        gobj_requester = gobj_find_service(dst_service, TRUE);
    }

    if(!gobj_requester) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "service not found",
            "service",      "%s", dst_service,
            NULL
        );
        JSON_DECREF(jn_ievent_id);
        KW_DECREF(kw);
        return 0;
    }
    JSON_DECREF(jn_ievent_id);

    KW_INCREF(kw);
    json_t *kw_redirect = msg_iev_answer(gobj, kw, kw, 0); // "__answer__"

    return gobj_send_event(
        gobj_requester,
        event,
        kw_redirect,
        gobj
    );
}

/***************************************************************************
 *  HACK nodo intermedio
 ***************************************************************************/
PRIVATE int ac_tty_mirror_close(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    json_t *jn_ievent_id = msg_iev_pop_stack(kw, IEVENT_MESSAGE_AREA_ID);
    const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);

    hgobj gobj_requester = gobj_child_by_name(
        gobj_find_service("__top_side__", TRUE),
        dst_service,
        0
    );
    JSON_DECREF(jn_ievent_id);

    if(!gobj_requester) {
        // Debe venir del agent
        jn_ievent_id = msg_iev_get_stack(kw, IEVENT_MESSAGE_AREA_ID, 0);
        JSON_INCREF(jn_ievent_id);
        const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);
        gobj_requester = gobj_find_service(dst_service, TRUE);
    }

    if(!gobj_requester) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "service not found",
            "service",      "%s", dst_service,
            NULL
        );
        JSON_DECREF(jn_ievent_id);
        KW_DECREF(kw);
        return 0;
    }
    JSON_DECREF(jn_ievent_id);

    KW_INCREF(kw);
    json_t *kw_redirect = msg_iev_answer(gobj, kw, kw, 0); // "__answer__"

    return gobj_send_event(
        gobj_requester,
        event,
        kw_redirect,
        gobj
    );
}

/***************************************************************************
 *  HACK nodo intermedio
 ***************************************************************************/
PRIVATE int ac_tty_mirror_data(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    json_t *jn_ievent_id = msg_iev_pop_stack(kw, IEVENT_MESSAGE_AREA_ID);
    const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);

    hgobj gobj_requester = gobj_child_by_name(
        gobj_find_service("__top_side__", TRUE),
        dst_service,
        0
    );
    JSON_DECREF(jn_ievent_id);

    if(!gobj_requester) {
        // Debe venir del agent
        jn_ievent_id = msg_iev_get_stack(kw, IEVENT_MESSAGE_AREA_ID, 0);
        JSON_INCREF(jn_ievent_id);
        const char *dst_service = kw_get_str(jn_ievent_id, "dst_service", "", 0);
        gobj_requester = gobj_find_service(dst_service, TRUE);
    }

    if(!gobj_requester) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "service not found",
            "service",      "%s", dst_service,
            NULL
        );
        JSON_DECREF(jn_ievent_id);
        KW_DECREF(kw);
        return 0;
    }
    JSON_DECREF(jn_ievent_id);

    KW_INCREF(kw);
    json_t *kw_redirect = msg_iev_answer(gobj, kw, kw, 0); // "__answer__"

    return gobj_send_event(
        gobj_requester,
        event,
        kw_redirect,
        gobj
    );
}

/***************************************************************************
 *  HACK nodo intermedio, pero al reves(???)
 ***************************************************************************/
PRIVATE int ac_write_tty(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*----------------------------------------*
     *  Check AUTHZS TODO función interna para todos? marca con flag
     *----------------------------------------*/
    const char *permission = "write-tty";
    if(!gobj_user_has_authz(gobj, permission, kw_incref(kw), src)) {
        gobj_send_event(
            src,
            event,
                msg_iev_build_webix(
                gobj,
                -1,
                json_sprintf("No permission to '%s'", permission),
                0,
                0,
                kw  // owned
            ),
            gobj
        );
        KW_DECREF(kw);
        return 0;
    }

    /*----------------------------------------*
     *  Job
     *----------------------------------------*/

    const char *agent_id = kw_get_str(kw, "agent_id", "", 0);

    json_t *jn_filter = json_pack("{s:s, s:s}",
        "__gclass_name__", GCLASS_IEVENT_SRV_NAME,
        "__state__", "ST_SESSION"
    );
    dl_list_t *dl_childs = gobj_match_childs_tree(priv->gobj_input_side, 0, jn_filter);

    hgobj child; rc_instance_t *i_hs;
    i_hs = rc_first_instance(dl_childs, (rc_resource_t **)&child);
    while(i_hs) {
        json_t *jn_attrs = gobj_read_json_attr(child, "attrs");
        if(!empty_string(agent_id)) {
            const char *id_ = kw_get_str(jn_attrs, "id", "", 0);
            if(strcmp(id_, agent_id)!=0) {
                i_hs = rc_next_instance(i_hs, (rc_resource_t **)&child);
                continue;
            }

        }

        json_t *webix = gobj_command( // debe retornar siempre 0.
            child,
            "write-tty",
            json_incref(kw),
            src
        );
        JSON_DECREF(webix);

        i_hs = rc_next_instance(i_hs, (rc_resource_t **)&child);
    }

    rc_free_iter(dl_childs, TRUE, 0);

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE int ac_timeout(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    uint64_t maxtxMsgsec = gobj_read_uint64_attr(gobj, "maxtxMsgsec");
    uint64_t maxrxMsgsec = gobj_read_uint64_attr(gobj, "maxrxMsgsec");
    if(priv->txMsgsec > maxtxMsgsec) {
        gobj_write_uint64_attr(gobj, "maxtxMsgsec", priv->txMsgsec);
    }
    if(priv->rxMsgsec > maxrxMsgsec) {
        gobj_write_uint64_attr(gobj, "maxrxMsgsec", priv->rxMsgsec);
    }

    gobj_write_uint64_attr(gobj, "txMsgsec", priv->txMsgsec);
    gobj_write_uint64_attr(gobj, "rxMsgsec", priv->rxMsgsec);

    priv->rxMsgsec = 0;
    priv->txMsgsec = 0;

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *                          FSM
 ***************************************************************************/
PRIVATE const EVENT input_events[] = {
    // top input
    {"EV_MT_COMMAND_ANSWER",    EVF_PUBLIC_EVENT,   0,  0},
    {"EV_TTY_DATA",             EVF_PUBLIC_EVENT,   0, 0},
    {"EV_MT_STATS_ANSWER",      EVF_PUBLIC_EVENT,   0,  0},
    {"EV_WRITE_TTY",            0,  0,  0},
    {"EV_ON_OPEN",              0,  0,  0},
    {"EV_ON_CLOSE",             0,  0,  0},
    {"EV_TTY_OPEN",             EVF_PUBLIC_EVENT,   0, 0},
    {"EV_TTY_CLOSE",            EVF_PUBLIC_EVENT,   0, 0},
    // bottom input
    {"EV_TIMEOUT",              0,  0,  0},
    {"EV_STOPPED",              0,  0,  0},
    // internal
    {NULL, 0, 0, ""}
};
PRIVATE const EVENT output_events[] = {
    {NULL, 0, 0, ""}
};
PRIVATE const char *state_names[] = {
    "ST_IDLE",
    NULL
};

PRIVATE EV_ACTION ST_IDLE[] = {
    {"EV_MT_STATS_ANSWER",          ac_stats_yuno_answer,       0},
    {"EV_MT_COMMAND_ANSWER",        ac_command_yuno_answer,     0},
    {"EV_TTY_DATA",                 ac_tty_mirror_data,         0},
    {"EV_WRITE_TTY",                ac_write_tty,               0},
    {"EV_ON_OPEN",                  ac_on_open,                 0},
    {"EV_ON_CLOSE",                 ac_on_close,                0},
    {"EV_TTY_OPEN",                 ac_tty_mirror_open,         0},
    {"EV_TTY_CLOSE",                ac_tty_mirror_close,        0},
    {"EV_TIMEOUT",                  ac_timeout,                 0},
    {"EV_STOPPED",                  0,                          0},
    {0,0,0}
};

PRIVATE EV_ACTION *states[] = {
    ST_IDLE,
    NULL
};

PRIVATE FSM fsm = {
    input_events,
    output_events,
    state_names,
    states,
};

/***************************************************************************
 *              GClass
 ***************************************************************************/
/*---------------------------------------------*
 *              Local methods table
 *---------------------------------------------*/
PRIVATE LMETHOD lmt[] = {
    {0, 0, 0}
};

/*---------------------------------------------*
 *              GClass
 *---------------------------------------------*/
PRIVATE GCLASS _gclass = {
    0,  // base
    GCLASS_CONTROLCENTER_NAME,
    &fsm,
    {
        mt_create,
        0, //mt_create2,
        mt_destroy,
        mt_start,
        mt_stop,
        mt_play,
        mt_pause,
        mt_writing,
        0, //mt_reading,
        0, //mt_subscription_added,
        0, //mt_subscription_deleted,
        0, //mt_child_added,
        0, //mt_child_removed,
        0, //mt_stats,
        0, //mt_command_parser,
        0, //mt_inject_event,
        0, //mt_create_resource,
        0, //mt_list_resource,
        0, //mt_update_resource,
        0, //mt_delete_resource,
        0, //mt_add_child_resource_link
        0, //mt_delete_child_resource_link
        0, //mt_get_resource
        0, //mt_authorization_parser,
        0, //mt_authenticate,
        0, //mt_list_childs,
        0, //mt_stats_updated,
        0, //mt_disable,
        0, //mt_enable,
        0, //mt_trace_on,
        0, //mt_trace_off,
        0, //mt_gobj_created,
        0, //mt_future33,
        0, //mt_future34,
        0, //mt_publish_event,
        0, //mt_publication_pre_filter,
        0, //mt_publication_filter,
        0, //mt_authz_checker,
        0, //mt_future39,
        0, //mt_create_node,
        0, //mt_update_node,
        0, //mt_delete_node,
        0, //mt_link_agents,
        0, //mt_future44,
        0, //mt_unlink_agents,
        0, //mt_topic_jtree,
        0, //mt_get_node,
        0, //mt_list_agents,
        0, //mt_shoot_snap,
        0, //mt_activate_snap,
        0, //mt_list_snaps,
        0, //mt_treedbs,
        0, //mt_treedb_topics,
        0, //mt_topic_desc,
        0, //mt_topic_links,
        0, //mt_topic_hooks,
        0, //mt_node_parents,
        0, //mt_node_childs,
        0, //mt_list_instances,
        0, //mt_node_tree,
        0, //mt_topic_size,
        0, //mt_future62,
        0, //mt_future63,
        0, //mt_future64
    },
    lmt,
    tattr_desc,
    sizeof(PRIVATE_DATA),
    authz_table,  // acl
    s_user_trace_level,
    command_table,  // command_table
    0,  // gcflag
};

/***************************************************************************
 *              Public access
 ***************************************************************************/
PUBLIC GCLASS *gclass_controlcenter(void)
{
    return &_gclass;
}

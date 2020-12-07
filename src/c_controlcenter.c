/***********************************************************************
 *          C_CONTROLCENTER.C
 *          Controlcenter GClass.
 *
 *          Control Center of Yuneta Systems
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ***********************************************************************/
#include <string.h>
#include <stdio.h>
#include <cjose/cjose.h>
#include <oauth2/oauth2.h>
#include <oauth2/mem.h>
#include <uuid/uuid.h>
#include "c_controlcenter.h"

/***************************************************************************
 *              Constants
 ***************************************************************************/

/***************************************************************************
 *              Structures
 ***************************************************************************/

/***************************************************************************
 *              Prototypes
 ***************************************************************************/
PRIVATE json_t *get_access_roles(
    hgobj gobj,
    json_t *token_roles  // not owned
);
PRIVATE void oauth2_log_callback(
    oauth2_log_sink_t *sink,
    const char *filename,
    unsigned long line,
    const char *function,
    oauth2_log_level_t level,
    const char *msg
);
PRIVATE int create_new_user(hgobj gobj, json_t *jwt_payload);


/***************************************************************************
 *          Data: config, public data, private data
 ***************************************************************************/
PRIVATE topic_desc_t db_fichador_desc[] = {
    // Topic Name,          Pkey            System Flag     Tkey        Topic Json Desc
    {"users_accesses",      "username",     sf_string_key,  "tm",       0},
    {0}
};

#include "schema_gest_controlcenter.c"

PRIVATE json_t *cmd_help(hgobj gobj, const char *cmd, json_t *kw, hgobj src);

PRIVATE sdata_desc_t pm_help[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "cmd",          0,              0,          "command about you want help."),
SDATAPM (ASN_UNSIGNED,  "level",        0,              0,          "command search level in childs"),
SDATA_END()
};

PRIVATE const char *a_help[] = {"h", "?", 0};

PRIVATE sdata_desc_t command_table[] = {
/*-CMD---type-----------name----------------alias---------------items-----------json_fn---------description---------- */
SDATACM (ASN_SCHEMA,    "help",             a_help,             pm_help,        cmd_help,       "Command's help"),
SDATA_END()
};


/*---------------------------------------------*
 *      Attributes - order affect to oid's
 *---------------------------------------------*/
PRIVATE sdata_desc_t tattr_desc[] = {
/*-ATTR-type------------name----------------flag----------------default-----description---------- */
SDATA (ASN_OCTET_STR,   "company",          SDF_RD,             "default",  "Company"),
SDATA (ASN_OCTET_STR,   "jwt_public_key",   SDF_RD,             0,          "JWT public key"),

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
 *              Private data
 *---------------------------------------------*/
typedef struct _PRIVATE_DATA {
    hgobj timer;
    int32_t timeout;

    hgobj gobj_top_side;
    hgobj treedb_gest;

    oauth2_log_t *oath2_log;
    oauth2_log_sink_t *oath2_sink;

    hgobj gobj_tranger;
    json_t *tranger;
    json_t *users_accesses;      // dict with users opened

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

    helper_quote2doublequote(schema_gest_controlcenter);

    /*
     *  Chequea schema fichador, exit si falla.
     */
    json_t *jn_schema_gest_controlcenter;
    jn_schema_gest_controlcenter = legalstring2json(schema_gest_controlcenter, TRUE);
    if(!jn_schema_gest_controlcenter) {
        exit(-1);
    }

    int level = OAUTH2_LOG_WARN;
    priv->oath2_sink = oauth2_log_sink_create(
        level,                  // oauth2_log_level_t level,
        oauth2_log_callback,    // oauth2_log_function_t callback,
        gobj                    // void *ctx
    );
    priv->oath2_log = oauth2_log_init(level, priv->oath2_sink);

    priv->timer = gobj_create(gobj_name(gobj), GCLASS_TIMER, 0, gobj);
    priv->ptxMsgs = gobj_danger_attr_ptr(gobj, "txMsgs");
    priv->prxMsgs = gobj_danger_attr_ptr(gobj, "rxMsgs");

    /*---------------------------*
     *  Create Timeranger
     *---------------------------*/
    const char *company = gobj_read_str_attr(gobj, "company");
    if(empty_string(company)) {
        log_critical(LOG_OPT_EXIT_ZERO,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_PARAMETER_ERROR,
            "msg",          "%s", "Attribute 'company' is REQUIRED!",
            NULL
        );
    }
    char path[PATH_MAX];
    snprintf(path, sizeof(path),
        "/yuneta/store/controlcenter/%s/%s",
        company,
        gobj_yuno_role_plus_name()
    );
    json_t *kw_tranger = json_pack("{s:s, s:s, s:b, s:i}",
        "path", path,
        "filename_mask", "%Y",
        "master", 1,
        "on_critical_error", (int)(LOG_OPT_EXIT_ZERO)
    );
    priv->gobj_tranger = gobj_create_service(
        "tranger",
        GCLASS_TRANGER,
        kw_tranger,
        gobj
    );

    /*----------------------*
     *  Create Treedb
     *----------------------*/
    const char *treedb_name = kw_get_str(
        jn_schema_gest_controlcenter,
        "id",
        "gest_controlcenter",
        KW_REQUIRED
    );
    json_t *kw_resource = json_pack("{s:s, s:o, s:i}",
        "treedb_name", treedb_name,
        "treedb_schema", jn_schema_gest_controlcenter,
        "exit_on_error", LOG_OPT_EXIT_ZERO
    );

    priv->treedb_gest = gobj_create_service(
        treedb_name,
        GCLASS_NODE,
        kw_resource,
        gobj
    );

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
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    EXEC_AND_RESET(oauth2_log_free, priv->oath2_log);
}

/***************************************************************************
 *      Framework Method start
 ***************************************************************************/
PRIVATE int mt_start(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    gobj_start(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method stop
 ***************************************************************************/
PRIVATE int mt_stop(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

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

    /*
     *  Start tranger/treedb
     */
    gobj_start(priv->gobj_tranger);
    priv->tranger = gobj_read_pointer_attr(priv->gobj_tranger, "tranger");
    gobj_write_pointer_attr(priv->treedb_gest, "tranger", priv->tranger);
    gobj_start(priv->treedb_gest);

    if(1) {
        /*---------------------------*
         *  Open topics as messages
         *  TODO crea gclass para trmsg
         *---------------------------*/
        trmsg_open_topics(
            priv->tranger,
            db_fichador_desc
        );

        /*
         *  To open users accesses
         */
        priv->users_accesses = trmsg_open_list(
            priv->tranger,
            "users_accesses",     // topic
            json_pack("{s:i}",  // filter
                "max_key_instances", 1
            )
        );
        {
            // FIX ERROR
            // WARNING ignora _sessions al cargar user_access,
            // se pueden haber salvado sesiones que son datos volatiles
            json_t *messages = trmsg_get_messages(priv->users_accesses);
            const char *k; json_t *msg;
            json_object_foreach(messages, k, msg) {
                json_t *active = kw_get_dict(msg, "active", 0, KW_REQUIRED);
                if(active) {
                    json_object_del(active, "_sessions");
                }
            }
        }
    }

    /*
     *  Start __top_side__
     */
    priv->gobj_top_side = gobj_find_service("__top_side__", TRUE);
    gobj_subscribe_event(priv->gobj_top_side, 0, 0, gobj);
    gobj_start_tree(priv->gobj_top_side);

    return 0;
}

/***************************************************************************
 *      Framework Method pause
 ***************************************************************************/
PRIVATE int mt_pause(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*
     *  Stop __top_side__
     */
    gobj_unsubscribe_event(priv->gobj_top_side, 0, 0, gobj);
    EXEC_AND_RESET(gobj_stop_tree, priv->gobj_top_side);

    /*
     *  Stop treeb/tranger
     */
    gobj_stop(priv->treedb_gest);
    gobj_stop(priv->gobj_tranger);
    priv->tranger = 0;

    clear_timeout(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method mt_authenticate
 ***************************************************************************/
PRIVATE json_t *mt_authenticate(hgobj gobj, const char *service, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    const char *jwt= kw_get_str(kw, "jwt", "", KW_REQUIRED);
    const char *pubkey = gobj_read_str_attr(gobj, "jwt_public_key");

    #define MY_CACHE_OPTIONS "options=max_entries%3D10"

    json_t *jwt_payload = NULL;
    oauth2_cfg_token_verify_t *verify = NULL;

    const char *rv = oauth2_cfg_token_verify_add_options(
        priv->oath2_log, &verify, "pubkey", pubkey,
        "verify.exp=skip&verify.cache." MY_CACHE_OPTIONS);
    if(rv != NULL) {
        return msg_iev_build_webix(
            gobj,
            -1,
            json_local_sprintf("Oauth failed"),
            0,
            0,
            kw  // owned
        );
    }

    if(!oauth2_token_verify(priv->oath2_log, verify, jwt, &jwt_payload)) {
        // TODO por aquí se pierde memoria
        JSON_DECREF(jwt_payload);
        oauth2_cfg_token_verify_free(priv->oath2_log, verify);
        return msg_iev_build_webix(
            gobj,
            -1,
            json_local_sprintf("Authentication rejected"),
            0,
            0,
            kw  // owned
        );
    }

    oauth2_cfg_token_verify_free(priv->oath2_log, verify);

    // HACK guarda jwt_payload (user y session) en channel_gobj
    gobj_write_user_data(src, "jwt_payload", jwt_payload);

    json_t *access_roles = get_access_roles(
        gobj,
        kw_get_list(jwt_payload, "resource_access`fichador`roles", 0, KW_REQUIRED)
    );
    json_object_set_new(jwt_payload, "access_roles", access_roles);
    //log_debug_json(0, jwt_payload, "jwt_payload");

    /*
     *  User autentificado, crea su registro si es nuevo
     *  e informa de su estado en el ack.
     */
    const char *username = kw_get_str(jwt_payload, "preferred_username", 0, KW_REQUIRED); // User id
    json_t *user = trmsg_get_active_message(priv->users_accesses, username);
    if(!user) {
        create_new_user(gobj, jwt_payload);
        user = trmsg_get_active_message(priv->users_accesses, username);
    }
    kw_get_dict(user, "_sessions", json_object(), KW_CREATE);
    /*
     *  Autorizado, informa
     */
    json_t *webix = msg_iev_build_webix(
        gobj,
        0,
        0,
        0,
        0,
        kw  // owned
    );

    return webix;
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




            /***************************
             *      Local Methods
             ***************************/




/***************************************************************************
 *
 ***************************************************************************/
PRIVATE void oauth2_log_callback(
    oauth2_log_sink_t *sink,
    const char *filename,
    unsigned long line,
    const char *function,
    oauth2_log_level_t level,
    const char *msg
)
{
    hgobj gobj = oauth2_log_sink_ctx_get(sink);

    void (*log_fn)(log_opt_t opt, ...) = 0;
    const char *msgset = MSGSET_OAUTH_ERROR;

    if(level == OAUTH2_LOG_ERROR) {
        log_fn = log_error;
    } else if(level == OAUTH2_LOG_WARN) {
        log_fn = log_warning;
    } else if(level == OAUTH2_LOG_NOTICE || level == OAUTH2_LOG_INFO) {
        log_fn = log_warning;
        msgset = MSGSET_INFO;
    } else if(level >= OAUTH2_LOG_DEBUG) {
        log_fn = log_debug;
        msgset = MSGSET_INFO;
    }

    log_fn(0,
        "gobj",             "%s", gobj_full_name(gobj),
        "function",         "%s", function,
        "msgset",           "%s", msgset,
        "msg",              "%s", msg,
        NULL
    );
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *get_access_roles(
    hgobj gobj,
    json_t *token_roles  // not owned
)
{
    json_t *access_roles = json_object();

    int i; json_t *jn_value;
    json_array_foreach(token_roles, i, jn_value) {
        int list_size;
        const char *str = json_string_value(jn_value);
        const char **s = split2(str, ":_- ", &list_size);
        json_t *resource = kw_get_list(access_roles, s[0], json_array(), KW_CREATE);
        json_array_append_new(resource, json_string(s[1]));
        split_free2(s);
    }
    return access_roles;
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE int create_new_user(hgobj gobj, json_t *jwt_payload)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    const char *username = kw_get_str(jwt_payload, "preferred_username", 0, KW_REQUIRED); // User id

    /*
     *  Crea user en users_accesses
     */
    json_t *user = json_pack("{s:s, s:s, s:I, s:O}",
        "ev", "new_user",
        "username", username,
        "tm", (json_int_t)time_in_seconds(),
        "jwt_payload", jwt_payload
    );

    trmsg_add_instance(
        priv->tranger,
        "users_accesses",
        user, // owned
        0,
        0
    );

    user = trmsg_get_active_message(priv->users_accesses, username);

    return 0;
}




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

    if(src != priv->gobj_top_side) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "on_open NOT from GOBJ_TOP_SIDE",
            "src",          "%s", gobj_full_name(src),
            NULL
        );
    }
    hgobj channel_gobj = (hgobj)(size_t)kw_get_int(kw, "__temp__`channel_gobj", 0, KW_REQUIRED);

    /*------------------------------*
     *      Get jwt info
     *------------------------------*/
    json_t *jwt_payload = gobj_read_user_data(channel_gobj, "jwt_payload");
    if(!jwt_payload) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "What fuck! open without jwt_payload",
            NULL
        );
        KW_DECREF(kw);
        return 0;
    }

    /*--------------------------------------------*
     *  Add login user
     *  El campo "username" del jwt es el id del user.
     *--------------------------------------------*/
    const char *username = kw_get_str(jwt_payload, "preferred_username", 0, KW_REQUIRED);
    json_t *user_ = trmsg_get_active_message(priv->users_accesses, username);
    if(!user_) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "What fuck! user not found",
            "username",          "%s", username,
            NULL
        );
    }
    json_t *user = json_deep_copy(user_);
    json_object_set_new(user, "ev", json_string("login"));
    json_object_set(user, "jwt_payload", jwt_payload);
    json_object_set_new(user, "tm", json_integer(time_in_seconds()));

    /*
     *  Save login record and recover
     *  (Before sessions)
     */
    trmsg_add_instance(
        priv->tranger,
        "users_accesses",
        user, // owned
        0,
        0
    );
    user = trmsg_get_active_message(priv->users_accesses, username);

    /*--------------------------------------------*
     *  Get sessions
     *  By now ONLY one session
     *--------------------------------------------*/
    json_t *sessions = kw_get_dict(user, "_sessions", 0, KW_REQUIRED);
    json_t *session;
    void *n; const char *k;
    json_object_foreach_safe(sessions, n, k, session) {
        /*-------------------------------*
         *  Only one connection allowed
         *-------------------------------*/
        hgobj prev_channel_gobj = (hgobj)(size_t)kw_get_int(session, "channel_gobj", 0, KW_REQUIRED);
        log_info(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INFO,
            "msg",          "%s", "User already connected",
            "user",         "%s", username,
            NULL
        );
        gobj_send_event(prev_channel_gobj, "EV_DROP", 0, gobj);
        json_object_del(sessions, k);
    }

    /*------------------------*
     *      Save session
     *------------------------*/
    const char *session_id = kw_get_str(jwt_payload, "session_state", 0, KW_REQUIRED);
    session = json_pack("{s:I}",
        "channel_gobj", (json_int_t)channel_gobj
    );
    if(!session) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "What fuck! json_pack() FAILED",
            NULL
        );
    }
    json_object_set_new(sessions, session_id, session);

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *  Identity_card off from
 *      Web clients (__top_side__)
 ***************************************************************************/
PRIVATE int ac_on_close(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    if(src == priv->gobj_top_side) {
        hgobj channel_gobj = (hgobj)(size_t)kw_get_int(kw, "__temp__`channel_gobj", 0, KW_REQUIRED);

        /*------------------------------*
        *      Get jwt info
        *------------------------------*/
        json_t *jwt_payload = gobj_read_user_data(channel_gobj, "jwt_payload");
        if(!jwt_payload) {
            log_error(0,
                "gobj",         "%s", gobj_full_name(gobj),
                "function",     "%s", __FUNCTION__,
                "msgset",       "%s", MSGSET_INTERNAL_ERROR,
                "msg",          "%s", "What fuck! open without jwt_payload",
                NULL
            );
            KW_DECREF(kw);
            return 0;
        }

        /*
         *  HACK !important, clean current data
         */
        gobj_write_user_data(src, "jwt_payload", json_null());

        /*--------------------------------------------*
         *  Add logout user
         *--------------------------------------------*/
        if(priv->tranger) { // Si han pasado a pause es 0
            const char *session_id = kw_get_str(jwt_payload, "session_state", 0, KW_REQUIRED);
            const char *username = kw_get_str(jwt_payload, "preferred_username", 0, KW_REQUIRED);
            json_t *user_ = trmsg_get_active_message(priv->users_accesses, username);
            if(!user_) {
                log_error(0,
                    "gobj",         "%s", gobj_full_name(gobj),
                    "function",     "%s", __FUNCTION__,
                    "msgset",       "%s", MSGSET_INTERNAL_ERROR,
                    "msg",          "%s", "What fuck! user not found",
                    "username",          "%s", username,
                    NULL
                );
            }
            json_t *user = json_deep_copy(user_);
            json_t *sessions = kw_get_dict(user, "_sessions", 0, KW_REQUIRED);
            json_t *session = kw_get_dict(sessions, session_id, 0, KW_EXTRACT); // Remove session
            JSON_DECREF(session);

            json_object_set_new(user, "ev", json_string("logout"));
            json_object_set_new(user, "tm", json_integer(time_in_seconds()));

            /*
            *  Save logout record
            */
            trmsg_add_instance(
                priv->tranger,
                "users_accesses",
                user, // owned
                0,
                0
            );
        }
    }

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
    {"EV_ON_OPEN",              0,  0,  0},
    {"EV_ON_CLOSE",             0,  0,  0},
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
    {"EV_ON_OPEN",                  ac_on_open,                 0},
    {"EV_ON_CLOSE",                 ac_on_close,                0},
    {"EV_TIMEOUT",                  ac_timeout,             0},
    {"EV_STOPPED",                  0,                      0},
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
        0, //mt_future24,
        mt_authenticate,
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
        0, //mt_future38,
        0, //mt_future39,
        0, //mt_create_node,
        0, //mt_update_node,
        0, //mt_delete_node,
        0, //mt_link_nodes,
        0, //mt_link_nodes2,
        0, //mt_unlink_nodes,
        0, //mt_unlink_nodes2,
        0, //mt_get_node,
        0, //mt_list_nodes,
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
        0, //mt_node_instances,
        0, //mt_save_node,
        0, //mt_future61,
        0, //mt_future62,
        0, //mt_future63,
        0, //mt_future64
    },
    lmt,
    tattr_desc,
    sizeof(PRIVATE_DATA),
    0,  // acl
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

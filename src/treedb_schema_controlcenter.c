#pragma once

/*

    {}  dict hook   (N unique childs)
    []  list hook   (n not-unique childs)
    (↖) 1 fkey      (1 parent)
    [↖] n fkeys     (n parents)
    {↖} N fkeys     (N parents) ???


    * field required
    = field inherited

                        systems
            ┌───────────────────────────┐
            │* id                       │
            │                           │
            │                systems {} │ ◀─┐N
            │                           │   │
            │             system_id (↖) │ ──┘ 1
            │                           │
            │  description              │
            │= disabled                 │
            │                           │
            │                  nodes {} │ ◀─────────┐N
            │                           │           │
            │               managers {} │ ◀─┐N      │
            │                           │   │       │
            │  _geometry                │   │       │
            └───────────────────────────┘   │       │
                                            │       │
                                            │       │
                        managers            │       │
            ┌───────────────────────────┐   │       │
            │* id                       │   │       │
            │                           │   │       │
            │               systems [↖] │ ──┘n      │
            │                           │           │
            │  properties               │           │
            │  disabled                 │           │
            │  __sessions               │           │
            │  _geometry                │           │
            └───────────────────────────┘           │
                                                    │
                                                    │
                        nodes                       │
            ┌───────────────────────────┐           │
            │* id                       │           │
            │                           │           │
            │               systems [↖] │ ──────────┘n
            │                           │
            │  description              │
            │  provider                 │
            │  provider_url             │
            │= disabled                 │
            │* ip                       │
            │                           │
            │               services {} │ ◀─────────┐N
            │                           │           │
            │  _geometry                │           │
            └───────────────────────────┘           │
                                                    │
                                                    │
                        services                    │
            ┌───────────────────────────┐           │
            │* id                       │           │
            │                           │           │
            │               nodes [↖]   │ ──────────┘n
            │                           │
            │  description              │
            │= disabled                 │
            │* url                      │
            │* dst_role                 │
            │* dst_service              │
            │  dst_yuno                 │
            │* viewer_engine            │
            │                           │
            │  _geometry                │
            └───────────────────────────┘


TODO
            │                           │
            │                  lists {} │ ◀─┐ N
            │                           │   │
            │                           │   │
            │  _geometry                │   │
            └───────────────────────────┘   │
                                            │
                                            │
                        lists               │
            ┌───────────────────────────┐   │
            │* id                       │   │
            │                           │   │
            │               devices [↖] │ ──┘ n
            │                           │
            │  disabled                 │
            │  topic_name               │
            │  key      []              │
            │  notkey   []              │
            │  from_tm                  │
            │  to_tm                    │
            │  from_rowid               │
            │  to_rowid                 │
            │  from_t                   │
            │  to_t                     │
            │  fields                   │
            │  rkey                     │
            │  return_data              │
            │  backward                 │
            │  only_md                  │
            │  user_flag                │
            │  not_user_flag            │
            │  user_flag_mask_set       │
            │  user_flag_mask_notset    │
            │                           │
            │          viewer_engine () │ ◀─────────┐ 1
            │                           │           │
            │  _geometry                │           │
            └───────────────────────────┘           │
                                                    │
                                                    │
                    viewer_engines                  │
            ┌───────────────────────────┐           │
            │* id                       │           │
            │                           │           │
            │                 lists [↖] │ ──────────┘ n
            │                           │
            │                           │
            │  _geometry                │
            └───────────────────────────┘



*/

static char treedb_schema_controlcenter[]= "\
{                                                                   \n\
    'id': 'treedb_controlcenter',                                   \n\
    'schema_version': '4',                                          \n\
    'topics': [                                                     \n\
        {                                                           \n\
            'topic_name': 'systems',                                \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '2',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'System',                             \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'systems': {                                        \n\
                    'header': 'Systems',                            \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'hook'                                      \n\
                    ],                                              \n\
                    'hook': {                                       \n\
                        'systems': 'system_id'                      \n\
                    }                                               \n\
                },                                                  \n\
                'system_id': {                                      \n\
                    'header': 'Top System',                         \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                'description': {                                    \n\
                    'header': 'Description',                        \n\
                    'fillspace': 10,                                \n\
                    'type': 'string',                               \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'inherit',                                  \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'nodes': {                                          \n\
                    'header': 'Nodes',                              \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook'],                               \n\
                    'hook': {                                       \n\
                        'nodes': 'systems'                          \n\
                    }                                               \n\
                },                                                  \n\
                'managers': {                                       \n\
                    'header': 'Managers',                           \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook'],                               \n\
                    'hook': {                                       \n\
                        'managers': 'systems'                       \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'managers',                               \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '3',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Manager',                            \n\
                    'fillspace': 10,                                \n\
                    'type': 'string',                               \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'systems': {                                        \n\
                    'header': 'System' ,                            \n\
                    'fillspace': 10,                                \n\
                    'type': 'array',                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                'properties': {                                     \n\
                    'header': 'Properties',                         \n\
                    'fillspace': 10,                                \n\
                    'type': 'blob',                                 \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'inherit',                                  \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                '__sessions': {                                     \n\
                    'header': 'Sessions',                           \n\
                    'fillspace': 10,                                \n\
                    'type': 'dict',                                 \n\
                    'flag': [                                       \n\
                    ]                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'nodes',                                  \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '3',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Node',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'systems': {                                        \n\
                    'header': 'Systems',                            \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                'description': {                                    \n\
                    'header': 'Description',                        \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'provider': {                                       \n\
                    'header': 'Provider',                           \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'provider_url': {                                   \n\
                    'header': 'Provider Url',                       \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'url',                                      \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'properties': {                                     \n\
                    'header': 'Properties',                         \n\
                    'fillspace': 10,                                \n\
                    'type': 'blob',                                 \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'inherit',                                  \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'ip': {                                             \n\
                    'header': 'Ip',                                 \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'services': {                                       \n\
                    'header': 'Services',                           \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook'],                               \n\
                    'hook': {                                       \n\
                        'services': 'nodes'                         \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'services',                               \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '2',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Service',                            \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'nodes': {                                          \n\
                    'header': 'Nodes',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                'description': {                                    \n\
                    'header': 'Description',                        \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'inherit',                                  \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'url': {                                            \n\
                    'header': 'Url',                                \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'dst_role': {                                       \n\
                    'header': 'Yuno Role',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'dst_service': {                                    \n\
                    'header': 'Yuno Service',                       \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'dst_yuno': {                                       \n\
                    'header': 'Yuno Name',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'viewer_engine': {                                  \n\
                    'header': 'Viewer Engine',                      \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        }                                                           \n\
    ]                                                               \n\
}                                                                   \n\
";


/*
                'lists': {                                          \n\
                    'header': 'Lists',                              \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook'],                               \n\
                    'hook': {                                       \n\
                        'lists': 'devices'                          \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'lists',                                  \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'List',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'devices': {                                        \n\
                    'header': 'Devices',                            \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'topic_name': {                                     \n\
                    'header': 'Topic',                              \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'key': {                                            \n\
                    'header': 'Key',                                \n\
                    'type': 'list',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'notkey': {                                         \n\
                    'header': 'Not Key',                            \n\
                    'type': 'list',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'from_tm': {                                        \n\
                    'header': 'From tm',                            \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'time',                                     \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'to_tm': {                                          \n\
                    'header': 'To tm',                              \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'time',                                     \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'from_rowid': {                                     \n\
                    'header': 'From rowid',                         \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'to_rowid': {                                       \n\
                    'header': 'To rowid',                           \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'from_t': {                                         \n\
                    'header': 'From t',                             \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'time',                                     \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'to_t': {                                           \n\
                    'header': 'To t',                               \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'time',                                     \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'fields': {                                         \n\
                    'header': 'Fields',                             \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'rkey': {                                           \n\
                    'header': 'Regex Key',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'return_data': {                                    \n\
                    'header': 'Return Data',                        \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'backward': {                                       \n\
                    'header': 'Backward',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'only_md': {                                        \n\
                    'header': 'Only Metadata',                      \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'user_flag': {                                      \n\
                    'header': 'User Flag',                          \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'not_user_flag': {                                  \n\
                    'header': 'Not User Flag',                      \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'user_flag_mask_set': {                             \n\
                    'header': 'User Flag Mask Set',                 \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'user_flag_mask_notset': {                          \n\
                    'header': 'User Flag Mask Not Set',             \n\
                    'type': 'integer',                              \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                'viewer_engine': {                                  \n\
                    'header': 'Viewer Engine',                      \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'hook'                                      \n\
                    ],                                              \n\
                    'hook': {                                       \n\
                        'viewer_engines': 'lists'                   \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'viewer_engines',                         \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Viewer Engines',                     \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'lists': {                                          \n\
                    'header': 'Lists',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'fkey'                                      \n\
                    ]                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'persistent'                                \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        }                                                           \n\
*/

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
            │  disabled                 │
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
            │  _sessions                │           │
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
            │  disabled                 │
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
            │  disabled                 │
            │* url                      │
            │* dst_role                 │
            │* dst_service              │
            │* visor                    │
            │                           │
            │  _geometry                │
            └───────────────────────────┘



*/

static char treedb_schema_controlcenter[]= "\
{                                                                   \n\
    'id': 'treedb_controlcenter',                                   \n\
    'schema_version': '1',                                          \n\
    'topics': [                                                     \n\
        {                                                           \n\
            'topic_name': 'systems',                                \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
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
            'topic_version': '1',                                   \n\
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
                        'inherit',                                  \n\
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                '_sessions': {                                      \n\
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
            'topic_version': '1',                                   \n\
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
                'disabled': {                                       \n\
                    'header': 'disabled',                           \n\
                    'fillspace': 8,                                 \n\
                    'type': 'boolean',                              \n\
                    'flag': [                                       \n\
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
            'topic_version': '1',                                   \n\
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

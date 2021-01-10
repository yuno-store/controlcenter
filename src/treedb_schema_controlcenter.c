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
            │* description              │
            │                           │
            │               services {} │ ◀─────────┐N
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
            │  _sessions                │           │
            └───────────────────────────┘           │
                                                    │
                                                    │
                        services                    │
            ┌───────────────────────────┐           │
            │* id                       │           │
            │                           │           │
            │               systems [↖] │ ──────────┘n
            │                           │
            │* description              │
            │* url                      │
            │* realm                    │
            │* service                  │
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
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'services': {                                       \n\
                    'header': 'Services',                           \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook'],                               \n\
                    'hook': {                                       \n\
                        'services': 'systems'                       \n\
                    }                                               \n\
                },                                                  \n\
                'managers': {                                       \n\
                    'header': 'Managers',                           \n\
                    'type': 'array',                                \n\
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
                        'persistent'                                \n\
                    ]                                               \n\
                },                                                  \n\
                '_sessions': {                                      \n\
                    'header': 'Sessions',                           \n\
                    'fillspace': 10,                                \n\
                    'type': 'dict',                                 \n\
                    'flag': [                                       \n\
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
                        'persistent',                               \n\
                        'required'                                  \n\
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
                'realm': {                                          \n\
                    'header': 'Realm',                              \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': [                                       \n\
                        'writable',                                 \n\
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                },                                                  \n\
                'service': {                                        \n\
                    'header': 'Service',                            \n\
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
                        'persistent',                               \n\
                        'required'                                  \n\
                    ]                                               \n\
                }                                                   \n\
            }                                                       \n\
        }                                                           \n\
    ]                                                               \n\
}                                                                   \n\
";

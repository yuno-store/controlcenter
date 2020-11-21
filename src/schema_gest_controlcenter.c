#pragma once

/*

                    gest_departments
                ┌───────────────────────┐
                │                       │ n (hook)
                │      'departments' {} │◀━━━━━━━━━━━━━━━━━━┓
                │                       │                   ┃
                │                       │ 1 (fkey)          ┃
                │     'department_id' s │◀━━━━━━━━━━━━━━━━━━┛
                │                       │
                │                       │
                │                       │ n (hook)
                │         'managers' [] │◀━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━┓
                │                       │                     ┃         ┃
                │                       │                     ┃         ┃
                │                       │ n (fkey)            ┃         ┃
                │            'users' [] │◀━━━━━━━━━━━━━━━━━━━━┛         ┃
                │                       │ n (hook)                      ┃
                │            'users' [] │◀━━━━━━━━━━━━━━┓               ┃
                │                       │               ┃               ┃
                └───────────────────────┘               ┃               ┃
                                                        ┃               ┃
                                                        ┃               ┃
                    gest_users                          ┃               ┃
                ┌───────────────────────┐               ┃               ┃
                │                       │ n (fkey)      ┃               ┃
                │      'departments' [] │◀━━━━━━━━━━━━━━┛               ┃
                │                       │                               ┃
                │                       │                               ┃
                │                       │ n (fkey)                      ┃
                │          'manager' [] │◀━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                │                       │
                │                       │
                └───────────────────────┘

                where n -> dl or []

*/

static char schema_gest_controlcenter[]= "\
{                                                                   \n\
    'id': 'gest_controlcenter',                                     \n\
    'schema_type': 'treedbs',                                       \n\
    'schema_version': '1',                                          \n\
    'topics': [                                                     \n\
        {                                                           \n\
            'topic_name': 'gest_systems',                           \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Name',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'description': {                                    \n\
                    'header': 'Description',                        \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'system_id': {                                      \n\
                    'header': 'Top System',                         \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['fkey','writable']                     \n\
                },                                                  \n\
                'systems': {                                        \n\
                    'header': 'Systems',                            \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_systems': 'system_id'                 \n\
                    }                                               \n\
                },                                                  \n\
                'services': {                                       \n\
                    'header': 'Services',                           \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_services': 'systems'                  \n\
                    }                                               \n\
                },                                                  \n\
                'users': {                                          \n\
                    'header': 'Users',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_users': 'departments'                 \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'gest_services',                          \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Name',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'description': {                                    \n\
                    'header': 'Description',                        \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'url': {                                            \n\
                    'header': 'Url',                                \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'yuno_role': {                                      \n\
                    'header': 'Yuno Role',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'yuno_service': {                                   \n\
                    'header': 'Yuno Service',                       \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'yuno_name': {                                      \n\
                    'header': 'Yuno Name',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'systems': {                                        \n\
                    'header': 'Systems',                            \n\
                    'type': 'array',                                \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['fkey','persistent','required','writable'] \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'gest_roles',                             \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Id',                                 \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'name': {                                           \n\
                    'header': 'Name',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'users': {                                          \n\
                    'header': 'Users',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_users': 'roles'                       \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'gest_departments',                       \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Id',                                 \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'name': {                                           \n\
                    'header': 'Name',                               \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'department_id': {                                  \n\
                    'header': 'Top Department',                     \n\
                    'type': 'string',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['fkey','writable']                     \n\
                },                                                  \n\
                'departments': {                                    \n\
                    'header': 'Departments',                        \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_departments': 'department_id'         \n\
                    }                                               \n\
                },                                                  \n\
                'users': {                                          \n\
                    'header': 'Users',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','fkey','writable'],             \n\
                    'hook': {                                       \n\
                        'gest_users': 'departments'                 \n\
                    }                                               \n\
                },                                                  \n\
                'managers': {                                       \n\
                    'header': 'Managers',                           \n\
                    'type': 'object',                               \n\
                    'fillspace': 10,                                \n\
                    'flag': ['hook','writable'],                    \n\
                    'hook': {                                       \n\
                        'gest_users': 'manager',                    \n\
                        'gest_departments': 'users'                 \n\
                    }                                               \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                }                                                   \n\
            }                                                       \n\
        },                                                          \n\
                                                                    \n\
        {                                                           \n\
            'topic_name': 'gest_users',                             \n\
            'pkey': 'id',                                           \n\
            'system_flag': 'sf_string_key',                         \n\
            'topic_version': '1',                                   \n\
            'cols': {                                               \n\
                'id': {                                             \n\
                    'header': 'Id',                                 \n\
                    'type': 'string',                               \n\
                    'fillspace': 5,                                 \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'uuid': {                                           \n\
                    'header': 'Id',                                 \n\
                    'fillspace': 4,                                 \n\
                    'type': 'string',                               \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'firstName': {                                      \n\
                    'header': 'First Name',                         \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'lastName': {                                       \n\
                    'header': 'Last Name',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 6,                                 \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'email': {                                          \n\
                    'header': 'Email',                              \n\
                    'type': 'string',                               \n\
                    'fillspace': 8,                                 \n\
                    'flag': ['persistent','required']               \n\
                },                                                  \n\
                'emailVerified': {                                  \n\
                    'header': 'Email Verified',                     \n\
                    'type': 'boolean',                              \n\
                    'fillspace': 1,                                 \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'createdTimestamp': {                               \n\
                    'header': 'createAt',                           \n\
                    'type': 'integer',                              \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent']                          \n\
                },                                                  \n\
                'enabled': {                                        \n\
                    'header': 'Disabled',                           \n\
                    'type': 'boolean',                              \n\
                    'fillspace': 1,                                 \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                'online': {                                         \n\
                    'header': 'On Line',                            \n\
                    'type': 'boolean',                              \n\
                    'fillspace': 1,                                 \n\
                    'flag': []                                      \n\
                },                                                  \n\
                'roles': {                                          \n\
                    'header': 'Roles',                              \n\
                    'type': 'array',                                \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['fkey','persistent','required','writable'] \n\
                },                                                  \n\
                'departments': {                                    \n\
                    'header': 'Department',                         \n\
                    'type': 'array',                                \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['fkey','persistent','required','writable'] \n\
                },                                                  \n\
                'manager': {                                        \n\
                    'header': 'Manager',                            \n\
                    'type': 'array',                                \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['fkey','persistent','required','writable'] \n\
                },                                                  \n\
                'proyecto': {                                       \n\
                    'header': 'Proyecto',                           \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'provincia': {                                      \n\
                    'header': 'Provincia',                          \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'tz': {                                             \n\
                    'header': 'Zona horaria',                       \n\
                    'type': 'string',                               \n\
                    'fillspace': 2,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'dni': {                                            \n\
                    'header': 'DNI',                                \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'telefono': {                                       \n\
                    'header': 'Teléfono Empresa',                   \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'telefono_personal': {                              \n\
                    'header': 'Teléfono Personal',                  \n\
                    'type': 'string',                               \n\
                    'fillspace': 4,                                 \n\
                    'flag': ['persistent','writable']               \n\
                },                                                  \n\
                'zona_horaria': {                                   \n\
                    'header': 'Zona horaria',                       \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 2,                                 \n\
                    'flag': ['persistent','required','writable']    \n\
                },                                                  \n\
                '_geometry': {                                      \n\
                    'header': 'Geometry',                           \n\
                    'type': 'blob',                                 \n\
                    'fillspace': 10,                                \n\
                    'flag': ['persistent','writable']               \n\
                }                                                   \n\
            }                                                       \n\
        }                                                           \n\
    ]                                                               \n\
}                                                                   \n\
";

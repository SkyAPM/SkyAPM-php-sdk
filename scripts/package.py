#
# Copyright 2021 SkyAPM
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import xml.dom.minidom
import time, os

version = '5.0.0'

git = os.popen('git ls-files')
res = git.read()
child = []

for line in res.splitlines():
    if line == "":
        continue

    if line == "package.xml" or line[0:1] == '.':
        continue

    ext = os.path.splitext(line)[-1][1:]
    role = 'src'
    if ext == 'md':
        role = 'doc'
    elif ext == 'phpt':
        role = 'test'
    elif ext == '':
        if line != 'docker/Dockerfile':
            role = 'doc'
    child.append({'key': 'file', 'attr': [
        {'key': 'role', 'value': role},
        {'key': 'name', 'value': line},
    ]})

doc = xml.dom.minidom.Document()

package = doc.createElement('package')
package.setAttribute('packagerversion', '1.9.4')
package.setAttribute('version', '2.0')
package.setAttribute('xmlns', 'http://pear.php.net/dtd/package-2.0')
package.setAttribute('xmlns:tasks', 'http://pear.php.net/dtd/tasks-1.0')
package.setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance')
location = 'http://pear.php.net/dtd/tasks-1.0 http://pear.php.net/dtd/tasks-1.0.xsd'
location += ' http://pear.php.net/dtd/package-2.0 http://pear.php.net/dtd/package-2.0.xsd'
package.setAttribute('xsi:schemaLocation', location)
doc.appendChild(package)

config = [
    {'key': 'name', 'value': 'skywalking'},
    {'key': 'channel', 'value': 'pecl.php.net'},
    {'key': 'summary', 'value': 'The PHP instrument agent for Apache SkyWalking.'},
    {'key': 'description', 'value': 'The package is the PHP instrumentation agent, which is compatible with Apache SkyWalking backend and others compatible agents/SDKs.'},
    {'key': 'lead', 'child': [
        {'key': 'name', 'value': 'Yanlong He'},
        {'key': 'user', 'value': 'yanlong'},
        {'key': 'email', 'value': 'yanlong@php.net'},
        {'key': 'active', 'value': 'yes'},
    ]},
    {'key': 'date', 'value': time.strftime("%Y-%m-%d", time.localtime())},
    {'key': 'version', 'child': [
        {'key': 'release', 'value': version},
        {'key': 'api', 'value': version},
    ]},
    {'key': 'stability', 'child': [
        {'key': 'release', 'value': 'stable'},
        {'key': 'api', 'value': 'stable'},
    ]},
    {'key': 'license', 'value': 'Apache2.0', 'attr': [
        {'key': 'uri', 'value': 'http://www.apache.org/licenses/LICENSE-2.0.html'}
    ]},
    {'key': 'notes', 'value': 'Support Skywalking 9.0.0'}, # release notes
    {'key': 'contents', 'child': [
        {'key': 'dir', 'attr': [
            {'key': 'name', 'value': '/'},
        ], 'child': child},
    ]},
    {'key': 'dependencies', 'child': [
        {'key': 'required', 'child': [
            {'key': 'php', 'child': [
                {'key': 'min', 'value': '7.0.0'}
            ]},
            {'key': 'pearinstaller', 'child': [
                {'key': 'min', 'value': '1.4.0'}
            ]}
        ]}
    ]},
    {'key': 'providesextension', 'value': 'skywalking'},
    {'key': 'extsrcrelease'},
    {'key': 'changelog', 'child': [
        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                    {'key': 'release', 'value': '5.0.0'},
                    {'key': 'api', 'value': '5.0.0'}
                ]
            },
            {'key': 'stability', 'child': [
                    {'key': 'release', 'value': 'stable'},
                    {'key': 'api', 'value': 'stable'}
                ]
            },
                {'key': 'notes', 'value': 'Support Skywalking 9.0.0'}
            ]
        },
        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                    {'key': 'release', 'value': '4.2.0'},
                    {'key': 'api', 'value': '4.2.0'}
                ]
            },
            {'key': 'stability', 'child': [
                    {'key': 'release', 'value': 'stable'},
                    {'key': 'api', 'value': 'stable'}
                ]
            },
                {'key': 'notes', 'value': 'Fix bugs'}
            ]
        },

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                    {'key': 'release', 'value': '4.1.3'},
                    {'key': 'api', 'value': '4.1.3'}
                ]
            },
            {'key': 'stability', 'child': [
                    {'key': 'release', 'value': 'stable'},
                    {'key': 'api', 'value': 'stable'}
                ]
            },
                {'key': 'notes', 'value': 'Fix bugs'}
            ]
        },

        {'key': 'release', 'child': [
                {'key': 'version', 'child': [
                        {'key': 'release', 'value': '4.1.2'},
                        {'key': 'api', 'value': '4.1.2'}
                    ]
                },
                {'key': 'stability', 'child': [
                        {'key': 'release', 'value': 'stable'},
                        {'key': 'api', 'value': 'stable'}
                    ]
                },
                {'key': 'notes', 'value': 'Add mysqli'}
            ]
        },

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '4.1.1'},
                {'key': 'api', 'value': '4.1.1'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix bugs'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '4.1.0'},
                {'key': 'api', 'value': '4.1.0'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix bugs, Support swoole'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '4.0.0'},
                {'key': 'api', 'value': '4.0.0'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'newly designed agent, modular design'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.3.2'},
                {'key': 'api', 'value': '3.3.2'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix bugs'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.3.1'},
                {'key': 'api', 'value': '3.3.1'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix bugs'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.3.0'},
                {'key': 'api', 'value': '3.3.0'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Added memcache collection and skywalking 8.0 support'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.2.8'},
                {'key': 'api', 'value': '3.2.8'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix bugs and add Dockerfile'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.2.6'},
                {'key': 'api', 'value': '3.2.6'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix the mistake of field entryOperationName in sw6 header.'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.2.5'},
                {'key': 'api', 'value': '3.2.5'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Fix the empty field bug in span refs for SkyWalking v6.'}
        ]},

        {'key': 'release', 'child': [
            {'key': 'version', 'child': [
                {'key': 'release', 'value': '3.2.4'},
                {'key': 'api', 'value': '3.2.4'}
            ]},
            {'key': 'stability', 'child': [
                {'key': 'release', 'value': 'stable'},
                {'key': 'api', 'value': 'stable'}
            ]},
            {'key': 'notes', 'value': 'Support php7.4'}
        ]},
    ]},
]


def create(ele, conf):
    for item in conf:
        element = doc.createElement(item['key'])
        if 'attr' in item.keys():
            for attr in item['attr']:
                element.setAttribute(attr['key'], attr['value'])

        if 'child' in item.keys():
            create(element, item['child'])
        else:
            if 'value' in item.keys():
                element.appendChild(doc.createTextNode(item['value']))
        ele.appendChild(element)


create(package, config)

fp = open('package.xml', 'w')
doc.writexml(fp, indent='', addindent='\t', newl='\n', encoding="UTF-8")

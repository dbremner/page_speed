# Copyright 2010 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'variables': {
    'libpagespeed_root': '<(DEPTH)/third_party/libpagespeed/src',
    'conditions': [
      ['OS=="win"', {
        'os_name': 'WINNT',
        'compiler_abi': 'msvc',
      }],
      ['OS=="linux"', {
        'os_name': 'Linux',
        'compiler_abi': 'gcc3',
      }],
      ['OS=="mac"', {
        'os_name': 'Darwin',
        'compiler_abi': 'gcc3',
      }],
      ['target_arch=="ia32"', {
        'cpu_arch': 'x86',
      }],
      ['target_arch=="x64"', {
        'cpu_arch': 'x86_64',
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'pagespeed_json_dom',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/core/core.gyp:pagespeed_core',
      ],
      'sources': [
        'json_dom.cc',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/core/core.gyp:pagespeed_core',
      ],
    },
    {
      'target_name': 'pagespeed_chromium_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed_json_dom',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
      ],
      'sources': [
        'json_dom_test.cc',
      ],
    },
    {
      'target_name': 'pagespeed_plugin',
      'type': 'loadable_module',
      'mac_bundle': 1,
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/core/core.gyp:pagespeed_core',
        '<(libpagespeed_root)/pagespeed/core/init.gyp:pagespeed_init',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed_library',
        '<(libpagespeed_root)/pagespeed/filters/filters.gyp:pagespeed_filters',
        '<(libpagespeed_root)/pagespeed/formatters/formatters.gyp:pagespeed_formatters',
        '<(libpagespeed_root)/pagespeed/har/har.gyp:pagespeed_har',
        '<(libpagespeed_root)/pagespeed/image_compression/image_compression.gyp:pagespeed_image_attributes_factory',
        '<(libpagespeed_root)/pagespeed/po/po_gen.gyp:pagespeed_all_po',
        '<(libpagespeed_root)/pagespeed/proto/proto_gen.gyp:pagespeed_output_pb',
        '<(libpagespeed_root)/pagespeed/proto/proto_gen.gyp:pagespeed_proto_formatted_results_converter',
        '<(libpagespeed_root)/pagespeed/timeline/timeline.gyp:pagespeed_timeline',
        'pagespeed_json_dom',
      ],
      'sources': [
        'pagespeed_chromium.cc',
        'pagespeed_plugin.rc',
        'npapi/np_entry.cc',
        'npapi/npp_entry.cc',
      ],
      'include_dirs': [
        '<(DEPTH)',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
	  'ModuleDefinitionFile': 'pagespeed_plugin.def',
	},
      },
      'xcode_settings': {
        'INFOPLIST_FILE': 'Info.plist',
      },
      'conditions': [
        ['OS=="mac"', {
          'product_extension': 'plugin',
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
            ],
          },
          'defines': [
            'WEBKIT_DARWIN_SDK',
          ],
        }],
      ],
    },
  ],

  'conditions': [
  ['OS!="win"', {
  'targets': [
    {
      'target_name': 'pagespeed_extension_make_dirs',
      'type': 'none',
      'actions': [{
        'action_name': 'make_dir',
        'inputs': [],
        'outputs': [
          '<(PRODUCT_DIR)/pagespeed',
          '<(PRODUCT_DIR)/pagespeed/_locales/en',
        ],
        'action': ['mkdir', '-p', '<@(_outputs)'],
      }],
    },
    {
      'target_name': 'pagespeed_extension_copy_files',
      'type': 'none',
      'dependencies': [
        'pagespeed_extension_make_dirs',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/pagespeed',
          'files': [
            'extension_files/background.html',
            'extension_files/background.js',
            'extension_files/devtools-page.html',
            'extension_files/devtools-page.js',
            'extension_files/errorRedDot.png',
            'extension_files/fetch_devtools_api.js',
            'extension_files/infoBlueDot.png',
            'extension_files/inject_pagespeed.js',
            'extension_files/manifest.json',
            'extension_files/naGrayDot.png',
            'extension_files/options.css',
            'extension_files/options.html',
            'extension_files/options.js',
            'extension_files/pagespeed-32.png',
            'extension_files/pagespeed-64.png',
            'extension_files/pagespeed-128.png',
            'extension_files/pagespeed.js',
            'extension_files/pagespeed-panel.css',
            'extension_files/pagespeed-panel.html',
            'extension_files/spinner.gif',
            'extension_files/successGreenDot.png',
            'extension_files/warningOrangeDot.png',
            'extension_files/_locales/en/messages.json',
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/pagespeed/_locales/en',
          'files': [
            'extension_files/_locales/en/messages.json',
          ],
        },
      ]
    },
    {
      'target_name': 'pagespeed_extension_copy_so',
      'type': 'none',
      'dependencies': [
        'pagespeed_plugin',
        'pagespeed_extension_make_dirs',
      ],
      'variables': {
        'so_input_path': '<(PRODUCT_DIR)/<(SHARED_LIB_PREFIX)pagespeed_plugin<(SHARED_LIB_SUFFIX)',
        'conditions': [
          ['OS=="mac"', {
            'so_output_path': '<(PRODUCT_DIR)/pagespeed/pagespeed_plugin.plugin',
          }, {
            'so_output_path': '<(PRODUCT_DIR)/pagespeed/<(SHARED_LIB_PREFIX)pagespeed_plugin_<(os_name)_<(cpu_arch)-<(compiler_abi)<(SHARED_LIB_SUFFIX)',
          }],
        ],
      },
      'actions': [
        {
          'action_name': 'copy_so',
          'inputs': [
            '<(PRODUCT_DIR)/pagespeed',
            '<(so_input_path)',
          ],
          'outputs': ['<(so_output_path)'],
          'conditions': [
            ['OS=="mac"', {
              'action': [
                'cp', '-r', '<(PRODUCT_DIR)/pagespeed_plugin.plugin',
                '<(PRODUCT_DIR)/pagespeed',
              ],
	    }, {
              'action': ['cp', '<(so_input_path)', '<(so_output_path)'],
            }],
          ],
        },
        {
          'action_name': 'touch_plugins',
          'inputs': [
            '<(PRODUCT_DIR)/pagespeed',
          ],
          'outputs': [
            # This list should contain exactly the filenames listed under
            # "plugins" in manifest.json.
            '<(PRODUCT_DIR)/pagespeed/pagespeed_plugin_WINNT_x86-msvc.dll',
            '<(PRODUCT_DIR)/pagespeed/libpagespeed_plugin_Linux_x86-gcc3.so',
            '<(PRODUCT_DIR)/pagespeed/libpagespeed_plugin_Linux_x86_64-gcc3.so',
            '<(PRODUCT_DIR)/pagespeed/pagespeed_plugin.plugin',
          ],
          # Don't touch the actual built plugin; it confuses the build system.
          'outputs!': ['<(so_output_path)'],
          'action': ['touch', '-a', '<@(_outputs)'],
        },
      ],
    },
  ],

  }],  # 'OS!="win"'
  ],   # end condition block
}

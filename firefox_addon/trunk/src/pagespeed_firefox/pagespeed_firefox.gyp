# Copyright 2009 Google Inc.
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
    'xulrunner_sdk_root': '<(DEPTH)/third_party/xulrunner-sdk',
    'xulrunner_sdk_os_root': '<(xulrunner_sdk_root)/arch/<(OS)',
    'xulrunner_sdk_arch_root': '<(xulrunner_sdk_os_root)/<(target_arch)',
    'protoc_out_dir': '<(SHARED_INTERMEDIATE_DIR)/protoc_out',
    'xpidl_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xpidl_out',
  },
  'targets': [
    {
      'target_name': 'xulrunner_sdk',
      'type': 'none',
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)',  # For headers generated from idl files
          '<(xulrunner_sdk_root)/include',
          '<(xulrunner_sdk_os_root)/all/include',
        ],
        'defines': [
          'NO_NSPR_10_SUPPORT',
        ],
        'conditions': [  # See https://developer.mozilla.org/en/XPCOM_Glue
          ['OS == "linux"', {
            'cflags': [
              '-fshort-wchar',
              '-include', 'pagespeed_firefox/<(xulrunner_sdk_os_root)/all/include/xpcom-config.h',
            ],
            'ldflags': [
              '-Lpagespeed_firefox/<(xulrunner_sdk_arch_root)/lib',
              '-Lpagespeed_firefox/<(xulrunner_sdk_arch_root)/bin',
              '-Wl,-rpath-link,pagespeed_firefox/<(xulrunner_sdk_arch_root)/bin',
            ],
            'link_settings': {
              'libraries': [
                '-lxpcomglue_s',
                '-lxpcom',
                '-lnspr4',
            ]},
          }],
          ['OS == "mac"', {
            'link_settings': {
              'libraries': [
                'libxpcomglue_s.a',
                'libxpcom.dylib',
                'libnspr4.dylib',
              ],
            },
            'xcode_settings': {
              'LIBRARY_SEARCH_PATHS': [
                '<(xulrunner_sdk_arch_root)/lib',
                '<(xulrunner_sdk_arch_root)/bin',
              ],
              'OTHER_CFLAGS': [
                '-fshort-wchar',
                '-include <(xulrunner_sdk_os_root)/all/include/xpcom-config.h',
              ],
            },
          }],
          ['OS == "win"', {
            'cflags': [
              '/FI "xpcom-config.h"',
              '/Zc:wchar_t-',
            ],
            'defines': [
              'XP_WIN',
            ],
            'link_settings': {
              'libraries': [
                '<(xulrunner_sdk_arch_root)/lib/xpcomglue_s.lib',
                '<(xulrunner_sdk_arch_root)/lib/xpcom.lib',
                '<(xulrunner_sdk_arch_root)/lib/nspr4.lib',
              ],
            },
          }]
        ],
      },
    },
    {
      'target_name': 'pagespeed_firefox_genidl',
      'type': 'none',
      'sources': [
        'idl/IJsMin.idl',
        'idl/IPageSpeedRules.idl',
      ],
      'rules': [
        {
          'rule_name': 'genidl',
          'extension': 'idl',
          'variables': {
            'rule_input_relpath': 'idl',
          },
          'outputs': [
            '<(xpidl_out_dir)/pagespeed_firefox/<(rule_input_relpath)/<(RULE_INPUT_ROOT).h',
          ],
          'action': [
            '<(xulrunner_sdk_arch_root)/bin/xpidl',
            '-m', 'header',
            '-I', '<(xulrunner_sdk_root)/idl',
            '-e', '<(xpidl_out_dir)/pagespeed_firefox/<(rule_input_relpath)/<(RULE_INPUT_ROOT).h',
            './<(rule_input_relpath)/<(RULE_INPUT_ROOT)<(RULE_INPUT_EXT)',
          ],
          'message': 'Generating C++ header from <(RULE_INPUT_PATH)',
        },
      ],
      'dependencies': [
        'xulrunner_sdk',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(xpidl_out_dir)/pagespeed_firefox/idl',
        ]
      },
      'export_dependent_settings': [
        'xulrunner_sdk',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_js_min',
      'type': '<(library)',
      'variables': {
        'js_min_root': 'cpp/js_min',
      },
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_genidl',
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/third_party/jsmin/jsmin.gyp:jsmin',
      ],
      'sources': [
        '<(js_min_root)/js_minifier.cc',
      ],
      'include_dirs': [
        '<(libpagespeed_root)',
      ],
      'export_dependent_settings': [
        'pagespeed_firefox_genidl',
        '<(DEPTH)/base/base.gyp:base',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_file_util',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ],
      'sources': [
        'cpp/pagespeed/file_util.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_json_input',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/third_party/cJSON/cJSON.gyp:cJSON',
        '<(libpagespeed_root)/pagespeed/core/core.gyp:pagespeed_core',
      ],
      'sources': [
        'cpp/pagespeed/pagespeed_json_input.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/core/core.gyp:pagespeed_core',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed_firefox_file_util',
        'pagespeed_firefox_json_input',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtestmain',
      ],
      'sources': [
        'cpp/pagespeed/file_util_test.cc',
        'cpp/pagespeed/pagespeed_json_input_test.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_library_rules',
      'type': '<(library)',
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_file_util',
        'pagespeed_firefox_genidl',
        'pagespeed_firefox_json_input',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed',
        '<(libpagespeed_root)/pagespeed/filters/filters.gyp:pagespeed_filters',
        '<(libpagespeed_root)/pagespeed/formatters/formatters.gyp:pagespeed_formatters',
        '<(libpagespeed_root)/pagespeed/image_compression/image_compression.gyp:pagespeed_image_attributes_factory',
      ],
      'sources': [
        'cpp/pagespeed/firefox_dom.cc',
        'cpp/pagespeed/pagespeed_rules.cc',
      ],
      'include_dirs': [
        'cpp/pagespeed',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(libpagespeed_root)/pagespeed/pagespeed.gyp:pagespeed',
        '<(libpagespeed_root)/pagespeed/filters/filters.gyp:pagespeed_filters',
        '<(libpagespeed_root)/pagespeed/formatters/formatters.gyp:pagespeed_formatters',
      ],
    },
    {
      'target_name': 'pagespeed_firefox_module',
      'type': 'loadable_module',
      'variables': {
        'src_root': 'cpp',
      },
      'dependencies': [
        'xulrunner_sdk',
        'pagespeed_firefox_js_min',
        'pagespeed_firefox_library_rules',
      ],
      'defines': [
        'PAGESPEED_INCLUDE_LIBRARY_RULES',

        # HAVE_VISIBILITY_ATTRIBUTE instructs the Firefox module headers
        # to mark NS_GetModule with gcc attribute visibility("default").
        # Without this, Firefox would not be able to find its entry point
        # into our shared library. See http://gcc.gnu.org/wiki/Visibility
        # for additional information on the gcc visibility attribute.
        'HAVE_VISIBILITY_ATTRIBUTE=1',
      ],
      'sources': [
        '<(src_root)/pagespeed/pagespeed_module.cc',
      ],
      'include_dirs': [
        '<(src_root)',
        '<(src_root)/pagespeed',
      ],
      'conditions': [['OS == "mac"', {
        'xcode_settings': {
          # We must null out these two variables when building this target,
          # because it is a loadable_module (-bundle).
          'DYLIB_COMPATIBILITY_VERSION':'',
          'DYLIB_CURRENT_VERSION':'',
        }
      }]],
    },
  ],
}

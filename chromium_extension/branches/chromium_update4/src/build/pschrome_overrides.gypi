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
    # Make sure we link statically so everything gets linked into a
    # single shared object.
    'library': 'static_library',
  },
  'conditions': [
    ['build_nacl==1', {
      'target_defaults': {
        'defines': [
          # NaCL newlib's libpthread.a provides the
          # GetRunningOnValgrind symbol already, so we should not
          # provide it.
          'DYNAMIC_ANNOTATIONS_PROVIDE_RUNNING_ON_VALGRIND=0',
        ],
        'include_dirs': [
          '<(DEPTH)/build/nacl_header_stubs',
        ],
      },
    }],
  ],
}

// Copyright 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PAGESPEED_CHROMIUM_PAGESPEED_CHROMIUM_H_
#define PAGESPEED_CHROMIUM_PAGESPEED_CHROMIUM_H_

#include <string>

namespace pagespeed_chromium {

// Runs Page Speed analysis on the given input data.
// "data" is a serialized JSON object that contains a serialized
// representation of the har, document, timeline, resource filter,
// locale, and whether to save optimized content. The field names and
// types are:
//  'har': string
//  'document': string
//  'timeline': string
//  'resource_filter': string
//  'locale': string
//  'save_optimized_content': boolean
// Returns true on success, false otherwise. If true, output_string
// will contain the result. If false, error_string will contain a
// human-readable error message.
bool RunPageSpeedRules(const std::string& data,
                       std::string* output_string,
                       std::string* error_string);

// Runs Page Speed analysis on the given input values.
// Returns true on success, false otherwise. If true, output_string
// will contain the result. If false, error_string will contain a
// human-readable error message.
bool RunPageSpeedRules(const std::string& har_data,
                       const std::string& document_data,
                       const std::string& timeline_data,
                       const std::string& resource_filter_name,
                       const std::string& locale,
                       bool save_optimized_content,
                       std::string* output_string,
                       std::string* error_string);

}  // namespace pagespeed_chromium

#endif  // PAGESPEED_CHROMIUM_PAGESPEED_CHROMIUM_H_

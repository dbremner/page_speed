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

#include "pagespeed_chromium/pagespeed_chromium.h"

#include <string>
#include <vector>

#include "base/at_exit.h"
#include "base/base64.h"
#include "base/basictypes.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/md5.h"
#include "base/scoped_ptr.h"
#include "base/stl_util-inl.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/values.h"
#include "googleurl/src/gurl.h"
#include "pagespeed/core/engine.h"
#include "pagespeed/core/file_util.h"
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_init.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource_filter.h"
#include "pagespeed/core/rule.h"
#include "pagespeed/dom/json_dom.h"
#include "pagespeed/timeline/json_importer.h"
#include "pagespeed/filters/ad_filter.h"
#include "pagespeed/filters/response_byte_result_filter.h"
#include "pagespeed/filters/tracker_filter.h"
#include "pagespeed/formatters/proto_formatter.h"
#include "pagespeed/har/http_archive.h"
#include "pagespeed/image_compression/image_attributes_factory.h"
#include "pagespeed/l10n/gettext_localizer.h"
#include "pagespeed/l10n/localizer.h"
#include "pagespeed/proto/formatted_results_to_json_converter.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/proto/pagespeed_proto_formatter.pb.h"
#include "pagespeed/proto/timeline.pb.h"
#include "pagespeed/rules/rule_provider.h"

namespace {

pagespeed::ResourceFilter* NewFilter(const std::string& analyze) {
  if (analyze == "ads") {
    return new pagespeed::NotResourceFilter(new pagespeed::AdFilter());
  } else if (analyze == "trackers") {
    return new pagespeed::NotResourceFilter(new pagespeed::TrackerFilter());
  } else if (analyze == "content") {
    return new pagespeed::AndResourceFilter(new pagespeed::AdFilter(),
                                            new pagespeed::TrackerFilter());
  } else {
    if (analyze != "all") {
      LOG(DFATAL) << "Unknown filter type: " << analyze;
    }
    return new pagespeed::AllowAllResourceFilter();
  }
}

void SerializeOptimizedContent(const pagespeed::Results& results,
                               DictionaryValue* optimized_content) {
  for (int i = 0; i < results.rule_results_size(); ++i) {
    const pagespeed::RuleResults& rule_results = results.rule_results(i);
    for (int j = 0; j < rule_results.results_size(); ++j) {
      const pagespeed::Result& result = rule_results.results(j);
      if (!result.has_optimized_content()) {
        continue;
      }

      const std::string key = base::IntToString(result.id());
      if (optimized_content->HasKey(key)) {
        LOG(ERROR) << "Duplicate result id: " << key;
        continue;
      }

      if (result.resource_urls_size() <= 0) {
        LOG(ERROR) << "Result id " << key
                   << " has optimized content, but no resource URLs";
        continue;
      }

      const std::string& url = result.resource_urls(0);
      const GURL gurl(url);
      if (!gurl.is_valid()) {
        LOG(ERROR) << "Invalid url: " << url;
        continue;
      }

      // TODO(mdsteele): Maybe we shouldn't base64-encode HTML/JS/CSS files?
      const std::string& content = result.optimized_content();
      std::string encoded;
      if (!base::Base64Encode(content, &encoded)) {
        LOG(ERROR) << "Base64Encode failed for " << url;
        continue;
      }

      const std::string& mimetype = result.optimized_content_mime_type();
      scoped_ptr<DictionaryValue> entry(new DictionaryValue());
      entry->SetString("filename", pagespeed::ChooseOutputFilename(
          gurl, mimetype, MD5String(content)));
      entry->SetString("mimetype", mimetype);
      entry->SetString("content", encoded);
      optimized_content->Set(key, entry.release());
    }
  }
}

}  // namespace

namespace pagespeed_chromium {

// Parse the HAR data and run the Page Speed rules, then format the results.
// Return false if the HAR data could not be parsed, true otherwise.
// This function will take ownership of the filter and document arguments, and
// will delete them before returning.
bool RunPageSpeedRules(const std::string& har_data,
                       const std::string& document_data,
                       const std::string& timeline_data,
                       const std::string& resource_filter_name,
                       const std::string& locale,
                       bool save_optimized_content,
                       std::string* output_string,
                       std::string* error_string) {
  // Instantiate an AtExitManager so our Singleton<>s are able to
  // schedule themselves for destruction.
  base::AtExitManager at_exit_manager;

  // Parse the HAR into a PagespeedInput object.  ParseHttpArchiveWithFilter
  // will ensure that filter gets deleted.
  scoped_ptr<pagespeed::PagespeedInput> input(
      pagespeed::ParseHttpArchiveWithFilter(
          har_data, NewFilter(resource_filter_name)));
  if (input.get() == NULL) {
    *error_string = "could not parse HAR";
    return false;
  }

  pagespeed::InstrumentationDataVector timeline_protos;
  STLElementDeleter<pagespeed::InstrumentationDataVector>
      timeline_deleter(&timeline_protos);
  if (!pagespeed::timeline::CreateTimelineProtoFromJsonString(
          timeline_data, &timeline_protos)) {
    *error_string = "error in timeline data";
    return false;
  }

  std::string error_msg_out;
  scoped_ptr<const Value> document_json(base::JSONReader::ReadAndReturnError(
      document_data,
      true,  // allow_trailing_comma
      NULL,  // error_code_out (ReadAndReturnError permits NULL here)
      &error_msg_out));
  if (document_json == NULL) {
    *error_string = "could not parse DOM: " + error_msg_out;
    return false;
  }
  if (!document_json->IsType(Value::TYPE_DICTIONARY)) {
    *error_string = "DOM must be a JSON dictionary";
    return false;
  }

  // Ownership of the document_json is transferred to the returned
  // DomDocument instance.
  pagespeed::DomDocument* document = pagespeed::dom::CreateDocument(
      static_cast<const DictionaryValue*>(document_json.release()));

  // Add the DOM document to the PagespeedInput object.
  if (document != NULL) {
    input->SetPrimaryResourceUrl(document->GetDocumentUrl());
  }
  input->AcquireDomDocument(document); // input takes ownership of document

  // Finish up the PagespeedInput object and freeze it.
  input->AcquireInstrumentationData(&timeline_protos);
  input->AcquireImageAttributesFactory(
      new pagespeed::image_compression::ImageAttributesFactory());
  input->Freeze();

  std::vector<pagespeed::Rule*> rules;

  // In environments where exceptions can be thrown, use
  // STLElementDeleter to make sure we free the rules in the event
  // that they are not transferred to the Engine.
  STLElementDeleter<std::vector<pagespeed::Rule*> > rule_deleter(&rules);

  pagespeed::rule_provider::AppendPageSpeedRules(
      save_optimized_content, &rules);
  std::vector<std::string> incompatible_rule_names;
  pagespeed::rule_provider::RemoveIncompatibleRules(
      &rules, &incompatible_rule_names, input->EstimateCapabilities());
  if (!incompatible_rule_names.empty()) {
    LOG(INFO) << "Removing incompatible rules: "
              << JoinString(incompatible_rule_names, ' ');
  }

  // Ownership of rules is transferred to the Engine instance.
  pagespeed::Engine engine(&rules);
  engine.Init();

  // Compute results.
  pagespeed::Results results;
  if (!engine.ComputeResults(*input, &results)) {
    std::vector<std::string> error_rules;
    for (int i = 0, size = results.error_rules_size(); i < size; ++i) {
      error_rules.push_back(results.error_rules(i));
    }
    LOG(WARNING) << "Errors during ComputeResults in rules: "
                 << JoinString(error_rules, ' ');
  }

  // Format results.
  pagespeed::FormattedResults formatted_results;
  {
    scoped_ptr<pagespeed::l10n::Localizer> localizer(
        pagespeed::l10n::GettextLocalizer::Create(locale));
    if (localizer.get() == NULL) {
      LOG(WARNING) << "Could not create GettextLocalizer for " << locale;
      localizer.reset(new pagespeed::l10n::BasicLocalizer);
    }

    formatted_results.set_locale(localizer->GetLocale());
    pagespeed::formatters::ProtoFormatter formatter(localizer.get(),
                                                    &formatted_results);
    pagespeed::ResponseByteResultFilter result_filter;
    if (!engine.FormatResults(results, result_filter, &formatter)) {
      *error_string = "error during FormatResults";
      return false;
    }
  }

  // The ResponseByteResultFilter may filter some results. In the
  // event that all results are filtered from a FormattedRuleResults,
  // we update its score to 100 and impact to 0, to reflect the fact
  // that we are not showing any suggestions. Likewise, if we find no
  // results in any rules, we set the overall score to 100. This is a
  // hack to work around the fact that scores are computed before we
  // filter. See
  // http://code.google.com/p/page-speed/issues/detail?id=476 for the
  // relevant bug.
  bool has_any_results = false;
  for (int i = 0; i < formatted_results.rule_results_size(); ++i) {
    pagespeed::FormattedRuleResults* rule_results =
        formatted_results.mutable_rule_results(i);
    if (rule_results->url_blocks_size() == 0) {
      rule_results->set_rule_score(100);
      rule_results->set_rule_impact(0.0);
    } else {
      has_any_results = true;
    }
  }
  if (!has_any_results) {
    formatted_results.set_score(100);
  }

  // Convert the formatted results into JSON:
  scoped_ptr<Value> json_results(
      pagespeed::proto::FormattedResultsToJsonConverter::
      ConvertFormattedResults(formatted_results));
  if (json_results == NULL) {
    *error_string = "failed to ConvertFormattedResults";
    return false;
  }

  // Put optimized resources into JSON:
  scoped_ptr<DictionaryValue> optimized_content(new DictionaryValue);
  if (save_optimized_content) {
    SerializeOptimizedContent(results, optimized_content.get());
  }

  // Serialize all the JSON into a string.
  {
    scoped_ptr<DictionaryValue> root(new DictionaryValue);
    root->Set("results", json_results.release());
    root->Set("optimizedContent", optimized_content.release());
    base::JSONWriter::Write(root.get(), false, output_string);
  }

  return true;
}

}  // namespace pagespeed_chromium

// Copyright 2010 and onwards Google Inc.
// Author: sligocki@google.com (Shawn Ligocki)

#include "net/instaweb/rewriter/public/url_input_resource.h"
#include "net/instaweb/util/public/simple_meta_data.h"
#include "net/instaweb/util/public/string_writer.h"
#include "net/instaweb/util/public/url_fetcher.h"

namespace net_instaweb {

UrlInputResource::UrlInputResource(const StringPiece& url,
                                   const StringPiece& absolute_url,
                                   UrlFetcher* url_fetcher)
    : meta_data_(NULL),
      url_fetcher_(url_fetcher) {
  // url_ holds the original URL from the href, which might be relative to
  // the containing page.  This is what is returned from the url() method,
  // e.g. to encode origin  URLs into url-safe paths for rewriting resources.
  //
  // absolute_url_ incorpoates the base path, if necessary, and is used for
  // initiating HTTP GET requests.
  url.CopyToString(&url_);
  absolute_url.CopyToString(&absolute_url_);
}

UrlInputResource::~UrlInputResource() {
}

bool UrlInputResource::Read(MessageHandler* message_handler) {
  bool ret = true;
  if (!loaded()) {
    StringWriter writer(&contents_);

    // TODO(jmarantz): consider request_headers.  E.g. will we ever
    // get different resources depending on user-agent?
    SimpleMetaData request_headers;
    meta_data_.reset(new SimpleMetaData);
    ret = url_fetcher_->StreamingFetchUrl(
        absolute_url_, request_headers, meta_data_.get(), &writer,
        message_handler);
  }
  return ret;
}

bool UrlInputResource::ContentsValid() const {
  return (loaded() && meta_data_->status_code() == HttpStatus::OK);
}

}  // namespace net_instaweb

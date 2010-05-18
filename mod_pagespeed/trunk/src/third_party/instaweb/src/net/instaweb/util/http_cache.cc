// Copyright 2010 and onwards Google Inc.
// Author: jmarantz@google.com (Joshua Marantz)

#include "net/instaweb/util/public/http_cache.h"
#include "net/instaweb/util/public/cache_interface.h"
#include "net/instaweb/util/public/meta_data.h"
#include "net/instaweb/util/public/string_writer.h"
#include "net/instaweb/util/public/timer.h"
#include "net/instaweb/util/public/writer.h"
#include "net/instaweb/util/util.pb.h"

namespace net_instaweb {

bool HTTPCache::Get(const char* key, MetaData* headers, Writer* writer,
                    MessageHandler* handler) {
  bool ret = false;
  std::string cache_buffer;
  StringWriter string_writer(&cache_buffer);
  if (cache_->Get(key, &string_writer, handler)) {
    // Decode the raw cached contents.  TODO(jmarantz): put this
    // in a templatized wrapper around the cache interface.
    CachedResponse cached_response;
    // TODO(jmarantz): use a MetaData factory so callers can override the
    // code used to handle cache headers.
    if (cached_response.ParseFromString(cache_buffer)) {
      headers->set_status_code(cached_response.status_code());
      headers->set_reason_phrase(cached_response.reason_phrase());
      headers->set_major_version(cached_response.major_version());
      headers->set_minor_version(cached_response.minor_version());

      for (int i = 0; i < cached_response.header_size(); ++i) {
        const CacheHeader& header = cached_response.header(i);
        headers->Add(header.name().c_str(), header.value().c_str());
      }
      headers->ComputeCaching();

      if (force_caching_ ||
          (headers->CacheExpirationTimeMs() > timer_->NowMs())) {
        ret = writer->Write(cached_response.content(), handler);
      } else {
        // This cache entry has expired; evict it to make room for
        // useful data.
        cache_->Delete(key, handler);
      }
    }
  }
  return ret;
}

void HTTPCache::Put(const char* key, const MetaData& headers,
                    const std::string& content,
                    MessageHandler* handler) {
  CachedResponse cached_response;
  cached_response.set_status_code(headers.status_code());
  cached_response.set_reason_phrase(headers.reason_phrase());
  cached_response.set_minor_version(headers.minor_version());
  cached_response.set_major_version(headers.major_version());

  for (int i = 0, n = headers.NumAttributes(); i < n; ++i) {
    CacheHeader* header = cached_response.add_header();
    header->set_name(headers.Name(i));
    header->set_value(headers.Value(i));
  }

  cached_response.set_content(content);
  std::string serialized_cache_response;
  cached_response.SerializeToString(&serialized_cache_response);
  cache_->Put(key, serialized_cache_response, handler);
}

CacheInterface::KeyState HTTPCache::Query(const char* key,
                                          MessageHandler* handler) {
  return cache_->Query(key, handler);
}

}  // namespace net_instaweb

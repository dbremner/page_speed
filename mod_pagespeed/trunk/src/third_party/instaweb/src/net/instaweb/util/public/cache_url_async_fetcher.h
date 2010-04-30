// Copyright 2010 Google Inc.
// Author: jmarantz@google.com (Joshua Marantz)

#ifndef NET_INSTAWEB_UTIL_PUBLIC_CACHE_URL_ASYNC_FETCHER_H_
#define NET_INSTAWEB_UTIL_PUBLIC_CACHE_URL_ASYNC_FETCHER_H_

#include "base/scoped_ptr.h"
#include <string>
#include "net/instaweb/util/public/url_async_fetcher.h"

namespace net_instaweb {

class HTTPCache;
class MessageHandler;
class UrlAsyncFetcher;

// Composes an asynchronous URL fetcher with an http cache, to
// generate an asynchronous caching URL fetcher.
//
// This fetcher will call the callback immediately for entries in the
// cache.  When entries are not in the cache, it will initiate an
// asynchronous 'get' and store the result in the cache, as well as
// calling the passed-in callback.
//
// See also CacheUrlFetcher, which will returns results only for
// URLs still in the cache.
class CacheUrlAsyncFetcher : public UrlAsyncFetcher {
 public:
  CacheUrlAsyncFetcher(HTTPCache* cache, UrlAsyncFetcher* fetcher)
      : http_cache_(cache),
        fetcher_(fetcher) {
  }
  virtual ~CacheUrlAsyncFetcher();

  virtual void StreamingFetch(
      const std::string& url,
      const MetaData& request_headers,
      MetaData* response_headers,
      Writer* fetched_content_writer,
      MessageHandler* message_handler,
      Callback* callback);

 private:
  HTTPCache* http_cache_;
  UrlAsyncFetcher* fetcher_;
};

}  // namespace net_instaweb

#endif  // NET_INSTAWEB_UTIL_PUBLIC_CACHE_URL_ASYNC_FETCHER_H_

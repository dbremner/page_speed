// Copyright 2010 and onwards Google Inc.
// Author: jmarantz@google.com (Joshua Marantz)

#ifndef NET_INSTAWEB_REWRITER_PUBLIC_REWRITE_DRIVER_H_
#define NET_INSTAWEB_REWRITER_PUBLIC_REWRITE_DRIVER_H_

#include <map>
#include "base/scoped_ptr.h"
#include <string>
#include "net/instaweb/util/public/url_async_fetcher.h"

namespace net_instaweb {

class AddHeadFilter;
class BaseTagFilter;
class CacheExtender;
class CssCombineFilter;
class FileSystem;
class Hasher;
class HtmlAttributeQuoteRemoval;
class HtmlParse;
class HtmlWriterFilter;
class ImgRewriteFilter;
class OutlineFilter;
class ResourceManager;
class RewriteFilter;
class UrlAsyncFetcher;
class UrlFetcher;
class Writer;

class RewriteDriver {
 public:
  explicit RewriteDriver(HtmlParse* html_parse,
                         UrlAsyncFetcher* url_async_fetcher);
  ~RewriteDriver();

  // Adds a resource manager and/or resource_server, enabling the rewriting of
  // resources. This will replace any previous resource managers.
  void SetResourceManager(ResourceManager* resource_manager);

  // Sets the base url for resolving relative URLs in a document.  This
  // will *not* necessarily add a base-tag filter, but will change
  // it if AddBaseTagFilter has been called to use this base.
  //
  // Neither AddBaseTagFilter or SetResourceManager should be called after this.
  void SetBaseUrl(const char* base);

  // Adds a filter that adds a 'head' section to html documents if
  // none found prior to the body.
  void AddHead();

  // Adds a filter that establishes a base tag for the HTML document.
  // This is required when implementing a proxy server.  The base
  // tag used can be changed for every request with SetBaseUrl.
  // Adding the base-tag filter will establish the AddHeadFilter
  // if needed.
  void AddBaseTagFilter();

  // Extend the cache lifetime of resources.  This can only be called once and
  // requires a resource_manager to be set.
  void ExtendCacheLifetime(Hasher* hasher);

  // Combine CSS files in html document.  This can only be called once and
  // requires a resource_manager to be set.
  void CombineCssFiles();

  // Cut out inlined styles and scripts and make them into external resources.
  // This can only be called once and requires a resource_manager to be set.
  void OutlineResources(bool outline_styles, bool outline_scripts);

  // Log encountered image urls.  Eventually rewrite them to reduce
  // file size, and possibly insert missing image sizes into img refs.
  void RewriteImages();

  // Remove extraneous quotes from html attributes.  Does this save enough bytes
  // to be worth it after compression?  In small examples it actually costs us
  // post-compression bytes.
  void RemoveQuotes();

  // Controls how HTML output is written.  Be sure to call this last, after
  // all other filters have been established.
  //
  // TODO(jmarantz): fix this in the implementation so that the caller can
  // install filters in any order and the writer will always be last.
  void SetWriter(Writer* writer);

  void FetchResource(const char* resource,
                     const MetaData& request_headers,
                     MetaData* response_headers,
                     Writer* writer,
                     MessageHandler* message_handler,
                     UrlAsyncFetcher::Callback* callback);

  HtmlParse* html_parse() { return html_parse_; }

 private:
  typedef std::map<std::string, RewriteFilter*> ResourceFilterMap;
  ResourceFilterMap resource_filter_map_;

  // These objects are provided on construction or later, and are
  // owned by the caller.
  HtmlParse* html_parse_;
  UrlAsyncFetcher* url_async_fetcher_;
  ResourceManager* resource_manager_;

  scoped_ptr<AddHeadFilter> add_head_filter_;
  scoped_ptr<BaseTagFilter> base_tag_filter_;
  scoped_ptr<CacheExtender> cache_extender_;
  scoped_ptr<CssCombineFilter> css_combine_filter_;
  scoped_ptr<HtmlAttributeQuoteRemoval> attribute_quote_removal_;
  scoped_ptr<ImgRewriteFilter> img_rewrite_filter_;
  scoped_ptr<OutlineFilter> outline_filter_;
  scoped_ptr<HtmlWriterFilter> html_writer_filter_;
};

}  // namespace net_instaweb

#endif  // NET_INSTAWEB_REWRITER_PUBLIC_REWRITE_DRIVER_H_

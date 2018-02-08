* **ex001** - fetch [www.google.it](www.google.it)
  * `curl_global_init` + `curl_global_cleanup`
  * CURLOPT_WRITEFUNCTION + CURLOPT_WRITEDATA (output to `std::string` or `FILE*`)
  * CURLOPT_ERRORBUFFER
  * CURLOPT_PROXY + CURLOPT_SSL_VERIFYPEER
  * CURLINFO_RESPONSE_CODE + CURLINFO_CONTENT_TYPE

* **ex002** - fetch with password ( [https://httpbin.org](https://httpbin.org) )
  * CURLAUTH_BASIC +  CURLAUTH_DIGEST
  * CURLOPT_USERNAME + CURLOPT_PASSWORD
  * CURLOPT_VERBOSE
  * reuse curl handle
  * `curl_easy_reset`
  * `CURLOPT_HTTP_VERSION` + `CURL_HTTP_VERSION_2_0` [https://http2.akamai.com/demo](https://http2.akamai.com/demo)
  * example compute JwtToken [https://jwt.io](https://jwt.io)

* **ex003** - fetch large download ( [http://ipv4.download.thinkbroadband.com/](http://ipv4.download.thinkbroadband.com/) )
  * STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES + MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL
  * CURLOPT_XFERINFOFUNCTION + CURLOPT_XFERINFODATA + CURLOPT_NOPROGRESS
  * CURLINFO_CONTENT_LENGTH_DOWNLOAD
  * `curl_easy_escape` + `curl_free`

* **ex004** - cookies ( [https://httpbin.org](https://httpbin.org) )
  * CURLOPT_FOLLOWLOCATION
  * CURLOPT_COOKIEJAR + CURLOPT_COOKIEFILE

* **ex005** - post verb and multipart/form-data ( [https://httpbin.org](https://httpbin.org) )
  * CURLOPT_POST + CURLOPT_POSTFIELDS + CURLOPT_POSTFIELDSIZE
  * `curl_mime_init` + CURLOPT_MIMEPOST + `curl_mime_free`
  * `curl_mime_addpart` + `curl_mime_name` + `curl_mime_data` + `curl_mime_filename` + `curl_mime_type` + `curl_mime_filedata`
  * `curl_slist_append` + CURLOPT_HTTPHEADER + `curl_slist_free_all`
  * read proxy configuration from registry

* **ex006** - multi interface
  * CURLOPT_PRIVATE + CURLINFO_PRIVATE
  * `curl_multi_init` + `curl_multi_cleanup` + `curl_multi_setopt`
  * `curl_multi_add_handle` + `curl_multi_perform` + `curl_multi_info_read` + `curl_multi_remove_handle`

* **ex007** - ftps
  * [https://curl.haxx.se/libcurl/relatedlibs.html](https://curl.haxx.se/libcurl/relatedlibs.html)

* **ex008*** - slack API
  * rapidjson [http://rapidjson.org/](http://rapidjson.org/)
  * fmt::format [http://fmtlib.net/](http://fmtlib.net/)
  *  [https://bignoplayground.slack.com](https://bignoplayground.slack.com)
// RESTClientConsoleWin32.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "curl.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <assert.h>

#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

struct string2 {
  char *ptr;
  size_t len;
};

void init_string(struct string2 *s) {
  s->len = 0;
  s->ptr = (char *)malloc(s->len + 16 * 1024);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string2 *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char *)realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  //fprintf(stdout, "total %d bytes data returned!\n", size * nmemb);
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

static void
parse_resp_body(const char* body_json, int len)
{
	fprintf(stdout, "parse body ....\n\n");
	//fprintf(stdout, "%s\n", body);
	Document document;
	document.Parse(body_json);
	assert(document.IsObject());
	assert(document.HasMember("1"));
	assert(document["1"].IsString());
	printf("the str @location#1 = %s\n", document["1"].GetString());

	return;
}

static void 
parse_response(CURL* cURL_handle, const string2* response)
{
	CURLcode 	rc = (CURLcode)0;
	long		header_len  = 0;
	double 		content_len = 0;

	// Get content length
	rc = curl_easy_getinfo(cURL_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_len);
	if (rc == -1) {
		fprintf(stderr, "Failed to get content length information...\n");
	} else {
		fprintf(stdout, "Content length is %d bytes\n", (int)content_len);
	}

	// Get Header length
	rc = curl_easy_getinfo(cURL_handle, CURLINFO_HEADER_SIZE, &header_len);
	if (rc == -1) {
		fprintf(stderr, "Failed to get header length information...\n");
	} else {
		fprintf(stdout, "Header length is %d bytes\n", header_len);
	}

	// Verify if the 2 values correct
	if (header_len + (int)content_len != response->len) {
		fprintf(stderr, "WARNING: in-consistent size information (%s:line#%d) ... \n", __FILE__, __LINE__);
		fprintf(stderr, "Total %d bytes received, but Header len is %d bytes and Body len is %d bytes ... \n", 
				response->len, header_len, (int)content_len);
	}

	// start the parsing
	char* header_p = (char*) calloc(header_len + 1, 1);
	assert(header_p != NULL);
	char* content_p = NULL;
	int content_len2 = ((int)content_len == -1) ? (response->len - header_len) : (int)content_len;
	fprintf(stdout, "WARNING: assuming body length is %d bytes\n", content_len2);
	content_p = (char*) calloc(content_len2 + 1, 1);
	assert(content_p != NULL);
	
	memcpy(header_p,  response->ptr, header_len);
	*(header_p + header_len) = '\0';
	memcpy(content_p, response->ptr + header_len, content_len2);
	*(content_p + content_len2) = '\0';
	
	printf("HEADER: %s\n", header_p);
	parse_resp_body(content_p, content_len2);

	free(header_p);
	free(content_p);


	return;
}

static void do_REST_API(const char* url)
{
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if (curl) {
	struct string2 s;
    init_string(&s);
    curl_easy_setopt(curl, CURLOPT_URL, url);
 
#define SKIP_PEER_VERIFICATION
#ifdef  SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#define SKIP_HOSTNAME_VERIFICATION
#ifdef  SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	// forcedly set the agent info and accept data format
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.41.0");
	struct curl_slist *list = NULL;
	list = curl_slist_append(list, "Accept: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	// FIXME: forced to use TSLv1.0
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s); 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if (res != CURLE_OK)
      fprintf(stderr, "cURL request failed: %s@%s:%d\n",
              curl_easy_strerror(res), __FILE__, __LINE__);
	else {
	  fprintf(stdout, "cURL request executed successfully!\n");	  
      fprintf(stdout, "response as followings: \n\n");
      fprintf(stdout, "%s\n\n", s.ptr);
	  parse_response(curl, &s);
      free(s.ptr);	  
 	}

    /* always cleanup */ 
    curl_easy_cleanup(curl);
	curl_slist_free_all(list); /* free the list again */
	} // valid curl
	return;
}

static void qualify_ssl(void)
{
	curl_version_info_data *vinfo = curl_version_info(CURLVERSION_NOW);
    // SSL support enabled
	if(vinfo->features & CURL_VERSION_SSL) {
		fprintf(stdout, "The libcurl used is SSL enabled!\n");
		return;
	} else { // No SSL	
		fprintf(stdout, "The libcurl used has No SSL supported. Abort ...\n");
		exit(1);
	}
}

#if 0
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	qualify_ssl();
	do_REST_API(argv[1]);
	return 0;
}

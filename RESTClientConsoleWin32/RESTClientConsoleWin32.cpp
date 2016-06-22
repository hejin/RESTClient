// RESTClientConsoleWin32.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "curl.h"
#include <string>
#include <sstream>

using namespace std;

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

static void do_REST_API(const char* url)
{
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {
	string content;  
	struct string2 s;
    init_string(&s);
    //curl_easy_setopt(curl, CURLOPT_URL, "https://192.168.17.134/mobile/list/1");
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
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.41.0");
	struct curl_slist *list = NULL;
	list = curl_slist_append(list, "Accept: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
 	//curl_easy_setopt(curl, CURLOPT_HEADER, "User-Agent: curl/7.41.0");	
	//curl_easy_setopt(curl, CURLOPT_HEADER, "Accept: application/json");
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s); 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
	else {
	  fprintf(stdout, "curl_easy_perform() OK!\n");	  
      fprintf(stdout, "response: %s\n", s.ptr);
      free(s.ptr);	  
	  //ostringstream out;
      //out << res;
      //content = out.str();
      //fprintf(stdout, "response: %s\n", content);
	  //cout << out << endl;
 	}

    /* always cleanup */ 
    curl_easy_cleanup(curl);
	curl_slist_free_all(list); /* free the list again */
	} // valid curl

	curl_version_info_data * vinfo = curl_version_info( CURLVERSION_NOW );
	if( vinfo->features & CURL_VERSION_SSL )
    // SSL support enabled
		fprintf(stdout, "SSL supported!\n");
	else
		fprintf(stdout, "No SSL supported...\n");
    // No SSL	
}

#if 0
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	do_REST_API(argv[1]);
	return 0;
}

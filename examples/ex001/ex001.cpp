// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

#define USE_FIDDLER

size_t helperWriteString(void *ptr, size_t size, size_t numelem, void *userp)
{
	size_t realsize = size * numelem;
	std::string* memToWrite = static_cast<std::string*>(userp);
	memToWrite->append(static_cast<const char *>(ptr), realsize);
	return realsize;
}

int main(int argc, char *argv[])
{
	CURLcode ret;
	CURL *hnd;

	curl_global_init(CURL_GLOBAL_ALL);

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L); //100Kb
	curl_easy_setopt(hnd, CURLOPT_URL, "https://www.google.it");
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, "..\\..\\distrib\\curl-ca-bundle.crt");
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	FILE* fp(0);
	std::string replyString;

	fopen_s(&fp, "result.dat", "wb");

	if ( fp )
	{
		replyString = "See result.dat";
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, fwrite);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, static_cast<void*>(fp));
	}
	else
	{
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, helperWriteString);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, static_cast<void *>(&replyString));
	}

	char last_error_message[CURL_ERROR_SIZE];
	last_error_message[0] = 0;
	curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, last_error_message);

#ifdef USE_FIDDLER
	curl_easy_setopt(hnd, CURLOPT_PROXY, "127.0.0.1:8888");
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	/* Here is a list of options the curl code used that cannot get generated
	as source easily. You may select to either not use them or implement
	them yourself.

	CURLOPT_WRITEDATA set to a objectpointer
	CURLOPT_INTERLEAVEDATA set to a objectpointer
	CURLOPT_WRITEFUNCTION set to a functionpointer
	CURLOPT_READDATA set to a objectpointer
	CURLOPT_READFUNCTION set to a functionpointer
	CURLOPT_SEEKDATA set to a objectpointer
	CURLOPT_SEEKFUNCTION set to a functionpointer
	CURLOPT_ERRORBUFFER set to a objectpointer
	CURLOPT_STDERR set to a objectpointer
	CURLOPT_HEADERFUNCTION set to a functionpointer
	CURLOPT_HEADERDATA set to a objectpointer
	*/

	std::cout << "www.google.it" << std::endl;

	ret = curl_easy_perform(hnd);

	if(fp)	fclose(fp);

	long httpStatus(0);
	CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

	std::cout << "http response code: " << httpStatus << std::endl;

	if (CURLE_OK == res && httpStatus == 200)
	{
		char *contentType(0);
		res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

		std::cout << "http content type: " << contentType << std::endl;

		if (CURLE_OK == res && contentType && strncmp(contentType, "text/html", 9) == 0)
		{
			std::cout << replyString.substr(0,100) << "..." << std::endl;
		}
	}
	curl_easy_cleanup(hnd);
	curl_global_cleanup();

	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;
	
	::system("pause");

	return (int)ret;
}



// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // win32

void mySleep(int milliseconds) // Cross-platform sleep function
{
#ifdef WIN32
	Sleep(milliseconds);
#else
	usleep(milliseconds * 1000);
#endif // win32
}

size_t helperWriteString(void *ptr, size_t size, size_t numelem, void *userp)
{
	size_t realsize = size * numelem;
	std::string* memToWrite = static_cast<std::string*>(userp);
	memToWrite->append(static_cast<const char *>(ptr), realsize);
	return realsize;
}

int main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_ALL);

	CURLcode ret;
	CURL *hnd;

	std::string theUrl = "ftps://127.0.0.1/esempio.txt";
//	std::string theUrl = "ftps://127.0.0.1/";

	unsigned long auth_method = CURLAUTH_ANY;

	std::string username = "user01";
	std::string userpassword = "welcome";

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L); //100Kb
	curl_easy_setopt(hnd, CURLOPT_URL, theUrl.c_str());
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, "..\\..\\distrib\\bigno_certificate.crt");
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	std::string replyString;
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, helperWriteString);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, static_cast<void *>(&replyString));

	char last_error_message[CURL_ERROR_SIZE];
	last_error_message[0] = 0;
	curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, last_error_message);

#ifdef USE_FIDDLER
	curl_easy_setopt(hnd, CURLOPT_PROXY, "127.0.0.1:8888");
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, auth_method);
	curl_easy_setopt(hnd, CURLOPT_USERNAME, username.c_str());
	curl_easy_setopt(hnd, CURLOPT_PASSWORD, userpassword.c_str());

	std::cout << theUrl << std::endl;

	ret = curl_easy_perform(hnd);
	long httpStatus(0);
	CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

	std::cout << "http response code: " << httpStatus << std::endl;

	if (CURLE_OK == res && (httpStatus >= 200 && httpStatus < 300))
	{
		std::cout << replyString << std::endl;
	}

	curl_easy_cleanup(hnd);
	curl_global_cleanup();
	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;

	::system("pause");

	return (int)ret;
}
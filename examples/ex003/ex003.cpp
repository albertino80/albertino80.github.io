// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

//#define USE_FIDDLER

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

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         1024 * 1024 * 100
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1

struct myprogress {
	double lastruntime;
	curl_off_t lastdl;
	CURL *curl;
};

/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
static int xferinfo(void *p,
	curl_off_t dltotal, curl_off_t dlnow,
	curl_off_t ultotal, curl_off_t ulnow)
{
	struct myprogress *myp = (struct myprogress *)p;
	CURL *curl = myp->curl;
	double curtime = 0;

	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

	/* under certain circumstances it may be desirable for certain functionality
	to only run every N seconds, in order to do this the transaction time can
	be used */
	if ((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
		double speed = ((dlnow - myp->lastdl) / (curtime - myp->lastruntime) / 1024 / 1024);

		myp->lastruntime = curtime;
		myp->lastdl = dlnow;

		fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
		fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T "  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T " Speed: %.2f Mbps\r\n", ulnow, ultotal, dlnow, dltotal, speed);
	}

	if (dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
		return 1;
	return 0;
}

int main(int argc, char *argv[])
{
	CURLcode ret;
	CURL *hnd;

	curl_global_init(CURL_GLOBAL_ALL);

	std::string theUrl = "http://ipv4.download.thinkbroadband.com/512MB.zip";
	unsigned long auth_method = CURLAUTH_ANY;

	std::string username = "alberto";
	std::string userpassword = "pass";

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L); //100Kb
	curl_easy_setopt(hnd, CURLOPT_URL, theUrl.c_str());
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, "..\\..\\distrib\\curl-ca-bundle.crt");
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
	curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
#endif

	curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, auth_method);
	curl_easy_setopt(hnd, CURLOPT_USERNAME, username.c_str());
	curl_easy_setopt(hnd, CURLOPT_PASSWORD, userpassword.c_str());

	struct myprogress prog;
	prog.lastruntime = 0;
	prog.curl = hnd;
	prog.lastdl = 0;
	
	curl_easy_setopt(hnd, CURLOPT_XFERINFOFUNCTION, xferinfo);
	curl_easy_setopt(hnd, CURLOPT_XFERINFODATA, &prog);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 0L);

	for (int nCall = 0; nCall < 1; nCall++)
	{
		replyString.clear();

		if (nCall > 0)
			mySleep(1000);

		std::cout << theUrl << " nCall:" << nCall << std::endl;

		ret = curl_easy_perform(hnd);
		long httpStatus(0);
		CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

		std::cout << "http response code: " << httpStatus << std::endl;

		if (CURLE_OK == res && httpStatus == 200)
		{
			double contentLength(0);
			char *contentType(0);
			res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);
			res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);

			std::cout << "http content type: " << contentType << std::endl;
			std::cout << "http content lenght: " << (int)contentLength << std::endl;

			if (CURLE_OK == res && contentType && strncmp(contentType, "application/zip", 15) == 0)
			{
				char *output = curl_easy_escape(hnd, replyString.substr(0, 10).c_str(), 10);
				std::cout << output << "..." << std::endl;
				curl_free(output);
			}
		}
	}

	curl_easy_cleanup(hnd);
	curl_global_cleanup();

	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;

	::system("pause");

	return (int)ret;
}
// ex006.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <curl/multi.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

static const char *urls[] = {
	"http://www.microsoft.com",
	"http://www.opensource.org",
	"http://www.google.com",
	"http://www.yahoo.com",
	"http://www.ibm.com",
	"http://www.mysql.com",
	"http://www.oracle.com",
	"http://www.ripe.net",
	"http://www.iana.org",
	"http://www.amazon.com",
	"http://www.netcraft.com",
	"http://www.heise.de",
	"http://www.chip.de",
	"http://www.ca.com",
	"http://www.cnet.com",
	"http://www.news.com",
	"http://www.cnn.com",
	"http://www.wikipedia.org",
	"http://www.dell.com",
	"http://www.hp.com",
	"http://www.cert.org",
	"http://www.mit.edu",
	"http://www.nist.gov",
	"http://www.ebay.com",
	"http://www.playstation.com",
	"http://www.uefa.com",
	"http://www.ieee.org",
	"http://www.apple.com",
	"http://www.symantec.com",
	"http://www.zdnet.com",
	"http://www.fujitsu.com",
	"http://www.supermicro.com",
	"http://www.hotmail.com",
	"http://www.ecma.com",
	"http://www.bbc.co.uk",
	"http://news.google.com",
	"http://www.foxnews.com",
	"http://www.msn.com",
	"http://www.wired.com",
	"http://www.sky.com",
	"http://www.usatoday.com",
	"http://www.cbs.com",
	"http://www.nbc.com",
	"http://slashdot.org",
	"http://www.techweb.com",
	"http://www.newslink.org",
	"http://www.un.org",
};

#define MAX 10  
#define TOTAL_URLS sizeof(urls)/sizeof(char *) 
#define WAIT_TIMEOUT_SECS 5

FILE* fp[TOTAL_URLS];

static void init(CURLM *multiHandle, int i)
{
	CURL *eh = curl_easy_init();

	std::string theUrl = urls[i];

	curl_easy_setopt(eh, CURLOPT_URL, theUrl.c_str());
	curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
	curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(eh, CURLOPT_CAINFO, "..\\..\\distrib\\curl-ca-bundle.crt");

	std::replace(theUrl.begin(), theUrl.end(), ':', '_');
	std::replace(theUrl.begin(), theUrl.end(), '/', '_');
	std::replace(theUrl.begin(), theUrl.end(), '.', '_');
	theUrl.append(".txt");

	fopen_s(&fp[i], theUrl.c_str(), "wb");
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, fwrite);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, static_cast<void*>(fp[i]));

	curl_multi_add_handle(multiHandle, eh);
}

int main(void)
{
	CURLM *multiHandle;
	CURLMsg *msg;
	unsigned int currentRequest = 0;
	int numMessages, runningHandles = -1;

	curl_global_init(CURL_GLOBAL_ALL);

	multiHandle = curl_multi_init();

	//we can optionally limit the total amount of connections this multi handle uses
	curl_multi_setopt(multiHandle, CURLMOPT_MAXCONNECTS, (long)MAX);

	for (currentRequest = 0; currentRequest < MAX; ++currentRequest)
	{
		init(multiHandle, currentRequest);
	}

	bool errorFound(false);
	while (runningHandles && !errorFound)
	{
		curl_multi_perform(multiHandle, &runningHandles);

		if (runningHandles)
		{
			int numfds(0);
			int res = curl_multi_wait(multiHandle, NULL, 0, WAIT_TIMEOUT_SECS, &numfds);
			if (res != CURLM_OK)
			{
				errorFound = true;
			}
		}

		while ((msg = curl_multi_info_read(multiHandle, &numMessages)))
		{
			if (msg->msg == CURLMSG_DONE)
			{
				char *url;
				CURL *e = msg->easy_handle;
				long httpStatus(0);
				curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &httpStatus);
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
				fprintf(stderr, "R: %d - http %d - %s <%s>\n", msg->data.result, httpStatus, curl_easy_strerror(msg->data.result), url);
				curl_multi_remove_handle(multiHandle, e);
				curl_easy_cleanup(e);
			}
			else
			{
				fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
			}
			
			if (currentRequest < TOTAL_URLS) 
			{
				init(multiHandle, currentRequest++);
				runningHandles++; // just to prevent it from remaining at 0 if there are more URLs to get
			}
		}
	}

	curl_multi_cleanup(multiHandle);
	curl_global_cleanup();

	for (currentRequest = 0; currentRequest < TOTAL_URLS; ++currentRequest)
	{
		fclose(fp[currentRequest]);
	}

	::system("pause");

	return EXIT_SUCCESS;
}
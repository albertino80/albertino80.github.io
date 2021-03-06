// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>

//#define USE_FIDDLER
//#define USE_DIGEST

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // win32

void testOpenSSL();

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

#ifdef USE_DIGEST
	std::string theUrl = "https://httpbin.org/digest-auth/auth/alberto/pass/MD5/never";
	unsigned long auth_method = CURLAUTH_DIGEST;
#else
	std::string theUrl = "https://httpbin.org/basic-auth/alberto/pass";
	unsigned long auth_method = CURLAUTH_BASIC;
#endif // USE_DIGEST

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
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
	
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

	for (int nCall = 0; nCall < 5; nCall++)
	{
		replyString.clear();

		if(nCall > 0)
			mySleep(1000);

		if (nCall == 3)
		{
			curl_easy_reset(hnd);
			curl_easy_setopt(hnd, CURLOPT_URL, theUrl.c_str());
		}

		std::cout << theUrl << " nCall:" << nCall << std::endl;

		ret = curl_easy_perform(hnd);
		long httpStatus(0);
		CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

		std::cout << "http response code: " << httpStatus << std::endl;

		if (CURLE_OK == res && httpStatus == 200)
		{
			char *contentType(0);
			res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

			std::cout << "http content type: " << contentType << std::endl;

			if (CURLE_OK == res && contentType && strncmp(contentType, "application/json", 16) == 0)
			{
				std::cout << replyString << std::endl;
			}
		}
	}

	curl_easy_cleanup(hnd);
	curl_global_cleanup();
	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;
	
	testOpenSSL();

	::system("pause");

	return (int)ret;
}

std::string encodeBase64(const std::string& theData)
{
	char encodedData[1000];
	int encodedLen = EVP_EncodeBlock((unsigned char *)encodedData, (unsigned char *)theData.c_str(), (int)theData.size());
	return std::string(encodedData, encodedLen);
}

void testOpenSSL()
{
	unsigned char result[64];
	unsigned int len = 64;

	std::string theHeader = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
	std::string theBody = "{\"iss\":\"c++\",\"sub\":\"Alberto\"}";
	std::string secretKey = "Marco Arena";

	std::string jwtToken = encodeBase64(theHeader);
	jwtToken.append(".");
	jwtToken.append(encodeBase64(theBody));

	HMAC_CTX* ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, secretKey.c_str(), (int)secretKey.size(), EVP_sha256(), NULL);
	HMAC_Update(ctx, (unsigned char*)jwtToken.c_str(), (int)jwtToken.size());
	HMAC_Final(ctx, result, &len);
	HMAC_CTX_free(ctx);

	jwtToken.append(".");

	jwtToken.append(encodeBase64(std::string((const char*)result, len)));

	std::cout << "JwtToken: " << jwtToken << std::endl;
}
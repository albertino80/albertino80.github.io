// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

#include <sstream>
#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#endif

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

void set_proxy_from_registry(CURL *hnd)
{
#ifdef _WIN32
	DWORD vProxyEnable = 0;
	DWORD dwType = 0;
	DWORD sProxyEnable = sizeof(DWORD);

	LONG err_code = SHGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", L"ProxyEnable", &dwType, &vProxyEnable, &sProxyEnable);
	if (err_code == ERROR_SUCCESS && vProxyEnable == 1)
	{
		wchar_t vProxyServer[1024];
		DWORD sProxyServer = 1024;

		dwType = REG_SZ;
		err_code = SHGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", L"ProxyServer", &dwType, &vProxyServer, &sProxyServer);
		if (err_code == ERROR_SUCCESS)
		{
			const int size = WideCharToMultiByte(CP_UTF8, 0, vProxyServer, -1, NULL, 0, NULL, NULL);
			char* single_byte = new char[size];
			WideCharToMultiByte(CP_UTF8, 0, vProxyServer, -1, single_byte, size, NULL, NULL);

			std::istringstream f(single_byte);
			std::string s;
			while (std::getline(f, s, ';')) {
				if(s.substr(0, 5) == "http=")
				{
					curl_easy_setopt(hnd, CURLOPT_PROXY, s.substr(5).c_str());
					curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
				}
			}

			delete[] single_byte;
		}
	}
#endif
}

size_t helperWriteString(void *ptr, size_t size, size_t numelem, void *userp)
{
	size_t realsize = size * numelem;
	std::string* memToWrite = static_cast<std::string*>(userp);
	memToWrite->append(static_cast<const char *>(ptr), realsize);
	return realsize;
}

void makePostRequest(CURL *hnd, std::string& replyString)
{
	CURLcode ret;
	std::string theUrl = "https://httpbin.org/post";

	std::string thePostData = "This is a long string in order to test post request";

	curl_easy_setopt(hnd, CURLOPT_URL, theUrl.c_str());
	curl_easy_setopt(hnd, CURLOPT_POST, 1L);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, thePostData.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, thePostData.size());

	struct curl_slist *list = NULL;
	list = curl_slist_append(list, "Content-Type: text/plain");
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, list);

	replyString.clear();
	ret = curl_easy_perform(hnd);

	curl_slist_free_all(list);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, 0L);

	long httpStatus(0);
	CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

	std::cout << __FUNCTION__ << " - http response code: " << httpStatus << std::endl;

	if (CURLE_OK == res && httpStatus == 200)
	{
		char *contentType(0);
		res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

		std::cout << __FUNCTION__ << " - http content type: " << contentType << std::endl;

		if (CURLE_OK == res && contentType && (strncmp(contentType, "application/json", 16) == 0 || strncmp(contentType, "text", 4) == 0))
		{
			std::cout << __FUNCTION__ << " - " << replyString << std::endl;
		}
	}
}

bool multipartAddPart(curl_mime *multipart, const std::string& partName, const char* partData, int partLen, const std::string& fileNameForReceiver, const std::string& p_content_type)
{
	curl_mimepart *part = curl_mime_addpart(multipart);
	curl_mime_name(part, partName.c_str());
	curl_mime_data(part, partData, partLen);
	if (!fileNameForReceiver.empty())
		curl_mime_filename(part, fileNameForReceiver.c_str());
	if (!p_content_type.empty())
		curl_mime_type(part, p_content_type.c_str());

	//see also curl_mime_filedata

	return true;
}

void makeMultiPartRequest(CURL *hnd, std::string& replyString)
{
	CURLcode ret;
	std::string theUrl = "https://httpbin.org/post";

	curl_easy_setopt(hnd, CURLOPT_URL, theUrl.c_str());

	std::string thePostData1 = "This is a long string in order to test post request";
	std::string thePostData2 = "Another part";
	std::string thePostData3 = "The final part";

	curl_mime *multipart = curl_mime_init(hnd);
	multipartAddPart(multipart, "part_01", thePostData1.c_str(), (int)thePostData1.size(), "partOne.txt", "text/plain");
	multipartAddPart(multipart, "part_02", thePostData2.c_str(), (int)thePostData2.size(), "", "");
	multipartAddPart(multipart, "part_03", thePostData3.c_str(), (int)thePostData3.size(), "", "");
	curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);

	replyString.clear();
	ret = curl_easy_perform(hnd);
	long httpStatus(0);
	CURLcode res = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);

	curl_mime_free(multipart);

	std::cout << __FUNCTION__ << " - http response code: " << httpStatus << std::endl;

	if (CURLE_OK == res && httpStatus == 200)
	{
		char *contentType(0);
		res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

		std::cout << __FUNCTION__ << " - http content type: " << contentType << std::endl;

		if (CURLE_OK == res && contentType && (strncmp(contentType, "application/json", 16) == 0 || strncmp(contentType, "text", 4) == 0))
		{
			std::cout << __FUNCTION__ << " - " << replyString << std::endl;
		}
	}
}

int main(int argc, char *argv[])
{
	CURL *hnd;

	curl_global_init(CURL_GLOBAL_ALL);

	unsigned long auth_method = CURLAUTH_ANY;

	std::string username = "alberto";
	std::string userpassword = "pass";

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L); //100Kb
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

	set_proxy_from_registry(hnd);

#ifdef USE_FIDDLER
	curl_easy_setopt(hnd, CURLOPT_PROXY, "127.0.0.1:8888");
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
	//curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
#endif

	curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, auth_method);
	curl_easy_setopt(hnd, CURLOPT_USERNAME, username.c_str());
	curl_easy_setopt(hnd, CURLOPT_PASSWORD, userpassword.c_str());

	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_COOKIEJAR, "cookies.dat");
	curl_easy_setopt(hnd, CURLOPT_COOKIEFILE, "cookies.dat");

	makePostRequest(hnd, replyString);
	makeMultiPartRequest(hnd, replyString);

	curl_easy_cleanup(hnd);
	curl_global_cleanup();

	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;

	::system("pause");

	return 0;
}
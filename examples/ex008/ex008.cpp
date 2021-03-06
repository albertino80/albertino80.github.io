// ex001.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>

#include "fmt\format.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

std::string replyString;

size_t helperWriteString(void *ptr, size_t size, size_t numelem, void *userp)
{
	size_t realsize = size * numelem;
	std::string* memToWrite = static_cast<std::string*>(userp);
	memToWrite->append(static_cast<const char *>(ptr), realsize);
	return realsize;
}

std::string getGeneralChannel(CURL *hnd, const std::string& apiEndPoint, const std::string& apiToken)
{
	curl_easy_setopt(hnd, CURLOPT_URL, fmt::format("{}channels.list?token={}", apiEndPoint, apiToken).c_str());

	replyString.clear();
	CURLcode ret = curl_easy_perform(hnd);

	long httpStatus(0);
	ret = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);
	if (CURLE_OK == ret && httpStatus == 200)
	{
		char *contentType(0);
		ret = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

		std::cout << "http content type: " << contentType << std::endl;

		if (CURLE_OK == ret && contentType && strncmp(contentType, "application/json", 16) == 0)
		{
			rapidjson::Document d;
			rapidjson::ParseResult parseOk = d.Parse(replyString.c_str());
			if (parseOk)
			{
				if (d.HasMember("channels"))
				{
					rapidjson::Value& channels = d["channels"];
					for (rapidjson::SizeType i = 0; i < channels.Size(); i++)
					{
						if (channels[i]["name"] == "general")
						{
							return channels[i]["id"].GetString();
						}
					}
				}
			}
		}
	}

	return "";
}

void getGeneralChannelPhotos(CURL *hnd, const std::string& apiEndPoint, const std::string& apiToken, const std::string& channelId, std::vector<std::string>& photoUrls)
{
	curl_easy_setopt(hnd, CURLOPT_URL, fmt::format("{}channels.history?channel={}&token={}", apiEndPoint, channelId, apiToken).c_str());

	replyString.clear();
	CURLcode ret = curl_easy_perform(hnd);

	long httpStatus(0);
	ret = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);
	if (CURLE_OK == ret && httpStatus == 200)
	{
		char *contentType(0);
		ret = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &contentType);

		std::cout << "http content type: " << contentType << std::endl;

		if (CURLE_OK == ret && contentType && strncmp(contentType, "application/json", 16) == 0)
		{
			rapidjson::Document d;
			rapidjson::ParseResult parseOk = d.Parse(replyString.c_str());
			if (parseOk)
			{
				if (d.HasMember("messages"))
				{
					rapidjson::Value& messages = d["messages"];
					for (rapidjson::SizeType i = 0; i < messages.Size(); i++)
					{
						if (messages[i]["type"] == "message")
						{
							if (messages[i].HasMember("subtype"))
							{
								if (messages[i]["subtype"] == "file_share")
								{
									photoUrls.push_back(messages[i]["file"]["url_private"].GetString());
								}
							}
							else
							{
								std::cout << messages[i]["text"].GetString() << std::endl;
							}
						}
					}
				}
			}
		}
	}
}

void downloadPhoto(CURL *hnd, const std::string& photoUrl, const std::string& token, int iPhoto)
{
	curl_easy_setopt(hnd, CURLOPT_URL, photoUrl.c_str());

	FILE* fp(0);
	fopen_s(&fp, fmt::format("photo{}.jpeg", iPhoto).c_str(), "wb");

	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, fwrite);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, static_cast<void*>(fp));

	struct curl_slist *list = NULL;
	list = curl_slist_append(list, fmt::format("Authorization: Bearer {}", token).c_str());
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, list);

	CURLcode ret = curl_easy_perform(hnd);

	curl_slist_free_all(list);

	long httpStatus(0);
	ret = curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpStatus);
	std::cout << "photo: " << iPhoto << " status " << httpStatus << std::endl;
	fclose(fp);
}

int main(int argc, char *argv[])
{
	CURL *hnd;

	std::string apiEndPoint = "https://slack.com/api/";
	std::string apiToken = "xoxp-261395998839-259657860320-279915470182-1d6ec457f2f9e84c91cbdc82b3dfb72d";

	curl_global_init(CURL_GLOBAL_ALL);

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L); //100Kb
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, "..\\..\\distrib\\curl-ca-bundle.crt");
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, helperWriteString);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, static_cast<void *>(&replyString));
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_COOKIEJAR, "cookies.dat");
	curl_easy_setopt(hnd, CURLOPT_COOKIEFILE, "cookies.dat");

	char last_error_message[CURL_ERROR_SIZE];
	last_error_message[0] = 0;
	curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, last_error_message);


	std::string generalId = getGeneralChannel(hnd, apiEndPoint, apiToken);
	if (!generalId.empty())
	{
		std::vector<std::string> photoUrls;
		getGeneralChannelPhotos(hnd, apiEndPoint, apiToken, generalId, photoUrls);
		
		int curPhoto(1);
		for (auto& x : photoUrls)
		{
			downloadPhoto(hnd, x, apiToken, curPhoto++);
		}
	}


	curl_easy_cleanup(hnd);
	curl_global_cleanup();

	hnd = NULL;

	std::cout << "Error buffer: " << last_error_message << std::endl;

	::system("pause");

	return 0;
}



#include "uploadData.h"

std::string uploadData::getKeyname()
{
	time_t systemTime = time(NULL);

	std::string localTime = ctime(&systemTime);
	std::string name;
	name.resize(localTime.size() - 1);

	for (size_t i = 0; i < localTime.size() - 1; i++)
	{
		if (localTime[i] == ' ' || localTime[i] == ':')
		{
			name[i] = '-';
		}
		else
			name[i] = localTime[i];
	}

	return name;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

bool uploadData::postRequest()
{
    CURL *curl;
	CURLcode res;
	std::string readBuffer;
    bool post = false;

    std::string url = "\"signed_url\": ";
    std::string postreq = "{\"file_name\":\"" + keyname + "\"}";

    curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, postURL.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postreq.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res = curl_easy_perform(curl);
        if (res != CURLE_OK)
		{
			qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
			/*
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			*/
		}
		else
		{
			qDebug() << "Post Request Complete!"<< '\n';
			//std::cout << "Post Request Complete!" << std::endl;
			post = true;
		}
		curl_easy_cleanup(curl);
	}

	if (post && url.compare(readBuffer.substr(1, url.size())) == 0)
	{
		putURL = readBuffer.substr(url.size() + 2, readBuffer.size() - url.size() - 4);
		//std::cout << putURL << std::endl;
	}
	else
	{
		qDebug() << "Bad Post Request!" << '\n';
		//std::cout << "Bad Post Request!" << std::endl;
	}

    return post;
}

bool uploadData::putRequest(std::string filename)
{
	CURL *curl;
	CURLcode res;
	bool put = false;

	FILE *file;
	file = fopen(filename.c_str(), "rb");
	struct stat file_info;
	stat(filename.c_str(), &file_info);

	if (file)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(curl, CURLOPT_PUT, 1L);
			curl_easy_setopt(curl, CURLOPT_URL, putURL.c_str());
			curl_easy_setopt(curl, CURLOPT_READDATA, file);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

			res = curl_easy_perform(curl);

			if (res == CURLE_OK)
			{
				put = true;
				qDebug() << "Upload Complete!" << '\n';
				//std::cout << "Upload Completed." << std::endl;
			}
			else
			{
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
			}

			curl_easy_cleanup(curl);
		}
	}
	fclose(file);

	return put;
}

uploadData::uploadData(std::string filename)
{
    keyname = getKeyname();
    bool post = postRequest();

	if (post) bool put = putRequest(filename);
}

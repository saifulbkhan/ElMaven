#ifndef UPLOADDATA_H
#define UPLOADDATA_H

#include <iostream>
#include <string>
#include <sys/stat.h>
//#include <fcntl.h>
#include <curl/curl.h>
#include <QDebug>

class uploadData {
public:
    uploadData(std::string filename);
    std::string getKeyname();
protected:
private:
    std::string keyname;
    std::string filename;
    std::string putURL;
    const std::string postURL = "https://9yzp306xy4.execute-api.us-west-2.amazonaws.com/test";

    bool postRequest();
    bool putRequest(std::string filename);
};

#endif

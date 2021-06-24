#ifndef _LOG_HPP_
#define _LOG_HPP_ 


#include<iostream>
#include<string>
#include<sys/time.h>
//日志级别
#define Normal 1
#define Warning 2
#define Fatal 3
//[Normal][12345654321][text to audio error!][jarvis.hpp][13]


long GetTimeStamp()
{
    struct timeval time;
    if(gettimeofday(&time, nullptr) == 0)
    {
        return time.tv_sec;
    }
    return 0;
}

void Log(std::string level, std::string msg, std::string file_name, int num)
{
    std::cerr << " [ " << level << " ][ " << GetTimeStamp() << " ][ " << msg << " ][ " << " ][ " << file_name << " ][ " << num <<  " [ " << std::endl;
}


#define LOG(level, message) Log(#level, message, __FILE__, __LINE__);
#endif 



#include "log.h"

#include <unistd.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <cstdarg>

#include <iostream>

SingletonLog *SingletonLog::_instance = new SingletonLog;
SingletonLog::garbo SingletonLog::_garbo;

SingletonLog *SingletonLog::get_instance()
{
    return _instance;
}
SingletonLog::SingletonLog():_msg_queue(1024), _msg_pool(sizeof(char))
{}

SingletonLog::~SingletonLog()
{}

bool SingletonLog::open_log(const std::string& log_path, LOG_LEVEL level, uint32_t rotate_size)
{
    _base_file_path = log_path;
    _log_level = level;
    _rotate_size = rotate_size*1024*1024;

    _fp = fopen(_base_file_path.c_str(), "a");
    if (_fp == NULL){
        return false;
    }

    struct stat st;
    int ret = fstat(fileno(_fp), &st);
    if (ret == -1){
        std::cerr<<"fstat log file error:"<<_base_file_path.c_str()<<std::endl;
        return false;
    }
    else{
        _curr_size = st.st_size;
    }
    is_done = false;
    return true;
}
void SingletonLog::close_log()
{
    is_done = true;
    fclose(_fp);
}

void SingletonLog::write_log_to_queue(LOG_LEVEL log_level, const char *fmt, ...)
{
    if (log_level > _log_level)
      return ;

    time_t time;
    struct timeval tv;
    struct tm *tm;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    tm = localtime(&time);

    char buffer[MAX_MSG_LENGTH];
    int len = sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d.%06d] %s:", tm->tm_year + 1900, tm->tm_mon + 1, 
                tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec), SingletonLog::get_level_name(log_level));
    if (len < 0)
      return ;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer+len, MAX_MSG_LENGTH-len, fmt, ap);
    va_end(ap);

    char *end = buffer + strlen(buffer);
    *end ++ = '\n';
    *end = '\0';
    //char *msg = (char *)_msg_pool.ordered_malloc(strlen(buffer) + 1);
    char *msg = new char[(strlen(buffer) + 1)];
    strcpy(msg, buffer);
    while(!_msg_queue.push(msg))
    {
        printf("again\n");
    }
}

bool SingletonLog::write_log_to_file()
{
    while (!is_done)
    {
        char *msg;
        while (_msg_queue.pop(msg)){
            int len = strlen(msg);
            fwrite(msg, len, 1, _fp);
            //_msg_pool.ordered_free((void *)msg, len+1);
            delete msg;
            fflush(_fp);
            _curr_size += len;
            if (_rotate_size > 0 && _curr_size > _rotate_size){
                rotate();
            }
        }
    }
    return true;
}

void SingletonLog::rotate()
{
    fclose(_fp);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t time = tv.tv_sec;
    struct tm *tm = localtime(&time);

    char time_str[32];
    memset(time_str, 0, 32);
    sprintf(time_str, "_%4d%02d%02d%02d%02d%02d%06d", tm->tm_year + 1990, tm->tm_mon + 1, 
                tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec);

    std::string new_file_name = _base_file_path + time_str;
    _fp = fopen(new_file_name.c_str(), "a");
    if (_fp == NULL){
        return ;
    }
    _curr_size = 0;
}

inline const char *SingletonLog::get_level_name(LOG_LEVEL level)
{
    switch (level){
        case SingletonLog::FATAL:
            return "[FATAL]";
        case SingletonLog::ERROR:
            return "[ERROR]";
        case SingletonLog::WARN:
            return "[WARN ]";
        case SingletonLog::INFO:
            return "[INFO ]";
        case SingletonLog::DEBUG:
            return "[DEBUG]";
        case SingletonLog::TRACE:
            return "[TRACE]";
        default:
                return "";
    }
}

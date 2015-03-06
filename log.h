/*=============================================================================
#      Filename : log.h
#   Description : 日志文件类
#        Author : chenqingming chenqingming0710@163.com
#        create : 2015-03-05 15:29
# Last modified : 2015-03-05 15:29
=============================================================================*/

#include <string>

#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool.hpp>
#include <boost/atomic.hpp>

class SingletonLog
{
    public:
#define MAX_MSG_LENGTH 1024  //单个消息最大长度
        enum LOG_LEVEL 
        {
            FATAL = 0,
            ERROR = 1,
            WARN = 2,
            INFO = 3,
            DEBUG = 4,
            TRACE = 5
        };

    public:
        static SingletonLog *get_instance();

        //初始化日志，试用期前必须调用且只调用一次该函数。rotate_size 单位:M
        bool open_log(const std::string& log_path, LOG_LEVEL level, uint32_t rotate_size = 0);
        //关闭日志
        void close_log();

        //将日志消息写到缓冲队列，可能阻塞。
        void write_log_to_queue(LOG_LEVEL log_level, const char *fmt, ...);

        //取出缓冲队列中的消息，写到日志文件。
        bool write_log_to_file();

    private:
        //关闭就文件，打开新的文件。
        void rotate();
        inline static const char *get_level_name(LOG_LEVEL level);

    private:
        SingletonLog();
        ~SingletonLog();
        SingletonLog(const SingletonLog&);
        SingletonLog& operator= (const SingletonLog&);

    private:
        static SingletonLog *_instance;

        FILE *_fp;
        LOG_LEVEL _log_level;
        uint64_t _curr_size;
        uint64_t _rotate_size;
        std::string _base_file_path;
        boost::lockfree::queue<char *> _msg_queue;
        boost::pool<> _msg_pool;
        boost::atomic<bool> is_done;

        class garbo
        {
            public:
                ~garbo()
                {
                    if (SingletonLog::_instance != NULL){
                        delete SingletonLog::_instance;
                        SingletonLog::_instance = NULL;
                    }
                }
        };
        static garbo _garbo;
};

#define log_debug(fmt, args...) \
    SingletonLog::get_instance()->write_log_to_queue(SingletonLog::DEBUG, fmt, ##args)

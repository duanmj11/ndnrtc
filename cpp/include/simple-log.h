//
//  simple-log.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev 
//  Created: 8/8/13
//

#ifndef __ndnrtc__simple__
#define __ndnrtc__simple__

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include "params.h"

#if !defined(NDN_LOGGING)
#undef NDN_TRACE
#undef NDN_INFO
#undef NDN_WARN
#undef NDN_ERROR
#undef NDN_DEBUG
#endif

#ifndef BASE_FILE_NAME
#define BASE_FILE_NAME (__FILE__)
#endif

// if defined detailed logging - print whole signature of the function.
#define __NDN_FNAME__ __PRETTY_FUNCTION__

// following macros are used for NdnRtcObject logging
// each macro checks, whether a logger, associated with object has been
// initialized and use it instead of global logger
#if defined (NDN_TRACE)

#define LogTrace(fname, ...) ndnlog::new_api::Logger::log(fname, (ndnlog::NdnLogType)ndnlog::NdnLoggerLevelTrace, BASE_FILE_NAME, __LINE__, ##__VA_ARGS__)
#define LogTraceC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelTrace, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())

#else

#define LogTrace(fname, ...) ndnlog::new_api::NilLogger::get()
#define LogTraceC ndnlog::new_api::NilLogger::get()

#endif

#if defined (NDN_DEBUG)

#define LogDebug(fname, ...) ndnlog::new_api::Logger::log(fname, ndnlog::NdnLoggerLevelDebug, __FILE__, __LINE__, ##__VA_ARGS__)
#define LogDebugC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelDebug, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())
#else

#define LogDebug(fmt, ...) ndnlog::new_api::NilLogger::get()
#define LogDebugC ndnlog::new_api::NilLogger::get()

#endif

#if defined (NDN_INFO)

#define LogInfo(fname, ...) ndnlog::new_api::Logger::log(fname, ndnlog::NdnLoggerLevelInfo, __FILE__, __LINE__, ##__VA_ARGS__)
#define LogInfoC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelInfo, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())

#else

#define LogInfo(fname, ...) ndnlog::new_api::NilLogger::get()
#define LogInfoC ndnlog::new_api::NilLogger::get()

#endif

#if defined (NDN_WARN)

#define LogWarn(fname, ...) ndnlog::new_api::Logger::log(fname, ndnlog::NdnLoggerLevelWarning   , __FILE__, __LINE__, ##__VA_ARGS__)
#define LogWarnC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelWarning, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())

#else

#define LogWarn(fname, ...) ndnlog::new_api::NilLogger::get()
#define LogWarnC ndnlog::new_api::NilLogger::get()

#endif

#if defined (NDN_ERROR)

#define LogError(fname, ...) ndnlog::new_api::Logger::log(fname, ndnlog::NdnLoggerLevelError, __FILE__, __LINE__, ##__VA_ARGS__)
#define LogErrorC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelError, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())

#else

#define LogError(fname, ...) ndnlog::new_api::NilLogger::get()
#define LogErrorC ndnlog::new_api::NilLogger::get()

#endif

#define LogStat(fname, ...) ndnlog::new_api::Logger::log(fname, (NdnLogType)NdnLoggerLevelStat, BASE_FILE_NAME, __LINE__, ##__VA_ARGS__)
#define LogStatC ((this->logger_) ? this->logger_->log((ndnlog::NdnLogType)ndnlog::NdnLoggerLevelStat, this, BASE_FILE_NAME, __LINE__) : ndnlog::new_api::NilLogger::get())

#define STAT_DIV "\t"

namespace ndnlog {
    typedef NdnLoggerLevel NdnLogType;

    /**
     * Thread-safe logger class to stdout or file
     * DEPRECATED use logger from new_api instead
     */
    namespace new_api
    {
        class ILoggingObject;
        class NilLogger;
        
        /**
         * Logger object. Performs thread-safe logging into a file or standard 
         * output.
         */
        class Logger
        {
        private:
            using endl_type = std::ostream&(std::ostream&);
            
        public:
            // interval at which logger is flushing data on disk (if file logging
            // was chosen)
            static unsigned int FlushIntervalMs;
            
            /**
             * Creates an instance of logger with specified logging level and 
             * file name. By default, creates logger wich logs everything into 
             * the console.
             * @param logLevel Minimal logging level for current instance. Eny 
             * log wich has level less than specified will br ignored.
             * @param logFileFmt Format for the logging file name. If NULL - 
             * logging is performed into standard output.
             */
            Logger(const NdnLoggerDetailLevel& logLevel = NdnLoggerDetailLevelAll,
                   const std::string& logFile = "");
            /**
             * Releases current instance and all associated resources
             */
            ~Logger();
            
            /**
             * Starts new logging entry with the header. Logging entry
             * can be completed with the "endl" function or by another new call
             * to this method (i.e. calling "<<" will append to the existing
             * logging entry initiated by the last "log" call).
             * @param logType Current logging type
             * @param loggingInstance Pointer to the instance of ILoggingObject
             * interface which performs logging
             * @param locationFile Name of the source file from which logging is
             * performed
             * @param locationLine Line number from the source file from which
             * logging is performed
             * @return A refernce to the current Logger instance
             */
            virtual Logger&
            log(const NdnLogType& logType,
                const ILoggingObject* loggingInstance = 0,
                const std::string& locationFile = "",
                const int& locationLine = -1);
            
            /**
             * Stream operator << implementation
             * Any consequent call appends data to the previously created log 
             * entry. Specific log entry can be created using "log" call,
             * otherwise - if no previous log call was made, calling this 
             * operator creates a new deafult log entry with INFO log level and 
             * with no logging instance (set to nullptr) which results in poor 
             * undetailed logging. However can be usfeul for system-wide logging.
             */
            template<typename T>
            Logger& operator<< (const T& data)
            {
                if (isWritingLogEntry_ &&
                    currentEntryLogType_ >= (NdnLogType)logLevel_)
                {
                    currentLogRecord_ << data;
                }
                
                return *this;
            }
            
            /**
             * Overload for std::endl explicitly
             * Closes current logging entry
             */
            virtual
            Logger& operator<< (endl_type endl)
            {
                if (isWritingLogEntry_ &&
                    currentEntryLogType_ >= (NdnLogType)logLevel_)
                {
                    isWritingLogEntry_ = false;
                    currentEntryLogType_ = NdnLoggerLevelTrace;
                    
                    currentLogRecord_ << endl;
                    finalizeLogRecord();
                    unlockStream();
                }
                
                return *this;
            }
            
            void
            setLogLevel(const NdnLoggerDetailLevel& logLevel)
            { logLevel_ = logLevel; }
            
            NdnLoggerDetailLevel
            getLogLevel()
            { return logLevel_; }
            
            static void
            initAsyncLogging();
            
            static void
            releaseAsyncLogging();
            
            static Logger&
            getLogger(const std::string &logFile);
            
            static void
            destroyLogger(const std::string &logFile);
            
            static Logger& log(const std::string &logFile,
                               const NdnLogType& logType,
                               const std::string& locationFile = "",
                               const int& locationLine = -1,
                               const ILoggingObject* loggingInstance = 0)
            {
                return getLogger(logFile).log(logType, loggingInstance,
                                       locationFile, locationLine);
            }
            
            
            static void
            initializeSharedInstance(const NdnLoggerDetailLevel& logLevel,
                                     const std::string& logFile)
            {
                if (sharedInstance_)
                    delete sharedInstance_;
                
                sharedInstance_ = new Logger(logLevel, logFile);
            }
            
            static Logger&
            sharedInstance()
            { return *sharedInstance_; }
            
            static std::string
            stringify(NdnLoggerLevel lvl);
            
            void
            flush();
            
            std::string
            getFileName() const
            { return logFile_; }
            
        private:
            NdnLoggerDetailLevel logLevel_;
            std::string logFile_;
            std::ostream* outStream_;
            int64_t lastFlushTimestampMs_;
            
            bool isWritingLogEntry_, isStdOutActive_;
            NdnLogType currentEntryLogType_;
            pthread_t current_;
            
            boost::recursive_mutex logMutex_;
            
            static boost::recursive_mutex stdOutMutex_;
            static std::map<std::string, Logger*> loggers_;
            static Logger* sharedInstance_;
            
            boost::atomic<bool> isProcessing_;
            std::stringstream currentLogRecord_;
            typedef boost::lockfree::spsc_queue<std::string, boost::lockfree::capacity<1024>> LogRecordQueue;
            LogRecordQueue recordsQueue_;
            
            void
            processLogRecords();
            
            void
            startLogRecord();
            
            void
            finalizeLogRecord();
            
            std::ostream&
            getOutStream() { return *outStream_; }
            
            std::ofstream&
            getOutFileStream()
            {
                std::ofstream *fstream = reinterpret_cast<std::ofstream*>(outStream_);
                return *fstream;
            }
            
            int64_t
            getMillisecondTimestamp();
            
            int i = 0;
            
            void
            lockStreamExclusively()
            {
                if (isStdOutActive_)
                    stdOutMutex_.lock();
                else
                    logMutex_.lock();
            }
            
            void unlockStream()
            {
                if (isStdOutActive_)
                    stdOutMutex_.unlock();
                else
                    logMutex_.unlock();
            }
        };
        
        /**
         * Interface for logging objects - instances, that can do logging 
         * should implement this interface
         */
        class ILoggingObject
        {
        public:
            /**
             * Instance short description
             * @return Description of the current instance - should contain 
             * short string representing just enough information about the 
             * instance, which preforms logging
             */
            virtual std::string
            getDescription() const
            { return description_; }
            
            virtual void
            setDescription(const std::string& desc)
            { description_ = desc; }
            
            virtual bool
            isLoggingEnabled() const
            { return true; }
            
            ILoggingObject():logger_(0), isLoggerCreated_(false){}
            ILoggingObject(const NdnLoggerDetailLevel& logLevel,
                           const std::string& logFile);
            
            virtual ~ILoggingObject()
            {
                if (logger_)
                    logger_->flush();
                if (isLoggerCreated_)
                    delete logger_;
                logger_ = NULL;
            }
            
            virtual void
            setLogger(Logger* logger);
            
            virtual Logger*
            getLogger() const
            { return logger_; }
            
        protected:
            bool isLoggerCreated_ = false;
            Logger* logger_;
            std::string description_ = "<no description>";
        };
        
        class NilLogger : public Logger {
        private:
            using endl_type = std::ostream&(std::ostream&);
            
        public:
            template<typename T>
            NilLogger& operator<<(T const&)
            { 
                return *this;
            }
            
            NilLogger& operator<< (endl_type endl)
            {
                return *this;
            }
            
            static NilLogger& get() { return nilLogger_; }
            
        private:
            static NilLogger nilLogger_;
        };

    }
}

#endif /* defined(__ndnrtc__simple__) */

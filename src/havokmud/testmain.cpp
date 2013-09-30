#include <boost/thread.hpp>
#include "thread/HavokThread.hpp"
#include "thread/ConnectionThread.hpp"
#include "thread/LoggingThread.hpp"

boost::thread::id g_mainThreadId;
bool g_debug = true;
havokmud::thread::LoggingThread *g_loggingThread;

namespace havokmud {
    namespace thread {
        class MainThread : public HavokThread
        {
        public:
            MainThread();
            ~MainThread()  {};

            virtual void start();
            void handle_stop()  {};
        };

        MainThread::MainThread() : HavokThread("Main")
        {
            pro_initialize<MainThread>();
        }

        void MainThread::start()
        {
            g_mainThreadId = boost::this_thread::get_id();

            g_loggingThread = new havokmud::thread::LoggingThread();

            havokmud::thread::ConnectionThread connectionThread(1234, 1000);

            boost::mutex locker;
            locker.lock();
            // Lock hard.
            locker.lock();
        }
    }
}


int main(int argc, char **argv)
{
    havokmud::thread::MainThread *mainThread = new havokmud::thread::MainThread();

    boost::mutex locker;
    locker.lock();
    // Lock hard.
    locker.lock();

    return 0;
}

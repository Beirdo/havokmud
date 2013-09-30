#include <boost/thread.hpp>
#include "thread/ConnectionThread.hpp"
#include "thread/LoggingThread.hpp"

boost::thread::id g_mainThreadId;
bool g_debug = true;
havokmud::thread::LoggingThread *g_loggingThread;

int main(int argc, char **argv)
{
    g_mainThreadId = boost::this_thread::get_id();

    g_loggingThread = new havokmud::thread::LoggingThread();

    havokmud::thread::ConnectionThread connectionThread(1234, 1000);

    boost::mutex locker;
    locker.lock();
    // Lock hard.
    locker.lock();

    return 0;
}

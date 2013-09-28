#include <boost/thread.hpp>
#include "thread/ConnectionThread.hpp"

int main(int argc, char **argv)
{
    havokmud::thread::ConnectionThread connectionThread(1234, 1000);

    boost::mutex locker;
    locker.lock();
    // Lock hard.
    locker.lock();

    return 0;
}

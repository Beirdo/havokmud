#include <boost/thread.hpp>
#include "thread/HavokThread.hpp"
#include "thread/ConnectionThread.hpp"
#include "thread/LoggingThread.hpp"
#include "thread/ResolveThread.hpp"
#include "thread/LoginThread.hpp"
#include "thread/PlayingThread.hpp"
#include "thread/SmtpThread.hpp"
#include "thread/SmtpThread.hpp"
#include "thread/DatabaseThread.hpp"
#include "thread/DatabaseUpgradeThread.hpp"
#include "corefunc/CoreDatabaseSchema.hpp"

boost::thread::id g_mainThreadId;
bool g_debug = true;
havokmud::thread::LoggingThread *g_loggingThread;
havokmud::thread::ResolveThread *g_resolveThread;
havokmud::thread::LoginThread   *g_loginThread;
havokmud::thread::PlayingThread *g_playingThread;
havokmud::thread::PlayingThread *g_immortalPlayingThread;
havokmud::thread::SmtpThread    *g_smtpThread;
havokmud::thread::DatabaseThread *g_databaseThread;

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
            m_startupMutex.unlock();

            g_loggingThread = new havokmud::thread::LoggingThread();
            g_databaseThread = new havokmud::thread::DatabaseThread();

            havokmud::thread::DatabaseUpgradeThread coreDbUpgrade(havokmud::corefunc::coreDatabaseSchema);

            g_resolveThread = new havokmud::thread::ResolveThread();
            g_loginThread = new havokmud::thread::LoginThread();
            g_playingThread = new havokmud::thread::PlayingThread(false);
            g_immortalPlayingThread = new havokmud::thread::PlayingThread(true);
            g_smtpThread = new havokmud::thread::SmtpThread();

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

#include <QApplication>
#include <QtWebEngine>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QLockFile>

#include <vector>

#include <httplib.h>

#include "paths.hpp"
#include "browserwindow.hpp"

httplib::Server server;
std::unique_ptr<QLockFile> instance_lock;

#ifdef Q_OS_UNIX
#include <csignal>

void sig_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        if (instance_lock)
        {
            instance_lock->unlock();
        }

        std::exit(0);
    }
}
#endif

void show_error(const QString &message)
{
    QMessageBox::critical(nullptr,
        QObject::tr("Error"),
        message);
}

bool is_already_running(const QString &instance_name = "default")
{
    const auto tmp = QDir::tempPath();
    instance_lock = std::make_unique<QLockFile>(QString("%1/%2").arg(tmp, "qelement-instance-"+instance_name+".lock"));

    if (!instance_lock->tryLock(100))
    {
        // locking failed, application already running
        return true;
    }

    // new instance created
    return false;
}

int main(int argc, char **argv)
{
#ifdef Q_OS_UNIX
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);
#endif

#ifdef DEBUG_BUILD
    QString instance_name = "debug";
#else
    QString instance_name = "default";
#endif

    // check if application is already running and acquire a single instance lock
    // qtwebengine corrupts its own storage on multiple instances of the process
    // also the internal webserver can't bind on the same port multiple times
    if (is_already_running(instance_name))
    {
        return 0;
    }

    // append command line arguments
    std::vector<char*> args{argv, argc + argv};
    args.push_back(const_cast<char*>("--disable-logging")); // disable 3rd party log messages from QtWebEngine
    args.push_back(nullptr);
    int newArgc = int(args.size()) - 1;

    // initialize application with modified arguments
    QtWebEngine::initialize();
    QApplication a(newArgc, args.data());
    a.setApplicationName("QElement");
    a.setApplicationVersion("1.0");
    a.setWindowIcon(QIcon(":/element.ico"));

    // create and validate data path
    if (paths::webengine_profile_path().isEmpty())
    {
        show_error(QObject::tr("Unable to access directory: %1").arg(
            paths::webengine_profile_path(false)));
        return 1;
    }

    // FIXME: try to use a custom url scheme handler just like the official Electron app
    //        instead of starting a web server

    // start the static file server to provide the element webapp via http
    QtConcurrent::run([]{
        server.set_mount_point("/", "/opt/Element/resources/webapp");
        server.listen("localhost", 63535);
    });

    // stop server when application quits
    QObject::connect(&a, &QApplication::aboutToQuit, &a, []{
        server.stop();
    });

    // load the browser window
    BrowserWindow webview;

    // show browser window
    webview.show();

    // enter qt event loop
    const auto res = a.exec();

    // remove single instance lock
    instance_lock->unlock();

    return res;
}

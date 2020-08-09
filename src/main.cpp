#include <QApplication>
#include <QTranslator>
#include <QCommandLineParser>
#include <QtWebEngine>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QLockFile>

#include <vector>
#include <string_view>

#include "paths.hpp"
#include "configmanager.hpp"
#include "browserwindow.hpp"
#include "elementurlscheme.hpp"

constexpr const std::string_view appname{"QElement"};
constexpr const std::string_view appversion{"1.3"};
const Paths *paths = nullptr;
ConfigManager *config = nullptr;

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

// force QDebug to print to console even when no tty is attached to avoid
// cluttering the systemd journal on systemd-based distros
void qdebug_force_console(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtDebugMsg)
    {
        std::fprintf(stderr, "%s\n", msg.toStdString().c_str());
    }
    else
    {
        std::fprintf(stdout, "%s\n", msg.toStdString().c_str());
    }
}

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
    qInstallMessageHandler(qdebug_force_console);

#ifdef Q_OS_UNIX
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);
#endif

    QStringList arguments;
    for (auto i = 0; i < argc; ++i)
    {
        arguments << QString::fromUtf8(argv[i]);
    }

    QCommandLineParser parser;
    QList<QCommandLineOption> options{
        QCommandLineOption("help", QObject::tr("Show this help")),
        QCommandLineOption("minimized", QObject::tr("Start minimized to tray")),
        QCommandLineOption("profile", QObject::tr("Profile to use"), "profile",
            #ifdef DEBUG_BUILD
                "debug"
            #else
                "default"
            #endif
            ),
        QCommandLineOption("webapp-root", QObject::tr("Use alternative webapp root"), "webapp-root"),
    };
    parser.addOptions(options);
    parser.process(arguments);

    if (parser.isSet("help"))
    {
        std::printf("%s %s\n", appname.data(), appversion.data());

        for (auto&& option : options)
        {
            std::printf("   --%s%s %s\t\t%s\n",
                option.names().at(0).toUtf8().constData(),
                ([&]{
                    if (!option.valueName().isEmpty())
                    {
                        return std::string{"=["} + option.valueName().toStdString() + std::string{"]"};
                    }
                    return std::string{};
                })().c_str(),
                ([&]{
                    if (option.valueName().isEmpty())
                    {
                        return "\t";
                    }
                    else
                    {
                        return "";
                    }
                })(),
                option.description().toUtf8().constData());
        }

        return 0;
    }

    // get profile to use
    QString instance_name = parser.value("profile");
    std::printf("using profile: %s\n", instance_name.toUtf8().constData());

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

    // setup element:// url scheme
    QWebEngineUrlScheme scheme(ElementUrlScheme::schemeName());
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    scheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(scheme);

    // initialize application with modified arguments
    QtWebEngine::initialize();
    QApplication a(newArgc, args.data());
    a.setApplicationName(appname.data());
    a.setApplicationVersion(appversion.data());
    a.setWindowIcon(QIcon(":/element.ico"));

    // load embedded translations for current locale using QRC language and alias magic :)
    // falls back to embedded English strings if no translation was found
    auto translator = std::make_unique<QTranslator>();
    if (translator->load(QLocale(), ":/i18n/lang.qm"))
    {
        a.installTranslator(translator.get());
    }
    else
    {
        translator.reset();
    }

    // initialize application paths
    paths = Paths::defaultInstance();

    // create and validate data path
    if (paths->webEngineProfilePath(instance_name).isEmpty())
    {
        show_error(QObject::tr("Unable to access directory: %1").arg(
            paths->webEngineProfilePath(instance_name, false)));
        return 1;
    }

    // initialize config manager
    auto configManager = std::make_unique<ConfigManager>(paths->webEngineProfilePath(instance_name));
    config = configManager.get();

    // check if an alternative webroot was requested
    auto webappRoot = config->webroot();
    const auto alternativeWebroot = parser.value("webapp-root");
    if (!alternativeWebroot.isEmpty())
    {
        qDebug() << "using alternative webroot:" << alternativeWebroot;
        webappRoot = alternativeWebroot;
    }

    // register element:// url scheme
    auto elementUrlHandler = std::make_unique<ElementUrlScheme>(webappRoot);
    QWebEngineProfile::defaultProfile()->installUrlSchemeHandler(ElementUrlScheme::schemeName(), elementUrlHandler.get());

    QObject::connect(config, &ConfigManager::configUpdated, [&](const ConfigManager::Key &key){
        if (key == ConfigManager::Key::Webroot)
        {
            qDebug() << "webroot updated to:" << config->webroot();
            elementUrlHandler->changeRoot(config->webroot());
        }
    });

    // load the browser window
    BrowserWindow webview(instance_name);

    // show browser window
    if (!parser.isSet("minimized"))
    {
        webview.show();
    }

    // enter qt event loop
    const auto res = a.exec();

    // remove single instance lock
    instance_lock->unlock();

    return res;
}

// THIS PROJECT IS THE CREATION OF AVERAGETITANFROMDESTINY

/*
AI DISCLOSURE: The vast majority of code was written by humans (me)

AI was used to research classes, types, and help me understand said objects across QML, C++ and CMake (CMakeLists) .
AI was also used to help translate the QMake file this project used to use.

Though, as of now, AI is not being used for any purposes whatsoever.
*/

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngineQuick>
#include <QFontDatabase>
#include <QFont>
#include <QSurfaceFormat>
#include <QQuickWindow>
#include <QWebEngineProfile>
#include <QMetaObject>
#include "utils.hpp"
#include <QWidget>

int main(int argc, char *argv[])
{
    // Folders "translations" and "qmltooling" are not strictly necessary in deployment.
    // The "\qml\QtQuick\Controls" folder contains files that may be safely removed.
    // consider enabling devtools (via remote debugging) on port 9222

    // qputenv("QSG_RENDER_LOOP", "basic");
    qputenv("QV4_JIT_CALL_THRESHOLD","1"); //Slower first-run, otherwise, helps ensure all functions are JIT-compiled.
    qputenv("QV4_CRASH_ON_STACKOVERFLOW", "1");
    qputenv("QV4_JS_MAX_STACK_SIZE","4000"); //Default stack size is 4 MB (4,000,000 Bytes)
    qputenv("QV4_MAX_CALL_DEPTH","1"); // Recursion? nuh uh.

    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);
    qputenv("QSG_RHI_DISABLE_SHADER_DISK_CACHE", "1"); //Really? We had to do the Attribute AND environment variable?

    QSurfaceFormat::defaultFormat().setSamples(-1);
    QSurfaceFormat::defaultFormat().setSwapInterval(0);

    QQuickWindow::setTextRenderType(QQuickWindow::TextRenderType::NativeTextRendering);

    // QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents);
    QCoreApplication::setAttribute(Qt::AA_CompressTabletEvents);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    if( Q_FileHelper::fileExists("flags.txt") ){
        // qDebug() << qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");
        QByteArray* currentFlags = new QByteArray();
        currentFlags ->prepend(Q_FileHelper::readLines_String("flags.txt", -1, false, " --", true).trimmed().toLocal8Bit());
        // prepends file-based flags instead of overriding!
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", *currentFlags);
        // You can now tweak chromium flags via a text file!!!!
        delete currentFlags;
    }


    QtWebEngineQuick::initialize();
    QApplication app(argc, argv);
    //Creating cache dirs...
    Q_FileHelper::mkFolder("webcache");
    Q_FileHelper::mkFolder("webstorage");
    Q_FileHelper::mkFolder("qmlcache");
    QByteArray* path = new QByteArray(Q_FileHelper::getLocalResource("qmlcache").toString().toLocal8Bit());
    qputenv( "QML_DISK_CACHE_PATH", *path ); // Will need this sooner or later...
    qDebug()<< *path;
    delete path;

    bool wipeHistoryOnClose = true; // will expose an option to un-set this

    //WEBENGINEPROFILE SETTINGS:
    QWebEngineProfile::defaultProfile()->setCachePath( Q_FileHelper::getLocalResource("webcache", true).toString() );
    QWebEngineProfile::defaultProfile()->setPersistentStoragePath( Q_FileHelper::getLocalResource("webstorage", true).toString() );
    QWebEngineProfile::defaultProfile()->setHttpCacheType(QWebEngineProfile::HttpCacheType::DiskHttpCache);
    QWebEngineProfile::defaultProfile() ->setPersistentCookiesPolicy(QWebEngineProfile::PersistentCookiesPolicy::AllowPersistentCookies);
    QWebEngineProfile::defaultProfile() ->setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::AskEveryTime);

    int fontid = QFontDatabase::addApplicationFont(Q_FileHelper::getLocalResource("font/FONT_MAIN.ttf").toString() ); // consider custom fonts?
    if(fontid != -1){
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontid).constFirst();
        if( fontFamily.isEmpty() == false ){
            QFont globalFont;
            globalFont.setFamily( fontFamily );
            globalFont.setKerning( 0 );
            globalFont.setPixelSize( 15 );
            globalFont.setHintingPreference( QFont::HintingPreference::PreferFullHinting  );
            app.setFont( globalFont );
        }
    }

    qDebug() << qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");

    QQmlApplicationEngine engine;
    engine.loadFromModule( "App.Core", "Main" );
    if( engine.rootObjects().isEmpty() )return 1;
\
    QObject::connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, [&](){
        QWidget* widg = new QWidget();
        widg->setGeometry(QRect(200, 200, 200, 200));
        widg->setWindowFlags(Qt::WindowType::Popup);
        widg->show();
    });

    QQuickWindow* wnd = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
    wnd->setFlags(Qt::Window | Qt::FramelessWindowHint);
    wnd->contentItem()->setSmooth(false);
    wnd->contentItem()->setAntialiasing(false);
    wnd->setColor(QColor(80, 80, 80));
    wnd->setVisible(true);
    int ret = 0;
    ret = QApplication::exec( );
    // after exec:
    if(wipeHistoryOnClose == true)QWebEngineProfile::defaultProfile() -> clearAllVisitedLinks();

    return ret;
}

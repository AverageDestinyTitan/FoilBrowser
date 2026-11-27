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
#include "Windows.h"
#include "cstdlib"
#include <downloadWindow.hpp>

// Folders "translations" and "qmltooling" are not strictly necessary in deployment.
// The "\qml\QtQuick\Controls" folder contains files that may be safely removed.


wchar_t* getRawRootPath(int& lastChar){
    wchar_t* pathArr= new wchar_t[MAX_PATH];
    int idx = 0;
    GetModuleFileNameW(NULL, pathArr, MAX_PATH);
    for(int i = 259; i >= 3; --i){
        if(pathArr[i] != L'\\'){continue;}
        else{idx = i;break;}
    }
    pathArr [idx + 1]= L'\0';
    lastChar = idx+1;
    qDebug()<<"IDX: "<<idx;
    return pathArr;
}
int main(int argc, char *argv[])
{
    int lastIDX = 0;
    wchar_t* dllpath = getRawRootPath(lastIDX);
    wchar_t* lib_suffix = new wchar_t[4]{ L'L', L'I' , L'B', L'\0'};
    memcpy(dllpath + lastIDX, lib_suffix, sizeof(dllpath));
    AddDllDirectory(dllpath);
    delete[] lib_suffix;
    lib_suffix = nullptr;

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
    QCoreApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents);
    QCoreApplication::setAttribute(Qt::AA_CompressTabletEvents);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    if( QFileInfo::exists("flags.txt") ){
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
    qputenv( "QML_DISK_CACHE_PATH", QByteArray(Q_FileHelper::getLocalResource("qmlcache").toString().toLocal8Bit())); // Will need this sooner or later...

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
    qDebug()<< QString(dllpath);


    QQmlApplicationEngine engine;
    engine.loadFromModule( "App.Core", "Main" );
    if( engine.rootObjects().isEmpty() )return 1;

    QQuickWindow* wnd = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
    wnd->setFlags(Qt::Window | Qt::FramelessWindowHint);
    wnd->contentItem()->setSmooth(false);
    wnd->contentItem()->setAntialiasing(false);
    wnd->setColor(QColor(80, 80, 80));
    Q_DownloadWindow* widg =  new Q_DownloadWindow(nullptr);
    QObject::connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, [&](QWebEngineDownloadRequest* req){
        widg->showWithReq(wnd, req);
    });
    wnd->setVisible(true);
    int ret = QApplication::exec( );
    // after exec:
    if(wipeHistoryOnClose == true)QWebEngineProfile::defaultProfile() -> clearAllVisitedLinks();

    return ret;
}

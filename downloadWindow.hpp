#include <QWindow>
#include <QStaticText>
#include <QTextEdit>
#include <qwebenginedownloadrequest.h>
#include <QPointer>

class Q_DownloadWindow : public QWidget{
    Q_OBJECT

public:

    Q_DownloadWindow( QWindow* wnd = nullptr, QWebEngineDownloadRequest* request = nullptr, const int width  = 200, const int height =200){

        if(request == nullptr){delete this; return;}
        this -> setWindowFlags(Qt::WindowType::Popup);
        this ->setAttribute(Qt::WA_DeleteOnClose);
        this->setGeometry(wnd == nullptr? QRect(100, 100, 200, 200) : wnd->geometry());

        m_requestPointer=request;

    }

private:
    QStaticText m_filenamelabel = QStaticText("File name: ");
    QTextEdit m_filenamebox = QTextEdit();


    QPointer<QWebEngineDownloadRequest>m_requestPointer=nullptr;


};

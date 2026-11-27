#include <QWindow>
#include <QStaticText>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <qevent.h>
#include <qpainter.h>
#include <qwebenginedownloadrequest.h>
#include <QPointer>

class Q_DownloadWindow : public QWidget{
    Q_OBJECT

public:

    Q_DownloadWindow(QWidget* parent = nullptr){
        this -> setWindowFlags(Qt::WindowStaysOnTopHint| Qt::Tool );
        m_rejectButton.setFlat(true);
        m_acceptButton.setFlat(true);
        m_fileDialogOpener.setFlat(true);
        row1.addWidget(&m_filenameLabel, 0, Qt::AlignmentFlag::AlignLeft);
        row1.addWidget(&m_fileNameBox, 1, Qt::AlignmentFlag::AlignRight);
        row2.addWidget(&m_filePathLabel, 0, Qt::AlignmentFlag::AlignLeft);
        row2.addWidget(&m_filePathBox, 1, Qt::AlignmentFlag::AlignRight);
        row2.addWidget(&m_fileDialogOpener, 0, Qt::AlignmentFlag::AlignRight);
        row3.addWidget(&m_rejectButton, 0, Qt::AlignmentFlag::AlignLeft);
        row3.addWidget(&m_acceptButton, 0, Qt::AlignmentFlag::AlignRight);
        stack.addLayout(&row1, 0);
        stack.addLayout(&row2, 0);
        stack.addLayout(&row3, 0);
        m_fileDialogOpener.setFixedWidth(25);

        QObject::connect(&m_acceptButton, &QPushButton::clicked, this, [&](){ acceptDownload(); });
        QObject::connect(&m_rejectButton, &QPushButton::clicked, this, [&](){ rejectDownload(); });
        QObject::connect(&m_fileDialogOpener, &QPushButton::clicked, this, [&](){
            m_filePathBox.setText(
                QFileDialog::getExistingDirectory(this, "Select Directory",
                                                  QDir::rootPath(), QFileDialog::ShowDirsOnly)
                );
        });
    }

    void showWithReq(QWindow* wnd = nullptr, QWebEngineDownloadRequest* request = nullptr) {
        if(request == nullptr){this->close();}
        this->setGeometry( wnd == nullptr? QRect(QPoint(100, 100), m_dialogSizeDefault) : QRect( wnd->position(), m_dialogSizeDefault ) );
        m_requestPointer=request;
        m_fileNameBox.setText(m_requestPointer->suggestedFileName());
        m_filePathBox.setText(m_requestPointer->downloadDirectory());
        this ->setVisible(true);
        this ->activateWindow();
    }
protected:
    void keyReleaseEvent(QKeyEvent* event)override{
        if(event->key() == Qt::Key::Key_Escape){
            rejectDownload();
            event->setAccepted(true);
        }
        if(event->key() == Qt::Key::Key_Return){
            acceptDownload();
            event->setAccepted(true);
        }
    }
    void closeEvent(QCloseEvent* event)override{
        rejectDownload();
        event->setAccepted(true);
    }

private:
    inline void acceptDownload()noexcept{
        if(m_requestPointer !=nullptr){
            m_requestPointer->setDownloadDirectory(m_filePathBox.toPlainText());
            m_requestPointer->setDownloadFileName(m_fileNameBox.toPlainText());
            m_requestPointer->accept();
        }
        this ->close();
    }
    inline void rejectDownload()noexcept{
        m_requestPointer->cancel();
        this ->close();
    }

    QHBoxLayout row1 = QHBoxLayout();
    QHBoxLayout row2 = QHBoxLayout();
    QHBoxLayout row3 = QHBoxLayout();
    QVBoxLayout stack = QVBoxLayout(this);

    const QSize m_dialogSizeDefault = QSize (350, 200);
    QPointer<QWebEngineDownloadRequest>m_requestPointer=nullptr;
    QLabel m_filenameLabel = QLabel("File name: ");
    QLabel m_filePathLabel = QLabel("File path: ");
    QTextEdit m_fileNameBox = QTextEdit();
    QTextEdit m_filePathBox = QTextEdit();

    QPushButton m_fileDialogOpener = QPushButton("...");
    QPushButton m_acceptButton = QPushButton("Accept");
    QPushButton m_rejectButton = QPushButton("Reject");
};
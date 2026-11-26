// NOTE: This component and the accompanying .cpp file are planned to be moved into the QComponentsproj module.

//NOTE: I really wanna abandon this dumpster fire.
#pragma once

#include "intrin.h"
#include <qqml.h>
#include <QQuickItem>
#include <QObject>
#include <QPointer>
#include <QList>
#include <QProperty>
#include <QQmlProperty>
#include <QHash>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QVariantMap>
#include <QDebug>
// #include "qvariant.h"

class QQuickStack : public QQuickItem{
    Q_OBJECT
    QML_NAMED_ELEMENT(StackWidget)
    Q_PROPERTY(QQuickItem* currentTab READ getCurrentTab NOTIFY currentTabChanged FINAL)
    Q_PROPERTY(qint64 currentIndex READ getCurrentIndex WRITE switchToIndex NOTIFY currentIndexChanged FINAL)
public:

    //Return value can only be used to check if the method was called (and therefore exists).
    Q_INVOKABLE bool invokeNamedFunctionOnCurrentTab(const QByteArray& name){
        return QMetaObject::invokeMethod(getCurrentTab(), name);
    }

    Q_INVOKABLE void removeCacheItem(const QString& item){
        qint64 idx = defaultCacheItems.indexOf(item);
        if(idx != -1)defaultCacheItems.removeAt(idx);
    }

    Q_INVOKABLE void addCacheItem(const QString& item){
        if(defaultCacheItems.contains(item) == false)defaultCacheItems.append(item); }

    // returns errorValue if read was unsuccessful
    Q_INVOKABLE QVariant readPropertyCache(const QString& propertyName, const QVariant& errorValue = false ){
        QQmlProperty prop = propertyCacheTable.value(propertyName);
        return prop.isValid()? prop.read() : errorValue;
    }

    // returns true if cache write was valid and successful, otherwise returns false.
    Q_INVOKABLE bool writePropertyCache(const QString& propertyName, const QVariant& data = QVariant(), const bool insertIfMissing = false){
        QQmlProperty prop = propertyCacheTable.value(propertyName);
        if( prop.isValid() == false ){
            if(insertIfMissing == true){
                QQmlProperty newProp(getCurrentTab(), propertyName);
                if(newProp.isValid() == true)propertyCacheTable.insert(propertyName, newProp);
                return true;
            }else
                if(insertIfMissing == false) return false;
        }
        prop.write(data);
        return true;
    }

    Q_INVOKABLE void rebuildPropertyCache(){
        this -> propertyCacheTable.clear();
        for( const QString item : defaultCacheItems ){
            QQmlProperty prop(getCurrentTab(), item);
            if( prop.isValid() == true )propertyCacheTable.insert(item, prop);
        }
    }

    Q_INVOKABLE qint64 getCurrentIndex()const{return m_currentIndex;}
    Q_INVOKABLE void switchToIndex(qint64 idx){
        if(childItems().size() == 0) return; // if there are no children, why switch tabs at all?
        if(idx >= childItems().size())return;

        if(idx < 0) {idx = childItems().size() -1;} //wrap-around
        m_currentIndex = idx;
        this->ensureCorrectVisibility();

        emit this ->currentTabChanged(m_currentIndex);
        emit this -> currentIndexChanged(); // Note to self, do NOT connect a functor to BOTH the CurrentTabChanged and CurrentIndexChanged signals.
    }

    Q_INVOKABLE void deleteTab( qint64 idx){
        if( (idx <0 || idx >= childItems().size()) || childItems().size() == 0){return;}
        childItems().at(idx)->deleteLater();
        switchToIndex(getCurrentIndex() -1); // switches to previous tab. Maybe I should make a convenience function to jump between tab indexes?
    }

    Q_INVOKABLE QQuickItem* getCurrentTab()const{
        return childItems().isEmpty()? nullptr : childItems().at( m_currentIndex );
    }

    Q_INVOKABLE void ensureCorrectVisibility(){
        QQuickItem* currentChild = getCurrentTab();
        if(currentChild == nullptr) return;
        for(int i = 0; i < childItems().size(); i++){
            if(this -> childItems().at(i) != currentChild){ this ->childItems().at(i) -> setVisible(false); }
        }
        if(currentChild -> isVisible() == false)currentChild -> setVisible(true);
        if( childItems().at(getCurrentIndex()) ->size() != this ->size() ) childItems().at(m_currentIndex)->setSize(this->size());
        if(currentChild ->hasActiveFocus() == false) currentChild ->forceActiveFocus();
    }

    Q_INVOKABLE void registerComponent( const QByteArray& src, const QByteArray& nickname = ""){
        QPointer<QQmlComponent> ptr = new QQmlComponent(
            QQmlEngine::contextForObject(this)->engine()
            , src
            , QQmlComponent::CompilationMode::PreferSynchronous);

        componentTable.insertOrAssign({nickname == "" ? src : nickname}, ptr);
        qDebug()<<"COMPONENT: "<<ptr.data();
    }

    Q_INVOKABLE void createObject(const QByteArray& nickname, bool focusOnCreation, const QVariantMap& properties){
        if( componentTable[nickname].data()->isReady() == true){
            qDebug()<<"READY!";
            QQuickItem* obj =  qobject_cast<QQuickItem*>(componentTable[nickname]->createWithInitialProperties(properties));
            obj->setParentItem(this);
            switchToIndex(-1);
            qDebug()<<"CREATED: " <<obj << "THIS: " << this;
            return;
        }
    }

    QQuickStack(QQuickItem* parent = nullptr){
        QObject::connect(this, &QQuickItem::heightChanged, [&](){
            QQuickItem* tab = getCurrentTab();
            if(tab != nullptr)tab->setHeight( this->height() );
        });
        QObject::connect(this, &QQuickItem::widthChanged, [&](){
            QQuickItem* tab = getCurrentTab();
            if(tab != nullptr)tab->setWidth( this->width() );
        });

        QObject::connect(this, &QQuickItem::visibleChildrenChanged, this,
                         [&](){  this -> ensureCorrectVisibility(); } );

        QObject::connect(this, &QQuickStack::currentTabChanged, this,[&](){ this -> rebuildPropertyCache(); });
    }

signals:
    void currentIndexChanged();
    void currentTabChanged(qint64 new_idx);

private:

    QList<QString> defaultCacheItems{"url", "loadProgress"};
    QHash< QByteArray , QPointer<QQmlComponent> > componentTable;
    QHash<QString, QQmlProperty> propertyCacheTable;
    qint64 m_currentIndex = 0;
};

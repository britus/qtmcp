/**
 * @file MCPHandlerResolver.cpp
 * @brief MCP Handler解析器实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPHandlerResolver.h"
#include <QCoreApplication>
#include <QVariant>
#include <QApplication>
#include <QWidget>

namespace
{
    // 添加对象的Handler到映射表
    void addHandlers(QObject* pObj, QMap<QString, QObject*>& handlers, bool resourceOnly)
    {
        QString strObjectName = pObj->objectName();
        if (!strObjectName.isEmpty())
        {
            handlers[strObjectName] = pObj;
        }
        
        QString strResourceHandlerName = pObj->property("MCPResourceHandlerName").toString();
        if (!strResourceHandlerName.isEmpty())
        {
            handlers[strResourceHandlerName] = pObj;
        }
        
        if (!resourceOnly)
        {
            QString strHandlerName = pObj->property("MPCToolHandlerName").toString();
            if (!strHandlerName.isEmpty())
            {
                handlers[strHandlerName] = pObj;
            }
        }
    }
    
    // 遍历对象列表并添加Handler
    void processObjects(const QList<QObject*>& objects, QMap<QString, QObject*>& handlers, bool resourceOnly)
    {
        for (QObject* pObj : objects)
        {
            addHandlers(pObj, handlers, resourceOnly);
        }
    }
    
    // 遍历默认范围（qApp子对象和所有QWidget）
    void processDefaultScope(QMap<QString, QObject*>& handlers, bool resourceOnly)
    {
        QCoreApplication* pApp = QCoreApplication::instance();
        if (pApp != nullptr)
        {
            processObjects(pApp->findChildren<QObject*>(), handlers, resourceOnly);
        }
        
        QApplication* pQApp = qobject_cast<QApplication*>(pApp);
        if (pQApp != nullptr)
        {
            QWidgetList allWidgets = QApplication::allWidgets();
            for (QWidget* pWidget : allWidgets)
            {
                addHandlers(pWidget, handlers, resourceOnly);
            }
        }
    }
}

QMap<QString, QObject*> MCPHandlerResolver::resolveHandlers(QObject* pSearchRoot)
{
    QMap<QString, QObject*> handlers;
    if (pSearchRoot == nullptr)
    {
        processDefaultScope(handlers, false);
    }
    else
    {
        processObjects(pSearchRoot->findChildren<QObject*>(), handlers, false);
    }
    return handlers;
}

QMap<QString, QObject*> MCPHandlerResolver::resolveResourceHandlers(QObject* pSearchRoot)
{
    QMap<QString, QObject*> handlers;
    if (pSearchRoot == nullptr)
    {
        processDefaultScope(handlers, true);
    }
    else
    {
        processObjects(pSearchRoot->findChildren<QObject*>(), handlers, true);
    }
    return handlers;
}

QObject* MCPHandlerResolver::findHandler(const QString& strHandlerName, QObject* pSearchRoot)
{
    QMap<QString, QObject*> handlers = resolveHandlers(pSearchRoot);
    return handlers.value(strHandlerName, nullptr);
}

QMap<QString, QObject*> MCPHandlerResolver::resolveDefaultHandlers()
{
    QMap<QString, QObject*> handlers;
    processDefaultScope(handlers, false);
    return handlers;
}


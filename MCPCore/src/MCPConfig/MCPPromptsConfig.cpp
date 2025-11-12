/**
 * @file MCPPromptsConfig.cpp
 * @brief MCP提示词配置类实现
 * @author zhangheng
 * @date 2025-01-09
 * @copyright Copyright (c) 2025 zhangheng. All rights reserved.
 */

#include "MCPPromptsConfig.h"
#include "IMCPServer.h"
#include "MCPLog/MCPLog.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDirIterator>

// ============================================================================
// MCPPromptConfig 实现
// ============================================================================

QJsonObject MCPPromptConfig::toJson() const
{
    QJsonObject json;
    json["name"] = strName;
    json["description"] = strDescription;
    json["template"] = strTemplate;
    
    QJsonArray argsArray;
    for (const MCPPromptArgumentConfig& arg : listArguments)
    {
        QJsonObject argObj;
        argObj["name"] = arg.strName;
        argObj["description"] = arg.strDescription;
        argObj["required"] = arg.bRequired;
        argsArray.append(argObj);
    }
    json["arguments"] = argsArray;
    
    return json;
}

MCPPromptConfig MCPPromptConfig::fromJson(const QJsonObject& json)
{
    MCPPromptConfig config;
    config.strName = json["name"].toString();
    config.strDescription = json["description"].toString();
    config.strTemplate = json["template"].toString();
    
    if (json.contains("arguments"))
    {
        QJsonArray argsArray = json["arguments"].toArray();
        for (const QJsonValue& argValue : argsArray)
        {
            if (argValue.isObject())
            {
                QJsonObject argObj = argValue.toObject();
                MCPPromptArgumentConfig arg;
                arg.strName = argObj["name"].toString();
                arg.strDescription = argObj["description"].toString();
                arg.bRequired = argObj["required"].toBool(false);
                config.listArguments.append(arg);
            }
        }
    }
    
    return config;
}

// ============================================================================
// MCPPromptsConfig 实现
// ============================================================================

MCPPromptsConfig::MCPPromptsConfig(QObject* pParent)
    : QObject(pParent)
{
}

MCPPromptsConfig::~MCPPromptsConfig()
{
}

void MCPPromptsConfig::addPrompt(const MCPPromptConfig& promptConfig)
{
    m_listPromptConfigs.append(promptConfig);
}

QList<MCPPromptConfig> MCPPromptsConfig::getPrompts() const
{
    return m_listPromptConfigs;
}

int MCPPromptsConfig::getPromptCount() const
{
    return m_listPromptConfigs.size();
}

void MCPPromptsConfig::clear()
{
    m_listPromptConfigs.clear();
}

void MCPPromptsConfig::loadFromJson(const QJsonArray& jsonArray)
{
    for (const QJsonValue& value : jsonArray)
    {
        if (value.isObject())
        {
            m_listPromptConfigs.append(MCPPromptConfig::fromJson(value.toObject()));
        }
    }
}

QJsonArray MCPPromptsConfig::toJson() const
{
    QJsonArray jsonArray;
    
    for (const MCPPromptConfig& promptConfig : m_listPromptConfigs)
    {
        jsonArray.append(promptConfig.toJson());
    }
    
    return jsonArray;
}

int MCPPromptsConfig::loadFromDirectory(const QString& strDirPath)
{
    QDir dir(strDirPath);
    if (!dir.exists())
    {
        MCP_CORE_LOG_WARNING() << "MCPPromptsConfig: 目录不存在:" << strDirPath;
        return 0;
    }
    
    m_listPromptConfigs.clear();
    
    // 递归查找所有.json文件（包括所有层级的子目录，递归遍历）
    QDirIterator it(strDirPath, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext())
    {
        QString fullPath = it.next();
        
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly))
        {
            MCP_CORE_LOG_WARNING() << "MCPPromptsConfig: 无法打开文件:" << fullPath;
            continue;
        }
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();
        
        if (parseError.error != QJsonParseError::NoError)
        {
            MCP_CORE_LOG_WARNING() << "MCPPromptsConfig: JSON解析错误:" << fullPath << parseError.errorString();
            continue;
        }
        
        if (doc.isObject())
        {
            m_listPromptConfigs.append(MCPPromptConfig::fromJson(doc.object()));
        }
    }
    
    MCP_CORE_LOG_INFO() << "MCPPromptsConfig: 从目录加载了" << m_listPromptConfigs.size() << "个提示词配置";
    return m_listPromptConfigs.size();
}


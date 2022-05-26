#include "StdAfx.h"
#include "ApplicationConfig.h"
#include "Utils.h"

const ApplicationConfig::GlobalConfig& ApplicationConfig::Global() const
{
    return m_globalConfig;
}

const std::list<std::shared_ptr<ApplicationConfig::DomainConfig>>& ApplicationConfig::Domains() const
{
    return m_domainConfigs;
}

std::shared_ptr<const ApplicationConfig::DomainConfig> ApplicationConfig::GetDomainConfig(const std::string& domain)
{
    std::shared_lock<std::shared_mutex> locked(m_lock);

    for (const std::shared_ptr<DomainConfig>& domainConfig : Domains())
    {
        for (const std::string& domainPattern : domainConfig->domainPatterns)
        {
            if (Utils::MatchString(domain.c_str(), domainPattern.c_str()))
                return domainConfig;
        }
    }

    return nullptr;
}

bool ApplicationConfig::LoadFile(const std::wstring& filePath)
{
    const std::string configString = Utils::ReadTextFile(filePath.c_str());

    return Load(configString);
}

bool ApplicationConfig::Load(const std::string& configString)
{
    YAML::Node configNode;

    if (!configString.empty())
    {
        try
        {
            configNode = YAML::Load(configString);
        }
        catch (const YAML::Exception&)
        {
            return false;
        }
    }

    return Load(configNode);
}

bool ApplicationConfig::Load(YAML::Node configNode)
{
    GlobalConfig globalConfig;
    std::list<std::shared_ptr<DomainConfig>> domainConfigs;

    globalConfig.includeSubdomains = true;
    globalConfig.httpFragmentationEnabled = true;
    globalConfig.httpFragmentationOffset = 2;
    globalConfig.httpFragmentationOutOfOrder = true;
    globalConfig.tlsFragmentationEnabled = true;
    globalConfig.tlsFragmentationOffset = 2;
    globalConfig.tlsFragmentationOutOfOrder = true;

    if (configNode.IsNull())
    {
        std::unique_lock<std::shared_mutex> locked(m_lock);

        m_globalConfig = globalConfig;
        return true;
    }

    if (!configNode.IsMap())
        return false;

    YAML::Node globalConfigNode = configNode["global"];
    YAML::Node domainConfigsNode = configNode["domains"];

    if (globalConfigNode.IsDefined())
    {
        if (!globalConfigNode.IsMap())
            return false;

        YAML::Node includeSubdomainsNode = globalConfigNode["includeSubdomains"];
        YAML::Node httpFragmentationNode = globalConfigNode["httpFragmentation"];
        YAML::Node tlsFragmentationNode = globalConfigNode["tlsFragmentation"];

        if (includeSubdomainsNode.IsDefined())
        {
            if (!includeSubdomainsNode.IsScalar())
                return false;

            try
            {
                globalConfig.includeSubdomains = includeSubdomainsNode.as<bool>();
            }
            catch (const YAML::Exception&)
            {
            }
        }

        if (httpFragmentationNode.IsDefined())
        {
            if (!tlsFragmentationNode.IsMap())
                return false;

            YAML::Node httpFragmentationEnabledNode = httpFragmentationNode["enabled"];
            YAML::Node httpFragmentationOffsetNode = httpFragmentationNode["offset"];
            YAML::Node httpFragmentationOutOfOrderNode = httpFragmentationNode["outOfOrder"];

            if (httpFragmentationEnabledNode.IsDefined() && !httpFragmentationEnabledNode.IsScalar())
                return false;
            if (httpFragmentationOffsetNode.IsDefined() && !httpFragmentationOffsetNode.IsScalar())
                return false;
            if (httpFragmentationOutOfOrderNode.IsDefined() && !httpFragmentationOutOfOrderNode.IsScalar())
                return false;

            try
            {
                globalConfig.httpFragmentationEnabled = httpFragmentationEnabledNode.as<bool>();
            }
            catch (const YAML::Exception&)
            {
            }

            try
            {
                globalConfig.httpFragmentationOffset = httpFragmentationOffsetNode.as<size_t>();
            }
            catch (const YAML::Exception&)
            {
            }

            try
            {
                globalConfig.httpFragmentationOutOfOrder = httpFragmentationOutOfOrderNode.as<bool>();
            }
            catch (const YAML::Exception&)
            {
            }
        }

        if (tlsFragmentationNode.IsDefined())
        {
            if (!tlsFragmentationNode.IsMap())
                return false;

            YAML::Node tlsFragmentationEnabledNode = tlsFragmentationNode["enabled"];
            YAML::Node tlsFragmentationOffsetNode = tlsFragmentationNode["offset"];
            YAML::Node tlsFragmentationOutOfOrderNode = tlsFragmentationNode["outOfOrder"];

            if (tlsFragmentationEnabledNode.IsDefined() && !tlsFragmentationEnabledNode.IsScalar())
                return false;
            if (tlsFragmentationOffsetNode.IsDefined() && !tlsFragmentationOffsetNode.IsScalar())
                return false;
            if (tlsFragmentationOutOfOrderNode.IsDefined() && !tlsFragmentationOutOfOrderNode.IsScalar())
                return false;

            try
            {
                globalConfig.tlsFragmentationEnabled = tlsFragmentationEnabledNode.as<bool>();
            }
            catch (const YAML::Exception&)
            {
            }

            try
            {
                globalConfig.tlsFragmentationOffset = tlsFragmentationOffsetNode.as<size_t>();
            }
            catch (const YAML::Exception&)
            {
            }

            try
            {
                globalConfig.tlsFragmentationOutOfOrder = tlsFragmentationOutOfOrderNode.as<bool>();
            }
            catch (const YAML::Exception&)
            {
            }
        }
    }

    if (domainConfigsNode.IsDefined())
    {
        if (!domainConfigsNode.IsSequence())
            return false;

        for (YAML::Node domainConfigNode : domainConfigsNode)
        {
            std::shared_ptr<DomainConfig> domainConfig = std::make_shared<DomainConfig>();

            domainConfig->includeSubdomains = globalConfig.includeSubdomains;
            domainConfig->httpFragmentationEnabled = globalConfig.httpFragmentationEnabled;
            domainConfig->httpFragmentationOffset = globalConfig.httpFragmentationOffset;
            domainConfig->httpFragmentationOutOfOrder = globalConfig.httpFragmentationOutOfOrder;
            domainConfig->tlsFragmentationEnabled = globalConfig.tlsFragmentationEnabled;
            domainConfig->tlsFragmentationOffset = globalConfig.tlsFragmentationOffset;
            domainConfig->tlsFragmentationOutOfOrder = globalConfig.tlsFragmentationOutOfOrder;

            if (domainConfigNode.IsMap())
            {
                YAML::Node domainNode = domainConfigNode["domain"];
                YAML::Node includeSubdomainsNode = domainConfigNode["includeSubdomains"];
                YAML::Node httpFragmentationNode = globalConfigNode["httpFragmentation"];
                YAML::Node tlsFragmentationNode = domainConfigNode["tlsFragmentation"];

                if (!domainNode.IsScalar())
                    return false;

                try
                {
                    std::string domain = domainNode.as<std::string>();

                    if (domain.empty())
                        return false;

                    domainConfig->domain = domain;
                }
                catch (const YAML::Exception&)
                {
                }

                if (includeSubdomainsNode.IsDefined())
                {
                    if (!includeSubdomainsNode.IsScalar())
                        return false;

                    try
                    {
                        domainConfig->includeSubdomains = includeSubdomainsNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }
                }

                if (httpFragmentationNode.IsDefined())
                {
                    if (!httpFragmentationNode.IsMap())
                        return false;

                    YAML::Node httpFragmentationEnabledNode = httpFragmentationNode["enabled"];
                    YAML::Node httpFragmentationOffsetNode = httpFragmentationNode["offset"];
                    YAML::Node httpFragmentationOutOfOrderNode = httpFragmentationNode["outOfOrder"];

                    if (httpFragmentationEnabledNode.IsDefined() && !httpFragmentationEnabledNode.IsScalar())
                        return false;
                    if (httpFragmentationOffsetNode.IsDefined() && !httpFragmentationOffsetNode.IsScalar())
                        return false;
                    if (httpFragmentationOutOfOrderNode.IsDefined() && !httpFragmentationOutOfOrderNode.IsScalar())
                        return false;

                    try
                    {
                        domainConfig->httpFragmentationEnabled = httpFragmentationEnabledNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig->httpFragmentationOffset = httpFragmentationOffsetNode.as<size_t>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig->httpFragmentationOutOfOrder = httpFragmentationOutOfOrderNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }
                }

                if (tlsFragmentationNode.IsDefined())
                {
                    if (!tlsFragmentationNode.IsMap())
                        return false;

                    YAML::Node tlsFragmentationEnabledNode = tlsFragmentationNode["enabled"];
                    YAML::Node tlsFragmentationOffsetNode = tlsFragmentationNode["offset"];
                    YAML::Node tlsFragmentationOutOfOrderNode = tlsFragmentationNode["outOfOrder"];

                    if (tlsFragmentationEnabledNode.IsDefined() && !tlsFragmentationEnabledNode.IsScalar())
                        return false;
                    if (tlsFragmentationOffsetNode.IsDefined() && !tlsFragmentationOffsetNode.IsScalar())
                        return false;
                    if (tlsFragmentationOutOfOrderNode.IsDefined() && !tlsFragmentationOutOfOrderNode.IsScalar())
                        return false;

                    try
                    {
                        domainConfig->tlsFragmentationEnabled = tlsFragmentationEnabledNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig->tlsFragmentationOffset = tlsFragmentationOffsetNode.as<size_t>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig->tlsFragmentationOutOfOrder = tlsFragmentationOutOfOrderNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }
                }
            }
            else if (domainConfigNode.IsScalar())
            {
                try
                {
                    std::string domain = domainConfigNode.as<std::string>();

                    if (domain.empty())
                        return false;

                    domainConfig->domain = domain;
                }
                catch (const YAML::Exception&)
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            domainConfig->domainPatterns.push_back(domainConfig->domain);
            if (domainConfig->includeSubdomains)
                domainConfig->domainPatterns.push_back("*." + domainConfig->domain);

            domainConfigs.push_back(std::move(domainConfig));
        }
    }

    {
        std::unique_lock<std::shared_mutex> locked(m_lock);

        m_globalConfig = globalConfig;
        m_domainConfigs = std::move(domainConfigs);
    }

    return true;
}

bool ApplicationConfig::SaveFile(const std::wstring& filePath) const
{
    try
    {
        const YAML::Node configNode = Save();
        const std::string configString = YAML::Dump(configNode);

        return Utils::WriteTextFile(configString, filePath.c_str());
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}

YAML::Node ApplicationConfig::Save() const
{
    YAML::Node configNode;

    YAML::Node globalConfigNode = configNode["global"];
    globalConfigNode["includeSubdomains"] = m_globalConfig.includeSubdomains;

    YAML::Node httpFragmentationNode = globalConfigNode["httpFragmentation"];
    httpFragmentationNode["enabled"] = m_globalConfig.httpFragmentationEnabled;
    httpFragmentationNode["offset"] = m_globalConfig.httpFragmentationOffset;
    httpFragmentationNode["outOfOrder"] = m_globalConfig.httpFragmentationOutOfOrder;

    YAML::Node tlsFragmentationNode = globalConfigNode["tlsFragmentation"];
    tlsFragmentationNode["enabled"] = m_globalConfig.tlsFragmentationEnabled;
    tlsFragmentationNode["offset"] = m_globalConfig.tlsFragmentationOffset;
    tlsFragmentationNode["outOfOrder"] = m_globalConfig.tlsFragmentationOutOfOrder;

    YAML::Node domainsConfigNode = configNode["domains"];
    for (const std::shared_ptr<DomainConfig>& domainConfig : m_domainConfigs)
    {
        YAML::Node domainConfigNode;

        if (m_globalConfig == *domainConfig)
        {
            domainConfigNode = domainConfig->domain;
        }
        else
        {
            domainConfigNode["domain"] = domainConfig->domain;

            if (m_globalConfig.includeSubdomains != domainConfig->includeSubdomains)
                domainConfigNode["includeSubdomains"] = domainConfig->includeSubdomains;

            if (m_globalConfig.httpFragmentationEnabled != domainConfig->httpFragmentationEnabled)
                domainConfigNode["httpFragmentation"]["enabled"] = domainConfig->httpFragmentationEnabled;
            if (m_globalConfig.httpFragmentationOffset != domainConfig->httpFragmentationOffset)
                domainConfigNode["httpFragmentation"]["offset"] = domainConfig->httpFragmentationOffset;
            if (m_globalConfig.httpFragmentationOutOfOrder != domainConfig->httpFragmentationOutOfOrder)
                domainConfigNode["httpFragmentation"]["outOfOrder"] = domainConfig->httpFragmentationOutOfOrder;

            if (m_globalConfig.tlsFragmentationEnabled != domainConfig->tlsFragmentationEnabled)
                domainConfigNode["tlsFragmentation"]["enabled"] = domainConfig->tlsFragmentationEnabled;
            if (m_globalConfig.tlsFragmentationOffset != domainConfig->tlsFragmentationOffset)
                domainConfigNode["tlsFragmentation"]["offset"] = domainConfig->tlsFragmentationOffset;
            if (m_globalConfig.tlsFragmentationOutOfOrder != domainConfig->tlsFragmentationOutOfOrder)
                domainConfigNode["tlsFragmentation"]["outOfOrder"] = domainConfig->tlsFragmentationOutOfOrder;
        }

        domainsConfigNode.push_back(domainConfigNode);
    }

    return configNode;
}

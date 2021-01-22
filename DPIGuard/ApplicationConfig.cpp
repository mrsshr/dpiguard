#include "StdAfx.h"
#include "ApplicationConfig.h"
#include "Utils.h"

ApplicationConfig::ApplicationConfig()
{
}

const ApplicationConfig::GlobalConfig& ApplicationConfig::Global() const
{
    return m_globalConfig;
}

const std::list<ApplicationConfig::DomainConfig>& ApplicationConfig::Domains() const
{
    return m_domainConfigs;
}

const ApplicationConfig::ApplicationConfig::DomainConfig* ApplicationConfig::GetDomainConfig(const std::string& domain)
{
    std::unique_lock<std::mutex> locked(m_lock);

    for (const DomainConfig& domainConfig : Domains())
    {
        size_t offset = 0;

        if (domain.size() < domainConfig.domain.size())
            continue;

        if (domainConfig.includeSubdomains)
            offset = domain.size() - domainConfig.domain.size();

        bool match = std::equal(
            domain.cbegin() + offset,
            domain.cend(),
            domainConfig.domain.cbegin(),
            domainConfig.domain.cend(),
            [](char a, char b) {
                return std::toupper(a) == std::toupper(b);
            });

        if (!match)
            continue;

        if (offset > 0 && domain[offset - 1] != '.')
            continue; // Not a subdomain

        return &domainConfig;
    }

    return nullptr;
}

bool ApplicationConfig::LoadFile(const std::wstring& filePath)
{
    std::string configString = Utils::ReadTextFile(filePath.c_str());

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
    std::list<DomainConfig> domainConfigs;

    globalConfig.includeSubdomains = true;
    globalConfig.httpFragmentationEnabled = true;
    globalConfig.httpFragmentationOffset = 2;
    globalConfig.tlsFragmentationEnabled = true;
    globalConfig.tlsFragmentationOffset = 2;

    if (configNode.IsNull())
    {
        std::unique_lock<std::mutex> locked(m_lock);

        m_globalConfig = std::move(globalConfig);
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

            if (httpFragmentationEnabledNode.IsDefined() && !httpFragmentationEnabledNode.IsScalar())
                return false;
            if (httpFragmentationOffsetNode.IsDefined() && !httpFragmentationOffsetNode.IsScalar())
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
        }

        if (tlsFragmentationNode.IsDefined())
        {
            if (!tlsFragmentationNode.IsMap())
                return false;

            YAML::Node tlsFragmentationEnabledNode = tlsFragmentationNode["enabled"];
            YAML::Node tlsFragmentationOffsetNode = tlsFragmentationNode["offset"];

            if (tlsFragmentationEnabledNode.IsDefined() && !tlsFragmentationEnabledNode.IsScalar())
                return false;
            if (tlsFragmentationOffsetNode.IsDefined() && !tlsFragmentationOffsetNode.IsScalar())
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
        }
    }

    if (domainConfigsNode.IsDefined())
    {
        if (!domainConfigsNode.IsSequence())
            return false;

        for (YAML::Node domainConfigNode : domainConfigsNode)
        {
            DomainConfig domainConfig;

            domainConfig.includeSubdomains = globalConfig.includeSubdomains;
            domainConfig.httpFragmentationEnabled = globalConfig.httpFragmentationEnabled;
            domainConfig.httpFragmentationOffset = globalConfig.httpFragmentationOffset;
            domainConfig.tlsFragmentationEnabled = globalConfig.tlsFragmentationEnabled;
            domainConfig.tlsFragmentationOffset = globalConfig.tlsFragmentationOffset;

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

                    domainConfig.domain = domain;
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
                        domainConfig.includeSubdomains = includeSubdomainsNode.as<bool>();
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

                    if (httpFragmentationEnabledNode.IsDefined() && !httpFragmentationEnabledNode.IsScalar())
                        return false;
                    if (httpFragmentationOffsetNode.IsDefined() && !httpFragmentationOffsetNode.IsScalar())
                        return false;

                    try
                    {
                        domainConfig.httpFragmentationEnabled = httpFragmentationEnabledNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig.httpFragmentationOffset = httpFragmentationOffsetNode.as<size_t>();
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

                    if (tlsFragmentationEnabledNode.IsDefined() && !tlsFragmentationEnabledNode.IsScalar())
                        return false;
                    if (tlsFragmentationOffsetNode.IsDefined() && !tlsFragmentationOffsetNode.IsScalar())
                        return false;

                    try
                    {
                        domainConfig.tlsFragmentationEnabled = tlsFragmentationEnabledNode.as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                    }

                    try
                    {
                        domainConfig.tlsFragmentationOffset = tlsFragmentationOffsetNode.as<size_t>();
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

                    domainConfig.domain = domain;
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

            domainConfigs.push_back(domainConfig);
        }
    }

    {
        std::unique_lock<std::mutex> locked(m_lock);

        m_globalConfig = std::move(globalConfig);
        m_domainConfigs = std::move(domainConfigs);
    }

    return true;
}

bool ApplicationConfig::SaveFile(const std::wstring& filePath)
{
    try
    {
        YAML::Node configNode = Save();
        std::string configString = YAML::Dump(configNode);

        return Utils::WriteTextFile(configString, filePath.c_str());
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}

YAML::Node ApplicationConfig::Save()
{
    YAML::Node configNode;

    YAML::Node globalConfigNode = configNode["global"];
    globalConfigNode["includeSubdomains"] = m_globalConfig.includeSubdomains;

    YAML::Node httpFragmentationNode = globalConfigNode["httpFragmentation"];
    httpFragmentationNode["enabled"] = m_globalConfig.httpFragmentationEnabled;
    httpFragmentationNode["offset"] = m_globalConfig.httpFragmentationOffset;

    YAML::Node tlsFragmentationNode = globalConfigNode["tlsFragmentation"];
    tlsFragmentationNode["enabled"] = m_globalConfig.tlsFragmentationEnabled;
    tlsFragmentationNode["offset"] = m_globalConfig.tlsFragmentationOffset;

    YAML::Node domainsConfigNode = configNode["domains"];
    for (const DomainConfig& domainConfig : m_domainConfigs)
    {
        YAML::Node domainConfigNode;

        if (m_globalConfig == domainConfig)
        {
            domainConfigNode = domainConfig.domain;
        }
        else
        {
            domainConfigNode["domain"] = domainConfig.domain;

            if (m_globalConfig.includeSubdomains != domainConfig.includeSubdomains)
                domainConfigNode["includeSubdomains"] = domainConfig.includeSubdomains;

            if (m_globalConfig.httpFragmentationEnabled != domainConfig.httpFragmentationEnabled)
                domainConfigNode["httpFragmentation"]["enabled"] = domainConfig.httpFragmentationEnabled;
            if (m_globalConfig.httpFragmentationOffset != domainConfig.httpFragmentationOffset)
                domainConfigNode["httpFragmentation"]["offset"] = domainConfig.httpFragmentationOffset;

            if (m_globalConfig.tlsFragmentationEnabled != domainConfig.tlsFragmentationEnabled)
                domainConfigNode["tlsFragmentation"]["enabled"] = domainConfig.tlsFragmentationEnabled;
            if (m_globalConfig.tlsFragmentationOffset != domainConfig.tlsFragmentationOffset)
                domainConfigNode["tlsFragmentation"]["offset"] = domainConfig.tlsFragmentationOffset;
        }

        domainsConfigNode.push_back(domainConfigNode);
    }

    return configNode;
}

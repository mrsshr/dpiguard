#pragma once

class ApplicationConfig
{
public:
    struct DomainConfig
    {
        DomainConfig()
        {
            includeSubdomains = false;

            httpFragmentationEnabled = false;
            httpFragmentationOffset = 0;

            tlsFragmentationEnabled = false;
            tlsFragmentationOffset = 0;
        }

        std::list<std::string> domainPatterns;
        std::string domain;
        bool includeSubdomains;

        bool httpFragmentationEnabled;
        size_t httpFragmentationOffset;

        bool tlsFragmentationEnabled;
        size_t tlsFragmentationOffset;
    };

    struct GlobalConfig
    {
        GlobalConfig()
        {
            includeSubdomains = false;

            httpFragmentationEnabled = false;
            httpFragmentationOffset = 0;

            tlsFragmentationEnabled = false;
            tlsFragmentationOffset = 0;
        }

        bool operator==(const DomainConfig& rhs) const
        {
            if (includeSubdomains != rhs.includeSubdomains)
                return false;

            if (httpFragmentationEnabled != rhs.httpFragmentationEnabled)
                return false;
            if (httpFragmentationOffset != rhs.httpFragmentationOffset)
                return false;

            if (tlsFragmentationEnabled != rhs.tlsFragmentationEnabled)
                return false;
            if (tlsFragmentationOffset != rhs.tlsFragmentationOffset)
                return false;

            return true;
        }

        bool includeSubdomains;

        bool httpFragmentationEnabled;
        size_t httpFragmentationOffset;

        bool tlsFragmentationEnabled;
        size_t tlsFragmentationOffset;
    };
public:
    ApplicationConfig();

    const GlobalConfig& Global() const;
    const std::list<std::shared_ptr<DomainConfig>>& Domains() const;

    std::shared_ptr<const DomainConfig> GetDomainConfig(const std::string& domain);

    bool LoadFile(const std::wstring& filePath);
    bool Load(const std::string& configString);
    bool Load(YAML::Node configNode);

    bool SaveFile(const std::wstring& filePath);
    YAML::Node Save();
private:
    GlobalConfig m_globalConfig;
    std::list<std::shared_ptr<DomainConfig>> m_domainConfigs;

    std::shared_mutex m_lock;
};

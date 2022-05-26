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
            httpFragmentationOutOfOrder = false;

            tlsFragmentationEnabled = false;
            tlsFragmentationOffset = 0;
            tlsFragmentationOutOfOrder = false;
        }

        std::list<std::string> domainPatterns;
        std::string domain;
        bool includeSubdomains;

        bool httpFragmentationEnabled;
        size_t httpFragmentationOffset;
        bool httpFragmentationOutOfOrder;

        bool tlsFragmentationEnabled;
        size_t tlsFragmentationOffset;
        bool tlsFragmentationOutOfOrder;
    };

    struct GlobalConfig
    {
        GlobalConfig()
        {
            includeSubdomains = false;

            httpFragmentationEnabled = false;
            httpFragmentationOffset = 0;
            httpFragmentationOutOfOrder = false;

            tlsFragmentationEnabled = false;
            tlsFragmentationOffset = 0;
            tlsFragmentationOutOfOrder = false;
        }

        bool operator==(const DomainConfig& rhs) const
        {
            if (includeSubdomains != rhs.includeSubdomains)
                return false;

            if (httpFragmentationEnabled != rhs.httpFragmentationEnabled)
                return false;
            if (httpFragmentationOffset != rhs.httpFragmentationOffset)
                return false;
            if (httpFragmentationOutOfOrder != rhs.httpFragmentationOutOfOrder)
                return false;

            if (tlsFragmentationEnabled != rhs.tlsFragmentationEnabled)
                return false;
            if (tlsFragmentationOffset != rhs.tlsFragmentationOffset)
                return false;
            if (tlsFragmentationOutOfOrder != rhs.tlsFragmentationOutOfOrder)
                return false;

            return true;
        }

        bool includeSubdomains;

        bool httpFragmentationEnabled;
        size_t httpFragmentationOffset;
        bool httpFragmentationOutOfOrder;

        bool tlsFragmentationEnabled;
        size_t tlsFragmentationOffset;
        bool tlsFragmentationOutOfOrder;
    };
public:
    ApplicationConfig() = default;

    const GlobalConfig& Global() const;
    const std::list<std::shared_ptr<DomainConfig>>& Domains() const;

    std::shared_ptr<const DomainConfig> GetDomainConfig(const std::string& domain);

    bool LoadFile(const std::wstring& filePath);
    bool Load(const std::string& configString);
    bool Load(YAML::Node configNode);

    bool SaveFile(const std::wstring& filePath) const;
    YAML::Node Save() const;
private:
    GlobalConfig m_globalConfig;
    std::list<std::shared_ptr<DomainConfig>> m_domainConfigs;

    std::shared_mutex m_lock;
};

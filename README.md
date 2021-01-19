DPIGuard
========

Configurable HTTP, TLS fragmentation tool



* HTTP, TLS fragmentation only for specified domains
* Support running as a service (Manual installation)
* Small memory footprint



## Example configuration

```yaml
global:
  includeSubdomains: true
  httpFragmentation:
    enabled: true
    offset: 2
  tlsFragmentation:
    enabled: true
    offset: 2
domains:
  - example.com # example.com will include subdomains
  - domain: example2.com # example2.com will not include subdomains
    includeSubdomains: false
  - domain: example3.com # Override global configuration
    httpFragmentation:
      enabled: false
    tlsFragmentation:
      offset: 20
```



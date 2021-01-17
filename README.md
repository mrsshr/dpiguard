DPIGuard
========

Configurable TLS ClientHello fragmentation tool



## Example configuration

```yaml
global:
  includeSubdomains: true
  tlsFragmentation:
    enabled: true
    offset: 2
domains:
  - example.com # example.com will include subdomains
  - domain: example2.com # example2.com will not include subdomains
    includeSubdomains: false
  - domain: example3.com # Override global configuration
    tlsFragmentation:
      offset: 20
```



## TODO

- [ ] Automatic reloading of configuration file

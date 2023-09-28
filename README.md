# ngx_http_c18n_checksum_body_module

Custom nginx module for computing a CRC32 checksum over the request body. 

## Build

```
auto/configure --with-cc-opt='-Wno-cheri-provenance' --with-compat --add-dynamic-module=src/ngx_http_c18n_checksum_body_module --with-debug
gmake modules
```

## Configure

```
location /test {
  checksum_body on;
  ...
}
```

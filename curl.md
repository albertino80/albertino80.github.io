get a web page
```
curl https://curl.haxx.se
```

Http service for tests: [https://httpbin.org/](https://httpbin.org/)

```
curl "https://httpbin.org/get?uno=1&due=2"
curl --data "uno=1&due=2"  https://httpbin.org/post
curl --upload-file testdata.txt https://httpbin.org/put
curl --user alberto:pass https://httpbin.org/basic-auth/alberto/pass
```

Save reply to file:
```
curl --user alberto:pass https://httpbin.org/basic-auth/alberto/pass -o reply.txt
```

For source code generation add  `--libcurl filename.c` option
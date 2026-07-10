// nova_http_shim.c — тонкая обёртка над libcurl. curl_easy_setopt в реальном C — variadic
// (переменное число аргументов), а наш FFI пока такое не умеет — поэтому вся возня с
// curl держится внутри шима на C, а Nova видит только простой string-in/string-out API.
//
// Собрать: gcc -shared -o nova_http_shim.dll nova_http_shim.c
//          -I <ucrt64>/include -L <ucrt64>/lib -lcurl -Wl,--out-implib,libnova_http_shim.a

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct nova_buf {
    char* data;
    size_t len;
    size_t cap;
};

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    struct nova_buf* buf = (struct nova_buf*)userp;
    if (buf->len + total + 1 > buf->cap) {
        size_t new_cap = (buf->cap == 0 ? 4096 : buf->cap * 2);
        while (new_cap < buf->len + total + 1) new_cap *= 2;
        buf->data = realloc(buf->data, new_cap);
        buf->cap = new_cap;
    }
    memcpy(buf->data + buf->len, contents, total);
    buf->len += total;
    buf->data[buf->len] = 0;
    return total;
}

static const char* do_request(const char* url, const char* method, const char* body) {
    static int initialized = 0;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        initialized = 1;
    }
    CURL* curl = curl_easy_init();
    if (!curl) {
        char* out = malloc(1);
        out[0] = 0;
        return out;
    }
    struct nova_buf buf = { NULL, 0, 0 };
    buf.data = malloc(1);
    buf.data[0] = 0;
    buf.cap = 1;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "nova-lang/0.1");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body ? body : "");
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        free(buf.data);
        const char* err = curl_easy_strerror(res);
        char* out = malloc(strlen(err) + 1);
        strcpy(out, err);
        curl_easy_cleanup(curl);
        return out;  // тело ошибки в этом простом шиме не отличить от успешного ответа — см. nova_http_status
    }
    curl_easy_cleanup(curl);
    return buf.data;
}

__declspec(dllexport) const char* nova_http_get(const char* url) {
    return do_request(url, "GET", NULL);
}

__declspec(dllexport) const char* nova_http_post(const char* url, const char* body) {
    return do_request(url, "POST", body);
}

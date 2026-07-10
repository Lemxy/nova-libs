<div align="center">

# nova-libs

**Официальные библиотеки для [Nova](https://github.com/lemxy/Nova) — ставятся одной командой, без ручного FFI.**

[![Nova](https://img.shields.io/badge/language-Nova-15803D)](https://github.com/lemxy/Nova)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()

</div>

---

## Зачем это

Nova умеет вызывать любую C-библиотеку напрямую через `extern "lib" fn ...` — но писать
сырые FFI-декларации каждый раз скучно и ошибкоопасно. Здесь — готовые, человекочитаемые
обёртки поверх популярных C-библиотек, устанавливаемые через встроенный пакетный менеджер:

```bash
python nova_pkg.py init
python nova_pkg.py add sqlite github:lemxy/nova-libs/sqlite@main
```

```nova
import "nova_modules/sqlite/sqlite.nova"

fn main() {
    db = db_open("app.db")
    db_exec(db, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)")
}
```

Никакого `extern`, никакой возни с сигнатурами — просто `import` и работающие функции.

---

## Библиотеки

<table>
<tr><th>Пакет</th><th>Что даёт</th><th>DLL-зависимости</th></tr>

<tr>
<td><code>sqlite</code></td>
<td>Полноценная встраиваемая SQL-база: <code>db_open</code>, <code>db_exec</code>,
<code>db_query_int</code>, <code>db_query_text</code>, работа со <code>stmt</code> напрямую</td>
<td><code>sqlite3.dll</code>, <code>libsqlite3-0.dll</code></td>
</tr>

<tr>
<td><code>http</code></td>
<td>HTTP-сервер поверх сокетов: <code>http_parse_request</code>,
<code>http_json / http_html / http_ok / http_not_found / http_serve_file</code></td>
<td>— (чистый Nova, только встроенные сокеты)</td>
</tr>

<tr>
<td><code>json</code></td>
<td><code>json_get</code> — чтение плоских JSON-объектов, <code>json_escape</code> —
экранирование строк для ручной сборки JSON</td>
<td>—</td>
</tr>

<tr>
<td><code>crypto</code></td>
<td>SHA-256, HMAC и другие примитивы через OpenSSL</td>
<td><code>libcrypto-3-x64.dll</code>, <code>libssl-3-x64.dll</code>, <code>nova_crypto_shim.dll</code></td>
</tr>

<tr>
<td><code>http_client</code></td>
<td>Исходящие HTTP-запросы (GET/POST и т.д.) через libcurl</td>
<td><code>libcurl-4.dll</code>, <code>nova_http_shim.dll</code></td>
</tr>

<tr>
<td><code>zlib</code></td>
<td>Сжатие / распаковка данных (deflate/gzip)</td>
<td><code>zlib1.dll</code></td>
</tr>

</table>

> Встроенный `json_object(key, value, ...)` для *сборки* JSON есть прямо в языке (не требует
> пакета) — `json`-библиотека нужна для *чтения* входящего JSON и ручного экранирования.

---

## Установка

Каждая библиотека — отдельная подпапка этого репозитория. Пакетный менеджер Nova умеет тянуть
конкретную подпапку монорепо без скачивания всего остального:

```bash
python nova_pkg.py add <имя>  github:lemxy/nova-libs/<папка>@main
```

Примеры:

```bash
python nova_pkg.py add sqlite      github:lemxy/nova-libs/sqlite@main
python nova_pkg.py add http        github:lemxy/nova-libs/http@main
python nova_pkg.py add json        github:lemxy/nova-libs/json@main
python nova_pkg.py add crypto      github:lemxy/nova-libs/crypto@main
python nova_pkg.py add http_client github:lemxy/nova-libs/http_client@main
python nova_pkg.py add zlib        github:lemxy/nova-libs/zlib@main
```

Всё установленное фиксируется в `nova.json` (что нужно) и `nova.lock` (что реально скачано,
с sha256 — для воспроизводимости).

### DLL

Библиотеки с C-зависимостями несут нужные `.dll` прямо в своей подпапке. После установки
скопируй их из `nova_modules/<имя>/` в корень проекта (рядом со скомпилированным `.bin`) —
компилятор и Windows-загрузчик ищут DLL там же, где сам исполняемый файл.

### Требования к платформе

Все DLL собраны под **Windows x64 (MinGW-w64 / UCRT)**. `http` и `json` — чистый Nova-код,
работают везде, где работает сам язык.

---

## Структура репозитория

```
nova-libs/
├── sqlite/         sqlite.nova + sqlite3.dll + libsqlite3-0.dll
├── http/           http.nova
├── json/           json.nova
├── crypto/         crypto.nova + OpenSSL DLL + C-шим (исходник included)
├── http_client/    http_client.nova + libcurl DLL + C-шим (исходник included)
└── zlib/           zlib.nova + zlib1.dll
```

`crypto` и `http_client` используют небольшие C-шимы (`nova_*_shim.c`) поверх OpenSSL/libcurl —
эти API слишком сложны для прямого `extern` (буферы, variadic-аргументы), поэтому шим даёт
Nova-дружелюбную сигнатуру. Исходники included для прозрачности; собранные `.dll` уже готовы
к использованию, пересборка не обязательна.

---

## Добавить свою библиотеку в этот репозиторий

1. Новая подпапка на верхнем уровне — её имя станет именем пакета для `nova pkg add`.
2. `.nova`-файл(ы) с человекочитаемым API поверх `extern "libname" fn ...`.
3. Все нужные `.dll` — рядом, в той же подпапке.
4. Строка в таблице выше + пример установки.

PR приветствуются.

---

<div align="center">

Часть проекта **[Nova](https://github.com/lemxy/Nova)** — языка программирования,
компилируемого в нативный машинный код через C.

</div>

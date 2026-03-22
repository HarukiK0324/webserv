 1. CGI の役割分担（誰が何をやる？）

### 🖥 サーバーの役割（network）

- TCP の接続を処理
- HTTP リクエストのパース
- どの CGI を呼ぶか判断
- CGI の出力を **HTTP レスポンス**としてクライアントへ返す

### 🧩 CGI スクリプトの役割（program logic）

- データアクセス（DB など）
- 動的な HTML 生成
- 必要な処理を実行し、**stdout にレスポンスを書く**

これらが役割分担。


---



# メタ変数

| 名前                | ざっくり意味                                            |
| ----------------- | ------------------------------------------------- |
| `REQUEST_METHOD`  | GET / POST / HEAD など                              |
| `QUERY_STRING`    | `?` 以降のクエリ（URLエンコードされた文字列）                        |
| `CONTENT_LENGTH`  | 本文のバイト数（POST のとき特に重要）                             |
| `CONTENT_TYPE`    | 本文の種類（フォーム, JSON, ファイルなど）                         |
| `SCRIPT_NAME`     | スクリプト自身のパス（`/cgi-bin/test.cgi` など）                |
| `PATH_INFO`       | スクリプトに続く追加パス（`/test.cgi/123/abc` の `/123/abc` 部分） |
| `SERVER_NAME`     | サーバーのホスト名（例：`example.com`）                        |
| `SERVER_PORT`     | 受けたポート番号（80, 8080, 8000 など）                       |
| `SERVER_PROTOCOL` | `HTTP/1.1` など                                     |
| `REMOTE_ADDR`     | クライアントの IP アドレス                                   |

---
## Script-URI


> **複数の違う URI から同じ CGI が呼ばれても、
> CGI の内部では同じメタ変数（SCRIPT_NAME・PATH_INFO など）になるように、
> サーバーが “正しい一つの URI（Script-URI）” に正規化（統合）する。**

この目的が Script-URI です。

---

# 🔰 あなたの例で説明する（server_name が複数ある場合）

```
server {
    listen 80;
    server_name example.com www.example.com blog.example.com;
    root /var/www/example;
}
```

ここでは **3つの異なるホスト名** でアクセスできます：

* `http://example.com/cgi/test.cgi`
* `http://www.example.com/cgi/test.cgi`
* `http://blog.example.com/cgi/test.cgi`

でも、**実行される CGI は同じ test.cgi**。
しかし CGI のメタ変数に入る値は Host によってバラバラになり得ます。

---

CGI が見られるメタ変数：

* `SCRIPT_NAME`
* `PATH_INFO`
* `QUERY_STRING`
* `SERVER_NAME`
* `SERVER_PORT`
---

## 🟢 Script-URI の役割は「一つの正規の URI へ統合」

サーバーは内部で、

> **複数の入口（URI）を、ひとつの“標準となる URI”にまとめる**


つまり：

### たくさんのアクセスURL

```
example.com/cgi/test.cgi
www.example.com/cgi/test.cgi
blog.example.com/cgi/test.cgi
```

### ↓ サーバー内部で一つの Script-URI に統合

```
http://example.com/cgi/test.cgi
```

# 🔥 補足：server_name が複数ある場合、Script-URI をどう決める？

サーバーは以下から Script-URI を構築します：

* SERVER_PROTOCOL（例：HTTP/1.1 → http）
* SERVER_NAME（host 名）
* SERVER_PORT（80）
* SCRIPT_NAME
* PATH_INFO
* QUERY_STRING

サーバーは *どの server_name を“正規の名前”にするか* を決めます（仕様上は自由）。

多くのサーバーでは：

* 最初に書かれた server_name をデフォルトにする
  例：上記の server ブロックでは `example.com`
* または config で指定される “canonical name” を使う



| 応答タイプ                                                              | Content-Type      | Status                       | Location                        | Message-Body  | その他ヘッダ                   |
| ------------------------------------------------------------------ | ----------------- | ---------------------------- | ------------------------------- | ------------- | ------------------------ |
| **6.2.1 Document Response**                                  | **MUST**（本文がある場合） | MAY（省略時は200）                 | MUST NOT（普通は不要）                 | MAY（本文返してOK）  | MAY（プロトコル固有ヘッダ OK）       |
| **6.2.2 Local Redirect Response**                      | **MUST NOT**      | MUST NOT                     | **MUST（local-path の Location）** | **MUST NOT**  | MUST NOT（CGI拡張も不可）       |
| **6.2.3 Client Redirect Response**                   | MUST NOT          | MUST NOT（サーバーが302を付ける）       | **MUST（絶対URI）**                 | **MUST NOT**  | MUST NOT（※サーバー定義拡張ヘッダ除く） |
| **6.2.4 Client Redirect Response with Document** | **MUST**          | **MUST（302 または他のリダイレクトコード）** | **MUST（絶対URI）**                 | MAY（本文を添付できる） | MAY（他のヘッダを付けてもOK）        |


# 8章


#  ③ PATH_TRANSLATED とは？

### 🌟 一言で：

**PATH_INFO を「物理ファイルパス」に変換した値。**

---

### 🔍 例で理解しよう

URL：

```
http://example.com/cgi-bin/hello.cgi/foo/bar
```

サーバが決めている：

```
SCRIPT_NAME = /cgi-bin/hello.cgi
PATH_INFO   = /foo/bar
DOCUMENT_ROOT = /var/www/html
```

このとき：

```
PATH_TRANSLATED = /var/www/html/foo/bar
```

つまり：

> PATH_INFO（URLの余り部分）を、サーバのファイルシステム上の絶対パスに翻訳したもの

---

### 🌟 PATH_INFO と PATH_TRANSLATED の関係

| 変数                  | 内容                           |
| ------------------- | ---------------------------- |
| **PATH_INFO**       | URL の「余った部分」                 |
| **PATH_TRANSLATED** | PATH_INFO をドキュメントルートへ変換した実パス |
| **SCRIPT_NAME**     | 実行された CGI のパス                |

---

### 🔍 例：CGIで画像処理をする CGI

```
/image.cgi/dog.jpg
```

なら：

```
PATH_INFO       = /dog.jpg
PATH_TRANSLATED = /var/www/html/dog.jpg  ← 実ファイル
```

CGI スクリプトは：

* PATH_TRANSLATED を開いて読み込む
* 画像加工して返す

という風に使える。

---

# 🔥 まとめ

| **PATH_TRANSLATED**  | PATH_INFO をサーバ上の実ファイルパスに変換したもの                 |

---

# 重要な要件　3,4,6章

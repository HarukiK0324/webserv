## 🌐 **ディレクティブ一覧**

| ディレクティブ名         | 文法（Syntax）                                               | デフォルト値（Default）               | Context（適用範囲） | 説明                                                    |     |
| ------------------------ | ------------------------------------------------------------ | ------------------------------------- | ------------------- | ------------------------------------------------------- | --- |
| **listen**               | `listen address[:port]`<br>`listen address`<br>`listen port` | `*:80`（root）<br>`*:8000`（非 root） | server              | サーバーの待ち受けアドレスとポートを設定                |     |
| **server_name**          | `server_name name...;`                                       | `""`                                  | server              | 対象 server ブロックのホスト名。virtual host 判定に使用 |     |
| **root**                 | `root path;`                                                 | `html`                                | server, location    | ドキュメントルートの基準ディレクトリ                    |     |
| **index**                | `index file...;`                                             | `index.html`                          | server, location    | `/` にアクセスされたときのインデックスファイル一覧      |     |
| **client_max_body_size** | `client_max_body_size size;`                                 | `1m`                                  | server, location    | クライアントが送信できるボディサイズ上限                |     |
| **keepalive_timeout**    | `keepalive_timeout time;`                                    | `75s`                                 | server, location    | Keep-Alive 接続を維持する秒数                           |     |
| **error_page**           | `error_page code... uri;`                                    | —                                     | server, location    | 特定のエラー発生時に返す内部リダイレクト先              |     |
| **return**               | `return code [text];`<br>`return code URL;`<br>`return URL;` | —                                     | server, location    | 即時レスポンス返却。リダイレクトに利用                  |     |
| **autoindex**            | `autoindex on or off;`                                       | `off`                                 | server, location    | ディレクトリリスティングを有効/無効化                   |
| **upload_enable**        | `upload_enable on or  off;`                                  | `off`                                 | location            | POST のアップロード機能の有効/無効                      |
| **cgi_pass**             | `cgi_pass .ext /path/to/cgi`                                 | —                                     | **location**        | 拡張子と CGI 実行バイナリを関連付ける                   |
| **upload_path**          | `upload_path /path`                                          | —                                     | **location**        | アップロードファイルの保存先                            |
| **allow_methods**        | `allow_methods GET POST DELETE`                              | —                                     | **location**        | 許可する HTTP メソッドの制限                            |

---

# 🎉 **さらに読みやすい README 形式（オプション）**

必要ならこのようにセクション化もできます：

---

## 📡 1. 接続設定

| ディレクティブ  | 説明                          |
| --------------- | ----------------------------- |
| **listen**      | アクセス受付のアドレス/ポート |
| **server_name** | バーチャルホスト名            |

---

## 📁 2. ルーティング / パスの設定

| ディレクティブ | 説明                          |
| -------------- | ----------------------------- |
| **root**       | ドキュメントルート            |
| **index**      | インデックスファイル          |
| **error_page** | エラー発生時のリダイレクト先  |
| **return**     | 即時レスポンス / リダイレクト |
| **autoindex**  | 自動ディレクトリ一覧表示      |

---

## 📤 3. リクエスト制御

| ディレクティブ           | 説明                      |
| ------------------------ | ------------------------- |
| **client_max_body_size** | ボディサイズ上限          |
| **keepalive_timeout**    | Keep-Alive のタイムアウト |

---

## 📦 4. アップロード機能

| ディレクティブ    | 説明                  |
| ----------------- | --------------------- |
| **upload_enable** | POST アップロード許可 |

---
 # Listen ソケットの関係について

| host の関係 | port の関係 | listen ソケット数 | ServerConfig の持ち方 |
|------------|------------|------------------|----------------------|
| 同じ       | 同じ       | 1                | 1つの listen に複数 |
| 異なる     | 同じ       | 2                | listen ごとに1つ    |
| 同じ       | 異なる     | 2                | listen ごとに1つ    |
| 異なる     | 異なる     | 2                | listen ごとに1つ    |

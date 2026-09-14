/*
 * ESP32-C6: Wi‑Fi точка доступа + веб-страница с надписью/рисунком
 *
 * Как пользоваться:
 * - Прошей скетч
 * - Подключись к Wi‑Fi: ESP32C6-AP (пароль ниже)
 * - Открой в браузере: http://192.168.4.1/
 *
 * Зависимости: стандартные WiFi/WebServer из Arduino-ESP32 core
 */

#include <WiFi.h>
#include <WebServer.h>

// ===== Настройки точки доступа =====
static const char* AP_SSID = "ESP32C6-AP";
static const char* AP_PASS = "12345678"; // минимум 8 символов

WebServer server(80);

static const char kIndexHtml[] PROGMEM = R"HTML(
<!doctype html>
<html lang="ru">
  <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width,initial-scale=1"/>
    <title>ESP32‑C6 AP</title>
    <style>
      body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;background:#0b1020;color:#e8eefc;margin:0;display:flex;min-height:100vh;align-items:center;justify-content:center}
      .card{width:min(720px,92vw);background:rgba(255,255,255,.06);border:1px solid rgba(255,255,255,.12);border-radius:18px;padding:18px 18px 14px;box-shadow:0 10px 35px rgba(0,0,0,.35)}
      h1{margin:0 0 8px;font-size:20px}
      p{margin:0 0 12px;opacity:.85}
      canvas{width:100%;height:auto;border-radius:14px;background:linear-gradient(135deg,#1a2a6c,#b21f1f,#fdbb2d)}
      .foot{margin-top:10px;font-size:12px;opacity:.75;display:flex;gap:10px;flex-wrap:wrap}
      code{background:rgba(255,255,255,.08);padding:2px 6px;border-radius:8px}
    </style>
  </head>
  <body>
    <div class="card">
      <h1>Привет от ESP32‑C6</h1>
      <p>Это точка доступа и простая страница с canvas‑рисунком.</p>
      <canvas id="c" width="900" height="420"></canvas>
      <div class="foot">
        <div>IP: <code>192.168.4.1</code></div>
        <div>URI: <code>/</code></div>
        <div>ESP32‑C6</div>
      </div>
    </div>
    <script>
      const c = document.getElementById('c');
      const g = c.getContext('2d');
      function draw(){
        g.clearRect(0,0,c.width,c.height);
        // рамка
        g.fillStyle='rgba(0,0,0,.22)';
        g.fillRect(18,18,c.width-36,c.height-36);
        g.strokeStyle='rgba(255,255,255,.35)';
        g.lineWidth=4;
        g.strokeRect(18,18,c.width-36,c.height-36);
        // "смайлик" точками
        function dot(x,y,r,col){g.beginPath();g.fillStyle=col;g.arc(x,y,r,0,Math.PI*2);g.fill();}
        dot(300,170,22,'#fff');
        dot(600,170,22,'#fff');
        for(let i=0;i<11;i++){
          const t=i/10;
          const x=330+t*240;
          const y=250+Math.sin((t-0.5)*Math.PI)*32;
          dot(x,y,10,'#ffe082');
        }
        // надпись
        g.fillStyle='#e8eefc';
        g.font='700 46px system-ui, -apple-system, Segoe UI, Roboto, Arial';
        g.fillText('ESP32‑C6 AP', 40, 95);
        g.font='500 24px system-ui, -apple-system, Segoe UI, Roboto, Arial';
        g.fillStyle='rgba(232,238,252,.92)';
        g.fillText('Открой 192.168.4.1', 40, 135);
      }
      draw();
    </script>
  </body>
</html>
)HTML";

static void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", FPSTR(kIndexHtml));
}

static void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8", "404: нет такого пути. Открой /");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_AP);
  const bool ok = WiFi.softAP(AP_SSID, AP_PASS);

  Serial.println();
  Serial.println("=== ESP32‑C6 AP Web ===");
  Serial.print("AP start: ");
  Serial.println(ok ? "OK" : "FAILED");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("PASS: ");
  Serial.println(AP_PASS);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP()); // обычно 192.168.4.1

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}


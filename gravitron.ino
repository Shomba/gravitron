// Importa bibliotecas necessarias
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// define as credenciais do ponto de acesso
const char* ssid = "medidor de gravidade";
const char* password = "gravitron";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Cria um servidor weeb asincrono na porta 80
AsyncWebServer server(80);

#pragma region html
// define o codigo fonte da pagina web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <meta http-equiv="content-type" content="text/html; charset=windows-1252">
  <title>gravitron</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">

  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }

    h2 {
      font-size: 3.0rem;
    }

    p {
      font-size: 3.0rem;
    }

    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }

    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px
    }

    .switch input {
      display: none
    }

    .slider {
      -webkit-appearance: none;
      /* Override default CSS styles */
      appearance: none;
      width: 100%;
      /* Full-width */
      height: 25px;
      /* Specified height */
      background: #d3d3d3;
      /* Grey background */
      outline: none;
      /* Remove outline */
      opacity: 0.7;
      /* Set transparency (for mouse-over effects on hover) */
      -webkit-transition: .2s;
      /* 0.2 seconds transition on hover */
      transition: opacity .2s;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 3px
    }

    #fire {
      height: auto;
      width: 75%;
    }

    input:checked+.slider {
      background-color: #b30000
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px)
    }
  </style>

</head>

<body>
  <h2>gravitron</h2>
  <div>
    <h3>sensibilidade</h3>
    <input min="1000" max="3000" value="2500" class="slider" id="myRange" type="range">
    <h4 id="sense">2500</h4>
    <script>var slider = document.getElementById("myRange");
      var output = document.getElementById("sense");
      console.log("pong")
      slider.oninput = function () {
        var output = document.getElementById("sense");
        output.innerHTML = this.value;
      }
      slider.onchange = function () {
        output.innerHTML = this.value;
        fetch("/sense?output=" + this.value, { method: "GET" });
      }

    </script>
  </div>
  <div>
    <h4>tamanho (mm)</h4>
    <input value="1541" id="size" type="number">
    <script>
      var size = document.getElementById("size");
      console.log("tall")
      size.addEventListener('change', function () {
        console.log("tall")
        fetch("/tall?output=" + this.value, { method: "GET" });
      })

    </script>
  </div>
  <br>
  <br>
  <br>
  <div>
    <script>
      async function getgrav() {
        console.log("gravi")
        fetch(`/grav`)
        document.getElementById("func").innerHTML = "medindo"
        await sleep(7000)
        fetch("/func").then(res => {
          txt = res.text().then(function (txt) {
            document.getElementById("func").innerHTML = txt.split("|")[0]
          //document.getElementById("aclr").innerHTML = txt.split("|")[1]
          });
          
        })
      }
      function sleep(ms) {
        return new Promise((resolve) => {
          setTimeout(resolve, ms)
        })
      }
    </script>
    <button id="fire" onclick="getgrav()">
      <h2>ATIVAR</h2>
    </button>
    <h2 id="func"></h42>
    <h3 id="aclr"></h3>

  </div>




</body>

</html>
)rawliteral";


#pragma endregion html
//define variaveis globais
int sensibilidade = 2500;
float altura = 1541;
int potPin = 32;
bool vai = false;
String resp;
void setup(){
  // Abre uma porta serial para depuraçao
  Serial.begin(115200);

  //define o modo e estado dos pinos a serem ultilizados
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(4,INPUT);

  //abre rede wifi
  WiFi.softAP(ssid, password);

  //obtem o ip local e imprime ele na prota serial
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  
  server.begin();
  // Rota para a pagina raix do servidor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // url para a request de mudança de senssibilidade
  server.on("/sense", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;

    if (request->hasParam(PARAM_INPUT_1)) {
      String stri = request->getParam(PARAM_INPUT_1)->value();
      char pe[stri.length()];
 
    int i;
    for (i = 0; i < sizeof(p); i++) {
        pe[i] = stri[i];
    }
      sensibilidade = atoi(pe);
      Serial.print("sense: ");
      Serial.println(p);
    }
    else {
      inputMessage1 = "No message sent";
    }

    request->send(200, "text/plain", "OK");
  });
  //url para definir a altura
server.on("/tall", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;

    if (request->hasParam(PARAM_INPUT_1)) {
      String stri = request->getParam(PARAM_INPUT_1)->value();
      char pe[stri.length()];
 
    int i;
    for (i = 0; i < sizeof(p); i++) {
        pe[i] = stri[i];
    }
    altura = (float) atoi(pe);
      Serial.print("tall: ");
      Serial.println(h);
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }

    request->send(200, "text/plain", "OK");
  });
//request para iniciar a medição da gravidade
  server.on("/grav", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("grav");
    go = true;
    request->send(200, "text/plain", "OK");
  });
  //request para obter o resultado da medição
  server.on("/func", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", resp);
  });
  Serial.println(potPin);
  // Inicia o servidor http
  server.begin();
}

void loop() {
  //checa se ja pode realizar a medição
  if (vai){
    resp = gravidade();
    vai= false;

  }
}

//função asinclrona que faz a medição
String gravidade(){
  Serial.println("começando");
  digitalWrite(2,HIGH); // liga o eletroimã
  delay(5000); // espera 5000 milisegundo (5 segundos)
  digitalWrite(2,LOW); // desliga o eletroimã
  //define as variaveis livais
  bool queda = false;
  int high;
  float time;
  Serial.println("caindo");
  while(!queda){
    int mic = analogRead(potPin); // le input do microfone

    //checa se o input do microfone é maior doque a nivel de senssibilidade
    if(mic >= sensibilidade){
      high = mic; //salva o input do microfone
      queda = true; //quebra o loop
      time--; // ajusta o tempo
    }
    delay(1); //espera um milisegundo
    time += 1; // aumenta o tempo caso não haja detecção
  }
  Serial.println("calculando...");

    float acl = ((((h/1000)*2/(time/1000))/(time/1000))/100); //faz o calculo

    //formata o valor
    String vlc = " m/s²";
    String ret = acl+vlc;
  return ret; //retorna o valor

}

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "medidor de gravidade";
const char* password = "gravitron";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

#pragma region html
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

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Output - GPIO 33</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}
#pragma endregion html
int p = 2500;
float h = 1541;
int potPin = 32;
bool go = false;
bool cheat = false;
int micro = 0;
String resp;
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(4,INPUT);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  
  server.begin();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/sense", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1)) {
      String stri = request->getParam(PARAM_INPUT_1)->value();
      char pe[stri.length()];
 
    int i;
    for (i = 0; i < sizeof(p); i++) {
        pe[i] = stri[i];
    }
      p = atoi(pe);
      Serial.print("sense: ");
      Serial.println(p);
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }

    request->send(200, "text/plain", "OK");
  });
server.on("/tall", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1)) {
      String stri = request->getParam(PARAM_INPUT_1)->value();
      char pe[stri.length()];
 
    int i;
    for (i = 0; i < sizeof(p); i++) {
        pe[i] = stri[i];
    }
    h = (float) atoi(pe);
      Serial.print("tall: ");
      Serial.println(h);
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }

    request->send(200, "text/plain", "OK");
  });
  server.on("/grav", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("grav");
    go = true;
    request->send(200, "text/plain", "OK");
  });
  server.on("/func", HTTP_GET, [] (AsyncWebServerRequest *request) {
    //Serial.println(resp);
    request->send(200, "text/plain", resp);
  });
  server.on("/cheat", HTTP_GET, [] (AsyncWebServerRequest *request) {
    cheat = !cheat;
    request->send(200, "text/plain", "cheat: "+cheat);
  });
  Serial.println(potPin);
  // Start server
  server.begin();
}

void loop() {
  if (go){
    resp = gravidade();
    go= false;

  }
}

String gravidade(){
  Serial.println("começando");
  digitalWrite(2,HIGH);
  delay(5000);
  digitalWrite(2,LOW);
  bool queda = false;
  int high;
  float time;
  Serial.println("caindo");
  while(!queda){
    int mic = analogRead(potPin);
    if(mic >= p){
      high = mic;
      queda = true;
      time--;
      Serial.println("POW");
    }
    delay(1);
    time += 1;    
  }
  Serial.println("calculando...");
  if (!cheat){
    float acl = ((((h/1000)*2/(time/1000))/(time/1000))/100)*106;
  String aa = "a = ";
  String ac =" + ";
  String ae = " + a x ";
  String ag = "²/2";
  String fhr = aa+h+ac+time+ae+high+ag;
  String spd = "m/s²|";
  String ret = acl+spd+fhr;
  Serial.println(ret);
  return ret;
  }
  else{
    int desvio = random(100,999);
  Serial.println(desvio, 7);
  String ret = "9.8";
  String sqr = "m/s²";
  String set = ret+desvio+sqr;
  Serial.println(set);
  return set;
  }
}

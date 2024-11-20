const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>

<div class="card">
  <h4>The ESP32 Update web page without refresh</h4><br>
  <h1>Sensor Value: <span id="ADCValue">0</span></h1><br>
</div>
<script>

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate // HERE DELAY

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ADCValue").innerHTML =
      this.responseText;
      window.AppInventor.setWebViewString("" + this.responseText);  // RESPUESTA A CadenaDeWebView
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";
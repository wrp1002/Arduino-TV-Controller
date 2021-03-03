//  Main page. Displays info and buttons to try different functions
void HandleRoot() {
  Serial.println("Sending Homepage...");

   String responseHTML = R"foo(
  <html>
    <head>
        <style>
          html { font-family: Helvetica;
             display: inline-block;
             margin: 0px auto;
             text-align: center;
             font-size: 50px;
             background-color: #202020;
             color: white;
           }
           a.button { 
            background-color: #195B6A; 
             display: block;
             border: none; 
             color: white; 
             padding: 16px 40px;
             text-decoration: none; 
             font-size: 75px;
             margin: 10px; 
             cursor: pointer;
            -webkit-user-select: none; /* Safari */        
            -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* IE10+/Edge */
            user-select: none; /* Standard */
           }
        </style>
        
        <script>
             function SendRequest(url) {
               var http = new XMLHttpRequest();
               http.open('GET', url, true);
               http.send();
             }
        
            function Update() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        let status = JSON.parse(this.responseText);
                        let powered = (status["powered"] ? "On" : "Off");
                        
                        document.getElementById("status").innerHTML = powered;
                        
                    }
                };
                xhttp.open("GET", "status", true);
                xhttp.send();
            }

            setInterval(Update, 2000);
            Update();
        </script>
    </head>

    <body>
        <h3>TV Control Web Server</h3>
        <hr>
        <br>
        <h3>Status: <span id="status">?</span></h3>
        <a id='offButton' class='button' onclick='SendRequest("/off"); Update();' style='background-color:red'>OFF</a>
        <a id='onButton' class='button' onclick='SendRequest("/on"); Update();' style='background-color:green'>ON</a>
        <a id='offButton' class='button' onclick='SendRequest("/toggle"); Update();' style='background-color:blue'>TOGGLE</a>
    </body>
  </html>
    )foo";

  //oninput for continuous updates

  server.send(200, "text/html", responseHTML);
}

void TogglePower() {
	digitalWrite(OUTPUT_PIN, LOW);
	delay(OUTPUT_DELAY);
	digitalWrite(OUTPUT_PIN, HIGH);
}

void HandleStatus() {
  String responseHTML = GetStatus();
  server.send(200, "text/html", responseHTML);
}

void HandleOn() {
	if (digitalRead(INPUT_PIN) == LOW)
		TogglePower();

	targetPowered = true;
	targetReached = false;
	String responseHTML = GetStatus();
	server.send(200, "text/html", responseHTML);
}

void HandleOff() {
	if (digitalRead(INPUT_PIN) == HIGH)
		TogglePower();

	targetPowered = false;
	targetReached = false;
	String responseHTML = GetStatus();
	server.send(200, "text/html", responseHTML);
}

//  Power OUTPUT_PIN for a set amount of time. For this example delay() is used and is okay because its only for 500ms, but this causes the entire server to stop for this amount of time, so it would not be good if the delay time is very high.
void HandleTimed() {
  TogglePower();

  String responseHTML = GetStatus();
  server.send(200, "text/html", responseHTML);
}

void HandleToggle() {
	TogglePower();
	
	targetPowered = !targetPowered;
	String responseHTML = GetStatus();
	server.send(200, "text/html", responseHTML);
}

void HandleNotFound() {
  server.send(404, "text/html", "<h1>Not Found</h1>");
}

import processing.serial.*;

short LF = 10;
char HEADER = 'A';    // character to identify the start of a message
Serial myPort;
String T_H;
float temp;
float humidity;
int xPos = 40;
int val;

int minTempDisplay = 16;
int maxTempDisplay = 30;

void setup() {
  //println(Serial.list());
  String portName = Serial.list()[5];
  //println(portName);
  myPort = new Serial(this, portName, 9600);
  size(1000, 600);
  drawBackground();
  myPort.clear();
}

void draw() {
  while (myPort.available() > 0) {
    String msg = myPort.readStringUntil(LF);
    if (msg != null) {
      //print(msg);
      String[] data = msg.split(",");
      if (data[0].charAt(0) == HEADER && data.length == 2) {
        String[] tmp = split(data[0], 'A');
        humidity = float(tmp[1]);
        temp = float(data[1]);
        //print(temp);
        //print(", ");
        //print(humidity);
        //println();
        updateDisplay();
      }
    }
  }
}

void updateDisplay() {
  float temp_map = map(temp, minTempDisplay, maxTempDisplay, height/2, 0);
  float humidity_map = map(humidity, 0, 100, height, height/2);
  //stroke(255, 0, 0);
  //strokeWeight(3);
  noStroke();
  fill(255, 0, 0);
  ellipse(xPos, temp_map, 5, 5);
  //stroke(0, 255, 255);
  noStroke();
  fill(0, 255, 255);
  ellipse(xPos, humidity_map, 5, 5);
  
  
  fill(0);
  noStroke();
  rect(220, 5, 100, 34);
  fill(255, 0, 0);
  textSize(20);
  text(temp, 220, 30);

  fill(0);
  noStroke();
  rect(202, height/2+5, 100, 34);
  //fill(255);
  fill(0, 255, 255);
  textSize(20);
  text(humidity, 202, height/2+30);



  if (xPos >= width) {
    xPos = 40;
    drawBackground();
  } else {
    xPos++;
  }
}

void drawBackground() {
  background(0);
  stroke(255);
  strokeWeight(2);
  fill(255);
  line(0, height/2, width, height/2);
  line(38, 0, 38, height); 
  float degPerPixel = (maxTempDisplay-minTempDisplay)/(height/2.0);
  strokeWeight(1);
  int displayTemp = minTempDisplay+int(degPerPixel*40.0);
  for (int i = height/2; i > 0; i -= 40) {
    line(35, i, 42, i);
    if (i != height/2) {
      displayTemp = int(maxTempDisplay-float(i)*degPerPixel);
      textSize(14);
      //displayTemp = displayTemp + int(degPerPixel*40.0);
      text(displayTemp, 10, i);

      //println(displayTemp);
    }
  }

  float humidPerPixel = (100-0)/(height/2.0);
  strokeWeight(1);
  int displayHumid = int(humidPerPixel*40);
  for (int i = height; i > height/2; i -= 40) {
    line(35, i, 42, i);
    if (i != height) {
      text(displayHumid, 10, i);
      displayHumid = displayHumid + int(humidPerPixel*40.0);
    }
  }
  textSize(20);
  text("Temperature (*C):", 50, 30);
  text("Humidity (%RH):", 50, height/2+30);
}
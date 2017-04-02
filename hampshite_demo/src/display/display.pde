import processing.serial.*;

short LF = 10;
char HEADER = 'A';    // character to identify the start of a message
Serial myPort;
String T_H;
float temp_1;
float humidity_1;
float voltage_1;
float temp_9;
float humidity_9;
float voltage_9;

int xPos_1 = 40;
int xPos_9 = 40;
int val;

int minTempDisplay = 40;
int maxTempDisplay = 90;

void setup() {
  //println(Serial.list());
  String portName = Serial.list()[2];
  println(portName);
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
      //println(data);
      if (data[0].charAt(0) == HEADER && data.length == 4) {
        String[] sensor_num = split(data[0], 'A'); //sensor num
        if (int(sensor_num[1]) == 1) {
          temp_1 = float(data[1]) * (9/5) + 32;
          humidity_1 = float(data[2]);
          voltage_1 = float(data[3]);
          update_1();
        }
        if (int(sensor_num[1]) == 9) {
          temp_9 = float(data[1]) * (9/5) + 32;
          humidity_9 = float(data[2]);
          voltage_9 = float(data[3]);
          update_9();
        }
      }
    }
  }
}

void update_1() {
  //draw temp 1
  float temp_map_1 = map(temp_1, minTempDisplay, maxTempDisplay, height/2, 0);
  float humidity_map_1 = map(humidity_1, 0, 100, height, height/2);
  //draw points
  noStroke();
  fill(255, 0, 0);
  ellipse(xPos_1, temp_map_1, 5, 5);
  
  noStroke();
  fill(0, 255, 255);
  ellipse(xPos_1, humidity_map_1, 5, 5);
  
  fill(0);
  noStroke();
  rect(75, 5, 70, 34);
  fill(255, 0, 0);
  
  textSize(18);
  text(temp_1, 75, 20);
  fill(0);
  noStroke();
  rect(75, height/2+5, 75, 34);
  
  fill(0, 255, 255);
  
  textSize(18);
  text(humidity_1, 75, height/2+20);
  
  if (xPos_1 >= width) {
    xPos_1 = 40;
    xPos_9 = 40;
    drawBackground();
  } else {
    xPos_1++;
  }
}

void update_9() {
  float temp_map_9 = map(temp_9, minTempDisplay, maxTempDisplay, height/2, 0);
  float humidity_map_9 = map(humidity_9, 0, 100, height, height/2);
  noStroke();
  fill(170, 10, 0);
  ellipse(xPos_9, temp_map_9, 5, 5);
  noStroke();
  fill(30, 200, 255);
  ellipse(xPos_9, humidity_map_9, 5, 5);

  fill(0);
  noStroke();
  rect(170, 5, 100, 34);
  fill(170, 10, 0);
  textSize(18);
  text(temp_9, 170, 20);

  fill(0);
  noStroke();
  rect(197, height/2+5, 100, 34);
  
  fill(30, 200, 255);
  textSize(18);
  text(humidity_9, 197, height/2+20);
  
  if (xPos_9 >= width) {
    xPos_1 = 40;
    xPos_9 = 40;
    drawBackground();
  } else {
    xPos_9++;
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
      text(displayTemp, 10, i);
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
  textSize(18);
  text(" T:", 50, 20);
  text("RH:", 50, height/2+20);
  
  text(" T:", 150, 20);
  text("RH:", 170, height/2+20);
}
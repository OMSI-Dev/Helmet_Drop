//library imprt //<>// //<>//
import processing.video.*;
import processing.serial.*;
import processing.sound.*;
import grafica.*;
import lord_of_galaxy.timing_utils.*;

//graph setup
public GPlot plot1;

//Serial Variables
Serial ardPort;
int inByte = -1;
int lf = 10;
int $ = 36;
String inputStr;
float[] ptData = new float[200];
String[] strData = new String[200];
long failCnt = 0;
boolean oneData = true;

//gplot specic varriables that exchange serial data
//to graph data
GPointsArray layer1points = new GPointsArray(200);
GPointsArray layer2points = new GPointsArray(200);
GPointsArray layer3points = new GPointsArray(200);

GPoint lastPoint = new GPoint(0.0,0.0);

//Graph Variables
int limit;
float x, y;
int plotSpeed = 10000;
float xPoint = 20;
float xStep = 20;
float xArrow = 0;
int dataState =1;
int lastData =0;
boolean addlayer = false;
//sets the low end & high end of the graph
//the  + 5 creates padding on the graph
float graphYmin = 0;
float graphYmax = 300 + 5;

boolean drawUpdate = false;
float lastPointX;
float lastPointY;

//timers
long senUpdate = 0;
int interval = 10;
Stopwatch Timer;

//How long gameplay is
int gameTime = 30000;


//images & bar
PImage en1, en2, en3, rArrow, yArrow, gArrow;
float xbarOffset = 150;
float largestNumber;
float enSize = 220;
float disSize = 170;

//movie
Movie attractor;
//this checks to see if the video is close enough to the end to loop
float videoOffset = 0.037457;

//sound files
SoundFile red;
SoundFile orange;
SoundFile green;
boolean playOnce;


//Text placements & strings
String spanish = "Impacto ";
String slash = "/";
String english = " Impact: ";
String units1 = " N";
//String units2 = "3";
//String units3 = "/s";
int titleX = 450;
int titleY = 150;

//gamestate flags
boolean gameOn = false;
boolean stopFlag = false;
boolean soundOn = true;
boolean firstRun = true;
boolean dataRecv = false;

//graph domains
int highVal = 200;
int lowVal = 100;

//log File
PrintWriter logFile;
//create a log file

void setup() {
  String timeStamp = str(day()) + "_" + str(hour()) + "_" + str(minute());
  logFile = createWriter(timeStamp+ "_" + "logFile.txt");
  
  fullScreen();
  size(1920, 1080);
  // create a font located in data folder
  PFont Hel = createFont("/font/helv.otf", 32);
  textFont(Hel);

  //sound assignment
  red = new SoundFile(this, "/sound/red.mp3");
  orange = new SoundFile(this, "/sound/orange.mp3");
  green = new SoundFile(this, "/sound/green.mp3");


  //image assignment
  //enable = en
  //disable = dis
  en1 = loadImage("/images/1_en.png");
  en2 = loadImage("/images/2_en.png");
  en3 = loadImage("/images/3_en.png");
  gArrow = loadImage("/images/greenArrow.gif");
  yArrow = loadImage("/images/yellowArrow.gif");
  rArrow  = loadImage("/images/redArrow.gif");

  //movie assignment
  //try {
  attractor = new Movie(this, "/images/attractor.mp4");
  //}
  //catch(Exception e) {
  //  println("error loading video");
  // }

  //set starting image postions
  imageMode(CENTER);
  image(en1, 1725, 350, 100, 100);
  imageMode(CENTER);
  image(en2, 1725, 600, 100, 100);
  imageMode(CENTER);
  image(en3, 1725, 850, 100, 100);


  // List all the available serial ports:
  printArray(Serial.list());
  logFile.println("Seraching for Serial");
  // Com1 is Serial.list()[0]
  // Serial.list()[1] will pick up the next used COM port  
  if (Serial.list().length > 1) {
    String portName = Serial.list()[1];
    ardPort = new Serial(this, portName, 115200);
    logFile.println("connected to: " + '\t' + portName);
  } else {
    String portName = Serial.list()[0];
    ardPort = new Serial(this, portName, 115200);
    logFile.println("connected to: " + '\t' + portName);
  }

  //Line Feed to set buffer to read
  ardPort.bufferUntil(lf);

  //Timer setup
  Timer = new Stopwatch(this);

  //Graph Setup
  plot1 = new GPlot(this);
  plot1.setPos(100, 215);
  plot1.setDim(1370, 757);
  plot1.setMar(0, 0, 0, 0);

  //Set Title
  updateTitle();

  //Sets Grid limit
  plot1.setYLim(graphYmin, graphYmax);
  plot1.setXLim(0.00, 210.000);
  //Sets how much info is show per axis
  plot1.setVerticalAxesNTicks(3);
  plot1.setHorizontalAxesNTicks(0);

  //sets colors & weights for plot box
  plot1.setBoxBgColor(#fed9a5);
  plot1.setBoxLineColor(0);
  plot1.setBoxLineWidth(5);
  plot1.setGridLineColor(0);

  //sets color & width for plot line
  plot1.setLineColor(0);
  plot1.setLineWidth(4.00);
  plot1.setPointColor(0);
  plot1.setPointSize(4.00);
  //per layer Setting
  plot1.addLayer("layer 1", layer1points);
  plot1.getLayer("layer 1").setLineColor(0);
  plot1.getLayer("layer 1").setLineWidth(4.00);
  plot1.getLayer("layer 1").setPointSize(4.00);
  plot1.getLayer("layer 1").setPointColor(0);

  plot1.addLayer("layer 2", layer1points);
  plot1.getLayer("layer 2").setLineColor(175);
  plot1.getLayer("layer 2").setLineWidth(2.00);
  plot1.getLayer("layer 2").setPointSize(2.00);
  plot1.getLayer("layer 2").setPointColor(175);

  //tell arduino to reestablish connection(used incase of crash or reset

  ardReset();

}

void draw()
{ 
  //setbackground to white
  background(255);
  noFill();

  
  if (gameOn == true)
  {
    //prevents movie from playing during gameplay
    if (stopFlag == true)
    {
      stopFlag = false;
      //dont use .stop() it breaks assignment of the movie file
      attractor.pause();
      //reset video to start
      attractor.jump(0);
    }

    //println("Game Timer: " + Timer.time());
    if (Timer.time() >= gameTime)
    {
      timerTick();
    }

    //update Title & Images for new sensor values
    //println("Update Title");
    updateTitle();
    //println("Update Graphics");
    imageUpdate();
    //println("update arrow postion");
    updateArrow();
    //update Graph
    //println("update Graph");
    drawGraph();
  } else
  {
    try
    {
      playMovie();     
    }
    catch(Exception e)
    {
      logFile.println("Video Fail");
      logFile.println("Error Output:" + e);
    }
  }
}


void serialEvent(Serial port){
  
    //reset largest number
    //largestNumber = 0; 
    drawUpdate = true;
  
    //clear the layer so it does not just keep adding to the array
    layer1points.set(new GPointsArray());
    //clear layer 1
    //clearLayer();
  
    //looks for incomming Sensor Data
    //trims any whitespace
    try
    {
      inputStr = trim(port.readString());
    }catch(Exception e){logFile.println("Serial Trim Failed" + e);};
    
    //print("Allow Data: ");
    //println(oneData);
    //Check to see what came in and do the appropirate action
  
  if (inputStr == null) { return; }
    
    if (inputStr.equals("#") == true)
    { 
      logFile.print("Input: ");
      logFile.println(inputStr);
      //flip the stop video flag
      stopFlag = true;
      //game is starting
      gameOn = true;
      Timer.start();
      //tell arduino to send data
      port.write(33);
      port.write(10);
    }
    
  
    if (gameOn && inputStr.equals("$"))
    { 
      logFile.print("Input: ");
      logFile.println(inputStr);
      logFile.println("Input: " + inputStr);
      //reset timer
      Timer.reset();
      Timer.start();    
    }
    
    if(inputStr.equals("@")) //<>//
    {
      logFile.print("Input: ");
      logFile.println(inputStr);
      logFile.flush();
      //logFile.close();
      oneData = true;
      //ardPort.clear();
    }
        
  try{
    if (gameOn  && !inputStr.equals("#") && !inputStr.equals("$") && !inputStr.equals("@") && oneData)
    { 
      //println("changing data");
      oneData = false;
      //cleanup the incoming string
      try
      {
      inputStr = inputStr.replaceAll(",$|^,", "");
      }catch(Exception e){logFile.println("Replace ',' failed:" + e);};
      
      try
      {
      //Split at the comma into the array
      strData = inputStr.split("\\s*,\\s*");
      }catch(Exception e){logFile.println("Split Failed:" + e);};
      
      ardPort.clear();
      
      //convert the string to float to be able to place on grid
      for (int i = 0; i<=199; i++)
      {
        if(i == 0)
        {
          logFile.println("Start Array");
        }
        ptData[i] = Float.valueOf(strData[i]); //<>//
        //print(strData[i]);
        //print(",");
        logFile.print(strData[i] + ",");
        if (i==199) 
        {
          println("");
          logFile.println(" ");
          logFile.println("End Array");
        }
      }
        
      //only update if we recived a full array
      if (ptData[0] == 1 & ptData[199] == 1)
      {
        //clear previous layer data
        clearLayer();
        largestNumber = 0;
        playOnce = true;
        //this is used to determine what the arrow height is at and what image to display
        for (int i = 0; i<=199; i++)
        {
          if (largestNumber < ptData[i])
          {
            largestNumber = ptData[i]; //<>//
          }
        }
        if(playOnce == true)
        {
        playsound();
        playOnce = false;      
        }
        logFile.println("Updating Title:");
        updateTitle();
        logFile.println("Updating points");
        updatePoints();      
      }
    }
    } catch(Exception e){logFile.println("Incorrect Array_Error Output:" + e);};
}
  


void updatePoints()
{

  //process sent ptData and apply it to the  Gpoints array
  for (int i = 0; i<=199; i++)
  {
    //x isn't being used from the sensor
    //its populated evenly to update the graph so its easier to read
    //it is based on the position in the array
    if (i != 0) 
    {
      try
      {
      layer1points.add(float(i+10), ptData[i]);
      }catch(Exception e){logFile.println("Assigning Layer 1 Points");logFile.println("Error Output:" + e);}
      
    } else {
      layer1points.add(float(i), ptData[i]);
    }
  }

  //set the points to the graph for layer 1
  try
  {
  plot1.addPoints(layer1points, "layer 1");
  }catch(Exception e){logFile.println("Adding layerPoints to plot");logFile.println("Error Output:" + e);}
  
  
  //Swap previous data with new data
  switch(dataState)
  {
  case 1:
    //Stores Array 1 into Array 2
    try
    {
    layer2points.set(layer1points);
    dataState = 2;
    }catch(Exception e){logFile.println("Failed: Storing Array 1 points to Layer 2");logFile.println("Storing Array 1 points to Layer 2");logFile.println("Error Output:" + e);}
    break;

  case 2:
    try{
             
      //Stores Array 1 into Array 3
      layer3points.set(new GPointsArray(200));
      layer3points.set(layer1points);
       
      //updates layer 2 with Array 2
      plot1.addPoints(layer2points, "layer 2");
      dataState = 3;
      
    }catch(Exception e){logFile.println("Storing array 1 -> 3, display layer 3");logFile.println("Error Output:" + e);};
    break;
    
  case 3:
    try{
  
      //Stores Array 1 into Array 2
      layer2points.set(new GPointsArray(200));
      layer2points.set(layer1points);
      
      //updates layer 2 with Array 3
      plot1.addPoints(layer3points, "layer 2");
      dataState = 2;
      
    }catch(Exception e){logFile.println("Storing array 1->2, display layer 3");logFile.println("Error Output:" + e);};

    break;
  }
  
}

void drawGraph()
{
  //Draw the  plot
  plot1.beginDraw();
  
  
  
  //*************************************************
  // Start update background color
  if (largestNumber >= highVal)
  {
    //update plot background to red
    plot1.setBoxBgColor(#fbc8b4);
  } else if (largestNumber >=lowVal && largestNumber < highVal)
  {
    //update color to orange
    plot1.setBoxBgColor(#fed9a5);
  } else if (largestNumber < lowVal)
  {
    //update graph to green
    plot1.setBoxBgColor(#cfe6bf);
  }
  //*************************************************
  //End update background color


  //*************************************************
  // Start draw chart
  try {
    plot1.drawBackground();
    plot1.drawBox();
    plot1.drawGridLines(GPlot.HORIZONTAL);
  }
  catch(Exception e) {
    //println("Error At Drawing Background, Box, or Gridlines...");
    logFile.println("Error At Drawing Background, Box, or Gridlines...");
    logFile.println("Error Output:" + e);
  }
  //*************************************************
  // End draw chart
  
  
  //*************************************************
  // Start draw layer 2 (last attempt's data)
  
  try {    
    //graph needs to be drawn from back to front
    if(layer2points.getNPoints() == 200)
    {
    plot1.getLayer("layer 2").drawLines();
    plot1.getLayer("layer 2").drawPoints();
    }
  }
  
  catch(Exception e) {
    logFile.println("Error Output:" + e);
    logFile.println("Error At Layer 2 Drawlines");
    //print("Error At Layer 2 Drawlines");
    //println(e);
   
    logFile.print("Data State: ");
    logFile.println(dataState);
    
    logFile.print("array Size:");
    logFile.println(layer2points.getNPoints());
    
    //plot1.getLayer("layer 2").drawLines();
    //plot1.getLayer("layer 2").drawPoints();
    try{
    plot1.getLayer("layer 2").drawLines();
    plot1.getLayer("layer 2").drawPoints();
    }catch(Exception f){logFile.println("failed on 2nd attempt");exit();}
    
  }
  
  //*************************************************
  // End draw layer 2 (last attempt's data)
    
  //*************************************************
  // Start draw layer 1 (current data)
  
  try {
    plot1.getLayer("layer 1").drawLines();
    plot1.getLayer("layer 1").drawPoints();
      }
  catch(Exception e) {  
    logFile.println("Error Output:" + e);
    logFile.println("Error At Layer 1 Drawlines or points");
    //println("Error At Layer 1 Drawlines or points");
        try{
    plot1.getLayer("layer 1").drawLines();
    plot1.getLayer("layer 1").drawPoints();
    }catch(Exception f){logFile.println("failed on 2nd attempt"); exit();}
  }
  //*************************************************
  // End draw layer 1 (current data)
  
  //update images to match score
  imageUpdate();
  
  //end draw
  plot1.endDraw();  
}



void imageUpdate()
{
  if (largestNumber >= highVal)
  {
    imageMode(CENTER);
    //applys no dimming effect
    //this needs to be added in front of any image
    //50% or none or it overrides the next image
    tint(255, 255);
    image(en3, 1725, 350, enSize, enSize);
    imageMode(CENTER);
    //applys 50% dimming effect
    tint(255, 128);
    image(en2, 1725, 600, disSize, disSize);
    imageMode(CENTER);
    tint(255, 128);
    image(en1, 1725, 850, disSize, disSize);
  } else if (largestNumber >=lowVal && largestNumber < highVal)
  {
    imageMode(CENTER);
    tint(255, 128);
    image(en3, 1725, 350, disSize, disSize);
    imageMode(CENTER);
    tint(255, 255);
    image(en2, 1725, 600, enSize, enSize);
    imageMode(CENTER);
    tint(255, 128);
    image(en1, 1725, 850, disSize, disSize);
  } else if (largestNumber <lowVal)
  {
    imageMode(CENTER);
    tint(255, 128);
    image(en3, 1725, 350, disSize, disSize);
    imageMode(CENTER);
    tint(255, 128);
    image(en2, 1725, 600, disSize, disSize);
    imageMode(CENTER);
    tint(255, 255);
    image(en1, 1725, 850, enSize, enSize);
  }
}

void updateTitle()
{
  //this handles all the title updating functions.
  //it should scale automatically based on the varibles at the top
  textSize(70.5);
  fill(#275daa);
  //Sets the postion and Text of the start of the title
  text(spanish, titleX, titleY);
  fill(#000000);
  text(slash, textWidth(spanish) + titleX, titleY);
  fill(#004d43);
  //Adds the next section based on the texts width
  text(english, titleX + textWidth(spanish + slash), titleY);
  //This compounds the widiths are start location together
  float titleWidth = titleX + textWidth(spanish) + textWidth(english);
  fill(#000000);
  textSize(60);
  //this finds the largest number in the set and updates it
  String tempSen = " " + str(round(largestNumber));
  text(tempSen, titleWidth, titleY);
  textSize(35.25);
  //reads the largest numbers and adds the units after
  //There is some sort of bug that doesn't perfectly slide it over
  //there is an extra offest that only happens in this section to align the
  //unit correctly
  float senOffset = 50.00;
  float senWidth = (textWidth(tempSen) +  titleWidth + senOffset);
  text(units1, senWidth, titleY);
}

void updateArrow() {
  //find position of most recent plot point and update arrow
  float Xcord = 1550 ;
  float Ycord = map(largestNumber, graphYmin, graphYmax, 970, 218);

  if (largestNumber >= highVal)
  {
    imageMode(CENTER);
    tint(255, 255);
    image(rArrow, Xcord, Ycord, 50, 50);
  } else if (largestNumber >=lowVal && largestNumber < highVal)
  {
    imageMode(CENTER);
    tint(255, 255);
    image(yArrow, Xcord, Ycord, 50, 50);
  } else if (largestNumber <lowVal)
  {
    imageMode(CENTER);
    tint(255, 255);
    image(gArrow, Xcord, Ycord, 50, 50);
  }
}

void clearLayer() {
  //plot1.beginDraw();
  //clear layer 1 before sending new data
  plot1.setPoints(layer1points, "layer 1");
  plot1.setPoints(new GPointsArray(), "layer 2");
  plot1.drawBackground();
  plot1.drawBox();
  plot1.drawGridLines(GPlot.HORIZONTAL);
  plot1.getLayer("layer 2").drawLines();
  plot1.getLayer("layer 2").drawPoints();
  plot1.getLayer("layer 1").drawLines();
  plot1.getLayer("layer 1").drawPoints();
 // plot1.endDraw();
}

void playMovie() {
  //println("Play movie");
  if (attractor.time() >= (attractor.duration()- videoOffset)) {
    //quick pause to update the start position
    attractor.pause();
    attractor.jump(0);
    image(attractor, width/2, height/2);
    attractor.play();
  } else {
    image(attractor, width/2, height/2);
    attractor.play();
  }
}

void movieEvent(Movie m) {
  try {
    m.read();
  }
  catch(Exception e) {
    logFile.println("Error Output:" + e);
    logFile.println("Video Fail");
  }
}

void timerTick() {  
  //println("Timer reset");
  Timer.reset(); 
  ardPort.clear();
  ardPort.write(37);
  //println("sent 37");
  ardPort.write(10);
  gameOn = false;
}

void ardReset() {
  //println("Arduino Reset");
  try {
    ardPort.write(37);
    //println("sent 37");
    ardPort.write(10);
  }
  catch(Exception e) {
    logFile.println("Error Output:" + e);
    logFile.println("Failed to send Reset Signal");
  }

  //println("Set game to false");
  gameOn = false;
  logFile.println("Arduino Reset finished");
 }

//

void playsound()
{

  if (largestNumber >= highVal)
  {
    red.play();
  } else if (largestNumber >=lowVal && largestNumber < highVal)
  {
    orange.play();
  } else if (largestNumber <lowVal)
  {
    green.play();
  }
}

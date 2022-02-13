//Libraries
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
//Headers
#include "credentials.h"
#include "SpotifyClient.h"

// Serial management
char message = '0';

const char* host = "api.spotify.com";
const int httpsPort = 443;

SpotifyClient client(clientID, clientSecret, redirectUri);
SpotifyAuth auth;


String formatTime(uint32_t time);
void saveRefreshToken(String refreshToken);
String loadRefreshToken();

void setup(){
  Serial.begin(115200);

  boolean mounted = SPIFFS.begin();

  if (!mounted) {
    Serial.println("FS not formatted. Doing that now");
    SPIFFS.format();
    Serial.println("FS formatted...");
    SPIFFS.begin();
  }

  // Initialize Wifi
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // MDNS
  if (!MDNS.begin(espotifierNodeName)) {             // Start the mDNS responder for espotify.home
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  // Initialize Spotify
  String code = "";
  String grantType = "";
  String refreshToken = loadRefreshToken();
  if (refreshToken == "") {
    Serial.println("No refresh token found. Requesting through browser");
  
    Serial.println ( "Open browser at http://" + espotifierNodeName + ".home" );

    code = client.startConfigPortal();
    grantType = "authorization_code";
  } else {
    Serial.println("Using refresh token found on the FS");
    code = refreshToken;
    grantType = "refresh_token";
  }
  client.getToken(&auth, grantType, code);
  Serial.printf("Refresh token: %s\nAccess Token: %s\n", auth.refreshToken.c_str(), auth.accessToken.c_str());
  if (auth.refreshToken != "") {
    saveRefreshToken(auth.refreshToken);
  }
}

void loop() {
    if(Serial.available() > 0){
      message = Serial.read();
//      Serial.print("I received: ");
//      Serial.println(message);
      if(message == '1' || message == '2' || message == '3' || message == '4'){
        serialPlayerInput(message);
      }
    }

//    if (millis() - lastUpdate > 1000) {
//      uint16_t responseCode = client.update(&data, &auth);
//      Serial.printf("HREF: %s\n", data.image300Href.c_str());
//      lastUpdate = millis();
//      Serial.printf("--------Response Code: %d\n", responseCode);
//      Serial.printf("--------Free mem: %d\n", ESP.getFreeHeap());
//      if (responseCode == 401) {
//        client.getToken(&auth, "refresh_token", auth.refreshToken);
//        if (auth.refreshToken != "") {
//          saveRefreshToken(auth.refreshToken);
//        }
//      }
//      if (responseCode == 200) {
//
//
//        String selectedImageHref = data.image300Href;
//        selectedImageHref.replace("https", "http");
//        
//        
//        if (selectedImageHref != currentImageUrl) {
//
//          Serial.println("New Image. Downloading it");
//
//          isDownloadingCover = true;
//          client.downloadFile(selectedImageHref, "/cover.jpg");
//          isDownloadingCover = false;
//          currentImageUrl = selectedImageHref;
//          drawJPEGFromSpiffs("/cover.jpg", 45, 0);
//          gfx.setColor(ILI9341_YELLOW);
//        }
// 
//      }
//      if (responseCode == 400) {
//        gfx.fillBuffer(MINI_BLACK);
//        gfx.setColor(MINI_WHITE);
//        gfx.setTextAlignment(TEXT_ALIGN_CENTER);
//        gfx.drawString(120, 20, "Please define\nclientId and clientSecret");
//        gfx.commit(0,160);
//      }
//
//    }
//    drawSongInfo();
//    
//    if (touchController.isTouched(500) && millis() - lastTouchMillis > 1000) {
//      TS_Point p = touchController.getPoint();
//      lastTouchPoint = p;
//      lastTouchMillis = millis();
//      String command = "";
//      String method = "";
//      if (p.y > 160) {
//        if (p.x < 80) {
//          method = "POST"; 
//          command = "previous";
//        } else if (p.x >= 80 && p.x <= 160) {
//          method = "PUT"; 
//          command = "play";
//          if (data.isPlaying) {
//            command = "pause";
//          }
//          data.isPlaying = !data.isPlaying;   
//        } else if (p.x > 160) {
//          method = "POST"; 
//          command = "next";
//        }
//        uint16_t responseCode = client.playerCommand(&auth, method, command);
//      Serial.print("playerCommand response =");
//      Serial.println(responseCode);
//    }
//  }
}

void serialPlayerInput(char message){
  String command = "";
  String method = "";
  if(message == '1'){ // Previous
    method = "POST";
    command = "previous";
    Serial.println("Previous");
  }else if(message == '2'){ // pause
    method = "PUT";
    command = "pause";
    Serial.println("Pause");
  }else if(message == '3'){ // resume
    method = "PUT";
    command = "play";
    Serial.println("Resume");
  }else if(message == '4'){ // next
    method = "POST";
    command = "next";
    Serial.println("Next");
  }else{
    Serial.println("unrecognised Command");
    message = '0';
    return;
  }
  uint16_t responseCode = client.playerCommand(&auth,method,command);
  Serial.printf("playerCommand res = %d\n",responseCode);
  message = '0';
}

String formatTime(uint32_t time) {
  char time_str[10];
  uint8_t minutes = time / (1000 * 60);
  uint8_t seconds = (time / 1000) % 60;
  sprintf(time_str, "%2d:%02d", minutes, seconds);
  return String(time_str);
}

void saveRefreshToken(String refreshToken) {
  
  File f = SPIFFS.open("/refreshToken.txt", "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return;
  }
  f.println(refreshToken);
  f.close();
}

String loadRefreshToken() {
  Serial.println("Loading config");
  File f = SPIFFS.open("/refreshToken.txt", "r");
  if (!f) {
    Serial.println("Failed to open config file");
    return "";
  }
  while(f.available()) {
      //Lets read line by line from the file
      String token = f.readStringUntil('\r');
      Serial.printf("Refresh Token: %s\n", token.c_str());
      f.close();
      return token;
  }
  return "";
}

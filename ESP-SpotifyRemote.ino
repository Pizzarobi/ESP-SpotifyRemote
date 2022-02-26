//Libraries
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
//Headers
#include "credentials.h" // Not included in Github repo. Template as credentials.txt
#include "SpotifyClient.h"

// Spotify Playlists
String espTestPlaylist = "0zvFsU9gkh4yWOHYs8zM71";

char message = '0';

const char* host = "api.spotify.com";
const int httpsPort = 443;

SpotifyClient client(clientID, clientSecret, redirectUri);
SpotifyData data;
SpotifyAuth auth;

long lastUpdate = 0;

String formatTime(uint32_t time);
void saveRefreshToken(String refreshToken);
String loadRefreshToken();
void clearRefreshToken();
//void serialPlayerInput(char message);

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
  if (!MDNS.begin(espotifierNodeName)) {             // Start the mDNS responder for espotify.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  // Initialize Spotify
  String code = "";
  String grantType = "";
  String refreshToken = loadRefreshToken();
  if (refreshToken == "") {
    Serial.println("No refresh token found. Requesting through browser");
  
    Serial.println ( "Open browser at http://" + espotifierNodeName + ".local" );

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
  }
}

void loop() {
  if(Serial.available() > 0){
    message = Serial.read();
    if(message == '1' || message == '2' || message == '3' || message == '4' || message == '5' || message == '9'){
      serialPlayerInput(message);
    }
  }

  if (millis() - lastUpdate > 10000) {
    uint16_t responseCode = client.update(&data, &auth);
    lastUpdate = millis();
    Serial.printf("--------Response Code: %d\n", responseCode);
    Serial.printf("Data: isPlaying: %d, isPlayerActive %d,\nsongUri: %s,\nplaylistUri: %s\n",data.isPlaying,data.isPlayerActive,data.songUri.c_str(),data.playlistUri.c_str());
    Serial.printf("--------Free mem: %d\n\n", ESP.getFreeHeap());
    if (responseCode == 401) {
      client.getToken(&auth, "refresh_token", auth.refreshToken);
      if (auth.refreshToken != "") {
        saveRefreshToken(auth.refreshToken);
      }
    }
  }
}

// Control ESP and Spotify using Serial input. Used for testing
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
  }else if(message == '5'){ // addSongToPlaylist
    uint16_t responseCode = client.addSongToPlaylist(&auth,data.songUri,espTestPlaylist);
    Serial.printf("Resonse Code: %d\n\n",responseCode);
    return;
  }else if(message == '9'){ // clearToken
    clearRefreshToken();
    Serial.println("Cleared refreshtoken from memory");
    return;
  }else{
    Serial.println("unrecognised Command");
    message = '0';
    return;
  }
  uint16_t responseCode = client.playerCommand(&auth,method,command);
  Serial.printf("playerCommand res = %d\n",responseCode);
  message = '0';
}

//
void getCurrentPlaying() {
  uint16_t responseCode = client.update(&data,&auth);
}

// clears token in memory
void clearRefreshToken(){
  File f = SPIFFS.open("/refreshToken.txt", "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return;
  }
  f.println("");
  f.close();
}

// saves refreshtoken in memory
void saveRefreshToken(String refreshToken) {
  
  File f = SPIFFS.open("/refreshToken.txt", "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return;
  }
  f.println(refreshToken);
  f.close();
}

// loads refreshtoken from memory
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

String formatTime(uint32_t time) {
  char time_str[10];
  uint8_t minutes = time / (1000 * 60);
  uint8_t seconds = (time / 1000) % 60;
  sprintf(time_str, "%2d:%02d", minutes, seconds);
  return String(time_str);
}

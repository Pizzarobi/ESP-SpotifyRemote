/**The MIT License (MIT)
 
 Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once
#include <Arduino.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <base64.h>

typedef void (*DrawingCallback)();

typedef struct SpotifyAuth {
  // "access_token": "XXX"
  String accessToken;
   // "token_type":"Bearer",
  String tokenType;
  // "expires_in":3600,
  uint16_t expiresIn;
  // "refresh_token":"XX",
  String refreshToken;
  // "scope":"user-modify-playback-state user-read-playback-state user-read-currently-playing user-read-private
  String scope;

} SpotifyAuth;

class SpotifyClient: public JsonListener {
  private:

  String currentKey;
  String currentParent;
  SpotifyAuth *auth;
  bool isDataCall;
  String rootPath[10];
  uint8_t level = 0;
  uint16_t currentImageHeight;
  DrawingCallback *drawingCallback;
  String clientId;
  String clientSecret;
  String redirectUri;
  ESP8266WebServer server;

  String getRootPath();
  void executeCallback();

  public:
    SpotifyClient(String clientId, String clientSecret, String redirectUri);

    uint16_t playerCommand(SpotifyAuth *auth, String method, String command);

    void getToken(SpotifyAuth *auth, String grantType, String code);

    void downloadFile(String url, String filename);

    void setDrawingCallback(DrawingCallback *drawingCallback) {
      this->drawingCallback = drawingCallback;
    }

    String startConfigPortal();



    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();

    
};

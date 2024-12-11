// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL4AnEFpL5p"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "qEjFtvtSFtEUyFgUdcyLCil66z_tXCc_"

// WiFi credentials

#define WIFI_SSID "Mostafa's Galaxy A52";   // Your WiFi SSID
#define WIFI_PASS "knfm5166";               // Your WiFi password

//WiFi credentials
struct WifiCredentials{
  String ssid;                  // Your WiFi SSID
  String pass;                  // Your WiFi password

  /*
  * Used to determine after boot that, if credential data in storage is valid.  
  */
  bool confEmpty() {
    return (!ssid.isEmpty() && !pass.isEmpty());
  }
  bool valid = false;           
};
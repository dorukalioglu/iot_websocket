#include <EtherCard.h>
#include <EEPROM.h>
#define STATIC 1

uint8_t reset = 0;
uint8_t staticip = 0;
static byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};

byte Ethernet::buffer[500]; // tcp/ip send and receive buffer
BufferFiller bfill;
void (*resetFunc)(void) = 0;
void setup()
{
  Serial.begin(9600);
  Serial.println("\n[backSoon]");
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println("Failed to access Ethernet controller");

  Serial.print("staticip");
  Serial.println(staticip);

  if (EEPROM.read(6) == 1)
  {
    static byte myip[] = {EEPROM.read(7), EEPROM.read(8), EEPROM.read(9), EEPROM.read(10)};
    if (EEPROM.read(11) == 1)
    {
      static byte mask[] = {EEPROM.read(12), EEPROM.read(13), EEPROM.read(14), EEPROM.read(15)};
      if (EEPROM.read(16) == 1)
      {
        static byte gwip[] = {EEPROM.read(17), EEPROM.read(18), EEPROM.read(19), EEPROM.read(20)};
        
        ether.staticSetup(myip, gwip, NULL, mask);
      }
      else
      {
        static byte gwip[] = {EEPROM.read(7), EEPROM.read(8), EEPROM.read(9), 1};

        ether.staticSetup(myip, gwip, NULL, mask);
      }

      staticip = 1;
    }
  }
  if (staticip != 1)
    if (!ether.dhcpSetup())
      Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  digitalWrite(9, HIGH);
}

const char http_OK[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
    "HTTP/1.0 302 Found\r\n"
    "Location: /\r\n\r\n";

void homePage()
{

  bfill.emit_p(PSTR("$F"
                    "<title>MachIoT</title>"
                    "<form methon='GET' action=''>"
                    "<label>Server IP</label><br/><input name='sw'><br/><br/>"
                    "<label>Device IP</label><br/><input name='ip'><br/><br/>"
                    "<label>Device MASK</label><br/><input name='mask'><br/><br/>"
                    "<label>Device GATEWAY</label><br/><input name='gw'><br/>"
                    "<input type='submit' value='Submit'>"
                    "</form>"),
               http_OK);
}

void loop()
{
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (pos)
  {

    delay(1); // necessary for my system
    bfill = ether.tcpOffset();
    char *data = (char *)Ethernet::buffer + pos;
    if (strncmp("GET /", data, 5) == 0)
    {
      data += 5;
      if (data[0] == ' ')
      {
        homePage();
      }
      else if (strncmp("?sw=", data, 4) == 0)
      {

        eepromWrite(data);

        homePage();
        //reset=1;
      }
    }

    ether.httpServerReply(bfill.position()); // send http response
  }
  if (reset)
  {
    delay(10);
    resetFunc();
  }
}
void eepromWrite(String data)
{
  String arr[] = {"?sw=", "&ip=", "&mask=", "&gw=", " HTTP"};
  uint8_t e = 1;
  for (uint8_t y = 0; y < 4; y++)
  {
    String str = data.substring(data.indexOf(arr[y]) + arr[y].length(), data.indexOf(arr[y + 1]));
    String s = "";
    EEPROM.write(e, 1);
    e++;
    for (uint8_t x = 0; x <= str.length(); x++)
    {
      if (str[x] == '.' || x == str.length())
      {
        if (s.length() > 0)
        {
          EEPROM.write(e, s.toInt());
          e++;
          s = "";
        }
      }
      else
      {
        s += str[x];
      }
    }
    Serial.println("");
  }
  reset=1;
}
void resetIP(){
    EEPROM.write(1, 0);
  }

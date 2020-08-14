#include <EtherCard.h>
#include <EEPROM.h>
#define STATIC 1
uint8_t swconnect = 0;
word pos = 0;
word len = 0;
byte dns[] = {8, 8, 8, 8};
int port = 5000;
uint8_t reset = 0;
static byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};
String arr[] = {"?sw=", "&ip=", "&mask=", "&gw=", "&key=", " HTTP"};
byte Ethernet::buffer[750]; // tcp/ip send and receive buffer
BufferFiller bfill;
void (*resetFunc)(void) = 0;
void setup()
{
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println("Failed to access Ethernet controller");

  if (EEPROM.read(6) == 1 && EEPROM.read(22) == 1)
  {
    static byte myip[] = {EEPROM.read(7), EEPROM.read(8), EEPROM.read(9), EEPROM.read(10)};
    if (EEPROM.read(11) == 1)
    {
      static byte mask[] = {EEPROM.read(12), EEPROM.read(13), EEPROM.read(14), EEPROM.read(15)};
      if (EEPROM.read(16) == 1)
      {
        static byte gwip[] = {EEPROM.read(17), EEPROM.read(18), EEPROM.read(19), EEPROM.read(20)};

        ether.staticSetup(myip, gwip, dns, mask);
      }
      else
      {
        static byte gwip[] = {EEPROM.read(7), EEPROM.read(8), EEPROM.read(9), 1};

        ether.staticSetup(myip, gwip, dns, mask);
        digitalWrite(9, HIGH);
      }
    }
  }
  else if (EEPROM.read(21) == 1 && EEPROM.read(22) == 1)
  {
    if (!ether.dhcpSetup())
      Serial.println("DHCP failed");
    else
      digitalWrite(9, HIGH);
  }
  else
  {
    byte a[] = {10, 34, 1, 138};
    byte b[] = {10, 34, 1, 1};
    byte d[] = {255, 255, 255, 0};
    ether.staticSetup(a, b, dns, d);
    digitalWrite(9, HIGH);
  }
  if (EEPROM.read(1) == 1)
  {
    byte hisip[] = {EEPROM.read(2), EEPROM.read(3), EEPROM.read(4), EEPROM.read(5)};
    ether.copyIp(ether.hisip, hisip);
    ether.hisport = port;
  }
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  ether.printIp("SRV: ", ether.hisip);

  if (EEPROM.read(23) != 1)
  {
    EEPROM.write(22, 0);
  }
}

const char http_OK[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n\r\n";

void homePage()
{
  bfill.emit_p(PSTR("$F"
                    "<title>MachIoT</title>"
                    "<form methon='GET'>"
                    "DHCP<br><input type='checkbox' name='dhcp' ><br>"
                    "Server<br><input name='sw'><br>"
                    "IP<br><input name='ip'><br>"
                    "MASK<br><input name='mask'><br>"
                    "GATEWAY<br><input name='gw'><br>"
                    "Key<br><input name='key'><br>"
                    "<input type='submit' value='Submit'>"
                    "</form>"),
               http_OK);
}
void resetpage()
{
  bfill.emit_p(PSTR("$F"
                    "<title>MachIoT</title>"
                    "Restarting.."),
               http_OK);
}
void wkey()
{

  bfill.emit_p(PSTR("$F"
                    "<title>MachIoT</title>"
                    "Wrong Key"),
               http_OK);
}
void swres()
{

  bfill.emit_p(PSTR("$F"
                    "<title>MachIoT</title>"
                    "{'success':true}"),
               http_OK);
}

void loop()
{
  // requestler saniyelik

  len = ether.packetReceive();
  pos = ether.packetLoop(len);

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
      }
      else if (strncmp("?dhcp=on=", data, 8) == 0)
      {
        dhcpon(data);
      }
      else if (strncmp("?swconnect=1", data, 12) == 0)
      {
        if (EEPROM.read(23) != 1)
        {
          EEPROM.write(23, 1);
          if (EEPROM.read(22) != 1)
          {
            EEPROM.write(22, 1);
          }
        }
      }
    }

    ether.httpServerReply(bfill.position());
  }

  if (reset)
  {
    delay(10);
    resetFunc();
  }
}
void dhcpon(String data)
{
  if (data.substring(data.indexOf(arr[4]) + arr[4].length(), data.indexOf(arr[5])) == "123m123M")
  {
    EEPROM.write(21, 1);
    reset = 1;
    resetpage();
  }
}
void eepromWrite(String data)
{

  uint8_t e = 1;
  Serial.println(data.indexOf(arr[4]));
  if (data.substring(data.indexOf(arr[4]) + arr[4].length(), data.indexOf(arr[5])) == "123m123M")
  {
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
      EEPROM.write(21, 0); //dhscp
      EEPROM.write(22, 1); //ilk ip atama reseti
    }
    reset = 1;
    resetpage();
  }
  else
  {
    wkey();
  }
}
void resetIP()
{
  EEPROM.write(1, 0);
}

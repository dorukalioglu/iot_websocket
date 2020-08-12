#include <EtherCard.h>
#include <EEPROM.h> // allow settings to save to flash
#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)

#if STATIC
// ethernet interface ip address
static byte myip[] = { 10,34,1,135 };
// gateway ip address
static byte gwip[] = { 10,34,1,1 };
#endif

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };



byte Ethernet::buffer[500]; // tcp/ip send and receive buffer
BufferFiller bfill;

void setup(){
  Serial.begin(9600);
  Serial.println("\n[backSoon]");
 
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip); 
  ether.printIp("DNS: ", ether.dnsip); 
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
        "<title>Ethercard LED</title>"
        "<form methon='GET' action=''><input name='ip'> <input type='submit' value='Submit'></form>"),
        http_OK);
       
}

void loop(){

   word len = ether.packetReceive();
   word pos = ether.packetLoop(len);
  if (pos) {

   
    delay(1);   // necessary for my system
        bfill = ether.tcpOffset();
        char *data = (char *) Ethernet::buffer + pos;
    if (strncmp("GET /", data, 5) == 0) {
        data += 5;
           if (data[0] == ' ' ) {
                // Return home page
                homePage();
            }
            else if (strncmp("?ip=", data,4) == 0) {
             uint8_t x=0;
              while(strncmp(" HTTP", data+x,5) != 0){
                x++;
              }
                String a=data+4;
                 Serial.println(a.substring(0,x-4));
             homePage();
            }

        }
       
        ether.httpServerReply(bfill.position());    // send http response
    }
  
  }

// =================================== ======= === = == =  =  = -- - -
// Minimalistic Wifi Packet Forwarder using ESP8266 Wifi Transceiver v1.0
//
// This is *not* a traditional router. This is a packet forwarder with
// a simple binary protocol to send and receive packets between
// connected devices using the ESP8266 as intermediary.
//
// It can handle up to 253 connected devices, although the limit is
// currently set to only 5 in the code ahead. You can change that limit
// easily just by setting the amount on the MAX_SLOTS define.
//
// The protocol is as follows:
//
// On first sent packet, a device will identify itself with a single-byte
// packet to the Forwarder telling which device index it is going to use.
// Each connected device must have a unique index from 1-254
// (index 0 = the Forwarder itself).
//
// If a device wants to have its index automatically assigned, it should
// then specify its index as 255 on the first packet.
//
// The Forwarder will always answer the first packet with a new
// single-byte packet back to the device containing its assigned index,
// or a single-byte packet with 255 if no index was assigned (index
// collision or not available slots).
//
// After having been identified on the network, a device can start
// sending and receiving packets.
//
// To send packets to another device, a device sends then a packet with
// the following format:
//
// [SRC][TGT][PKT]
//
// [SRC]: 1 byte - tells which device is contacting the Forwarder.
// [TGT]: 1 byte - target device to receive the packet.
// [PKT]: variable width - the packet itself.
//
// The Forwarder will then send the very same packet (including the two
// header bytes) to the target device identified by that TGT index.
//
// There is also a simple LED feedback to know when it's in AP (slow blinking)
// and Station mode (faster blinking). When it receives packets, the LED also
// is kept on for a few seconds.
//
// On my own tests, I'm getting about 10mbps throughput with this forwarder.
//
// Written by Vander 'imerso' Nunes | imersiva.com
// ======================================= ======= === == == =  =  = -- - -

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WebConfig.h>
#include "Slot.h"

//#define DEBUG
#define MAX_SLOTS 5

#define LED_PIN 2
#define BUFFERSIZE 10240

bool isLEDOn = false;
long lastBlink = 0;

WebConfig* pWebConfig;
WiFiUDP* pUdp;
bool isAP;
char buffer[BUFFERSIZE];

Slot slots[MAX_SLOTS];


// Initialize the system
void setup()
{
  // start serial monitor
  Serial.begin(57600);

  // initialize led GPIO
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // start with led on while initializing

  pWebConfig = new WebConfig("FORWARDER v1.0", "AVANTROUTER", "8266", false);
  pUdp = pWebConfig->UDP();
  isAP = pWebConfig->IsAP();

  // slot 0 is always used by the Forwarder itself.
  slots[0].Use(WiFi.localIP(), 0);

  // turn led off so we know it finished initialization
  digitalWrite(LED_PIN, LOW);
}


// Serve HTTP configuration interface,
// And manage connected devices.
void loop()
{
  ProcessUDP();
  pWebConfig->ProcessHTTP();
  Blink();

  yield();
}


// Forward UDP packets
void ProcessUDP()
{
  if (!pUdp) return;

  int len = pUdp->parsePacket();
  if (len)
  {
    // has udp packet
    IPAddress remoteIP = pUdp->remoteIP();
    int remotePort = pUdp->remotePort();

    // read the packet
    pUdp->read(buffer, BUFFERSIZE);

    #ifdef DEBUG
      Serial.println(String("Packet [") + String(len) + String("] from ") + String(remoteIP) + String(":"));
      String s = "";
      for (int i=0; i<len; i++)
      {
        s += String(buffer[i], HEX) + " ";
      }
      Serial.println(s);
    #endif

    if (len == 1)
    {
      // assume that this is a device identifying itself.
      int pktlen = 1;

      // check what is the requested SRC device index
      byte src = buffer[0];

      if (src == 0)
      {
        // cannot use slot 0, it's for the Forwarder itself.
        buffer[0] = 255;

        #ifdef DEBUG
          Serial.println("Slot 0 - no slot assigned.");
        #endif
      }
      else if (src == 255)
      {
        // device wants auto-assigned slot index
        for (byte i=0; i<MAX_SLOTS; i++)
        {
          if (!slots[i].used)
          {
            // found a free slot
            slots[i].Use(remoteIP, remotePort);
            buffer[0] = i;

            #ifdef DEBUG
              Serial.println(String("Auto-assigned slot ") + String(i));
            #endif

            break;
          }
        }

        // could not find any free slots for auto-assigning
        // send an "unassigned" answer packet.
        buffer[0] = 255;

        #ifdef DEBUG
          Serial.println("No slot assigned.");
        #endif
      }
      else
      {
        // device is requesting a specific slot index
        if (!slots[src].used || String(remoteIP) == String(slots[src].ip))
        {
          // slot is free, take it
          slots[src].Use(remoteIP, remotePort);
          buffer[0] = src;

          #ifdef DEBUG
            Serial.println(String("Assigned slot ") + String(src));
          #endif
        }
        else
        {
          // slot is occupied, refuse to take
          // send an "unassigned" answer packet.
          buffer[0] = 255;

          #ifdef DEBUG
            Serial.println("No slot assigned.");
          #endif
        }
      }

      // deliver answer
      pUdp->beginPacket(remoteIP, remotePort);
      pUdp->write(buffer, pktlen);
      pUdp->endPacket();

      #ifdef DEBUG
        Serial.println(String("Answered to ") + String(remoteIP) + String(":") + String(remotePort));
        s = "";
        for (int i=0; i<pktlen; i++)
        {
          s += String(buffer[i], HEX) + " ";
        }
        Serial.println(s);
      #endif
    }
    else if (len >= 3)
    {
      // a packet to be forwarded
      byte src = buffer[0];
      byte tgt = buffer[1];

      // check if both slots are used
      if (src >= MAX_SLOTS || !slots[src].used || tgt >= MAX_SLOTS || !slots[tgt].used)
      {
        // bad packet
        #ifdef DEBUG
          Serial.println("bad packet.");
        #endif
        return;
      }

      // check if the SRC is actually from the registered IP
      if (String(remoteIP) != String(slots[src].ip))
      {
        // not the same IP, someone trying to hack?
        // for now, just refuse the packet...
        #ifdef DEBUG
          Serial.println("hack attempt!");
        #endif
        return;
      }

      if (tgt == 0)
      {
          // message is for the Forwarder itself, just ignore it.
      }
      else
      {
        // everything seems good, forward the packet to the target.
        pUdp->beginPacket(slots[tgt].ip, slots[tgt].port);
        pUdp->write(buffer, len);
        pUdp->endPacket();

        #ifdef DEBUG
          Serial.println(String("Forwarded to ") + String(slots[tgt].ip) + String(":") + String(slots[tgt].port));
          s = "";
          for (int i=0; i<len; i++)
          {
            s += String(buffer[i], HEX) + " ";
          }
          Serial.println(s);
        #endif
      }
    }

    // flag received packet by keeping the LED on for a few seconds
    digitalWrite(LED_PIN, HIGH);
    lastBlink = millis() + 3000;
  }
}


// Blink LED at a different pattern
// depending on the operation mode
void Blink()
{
  long now = millis();

  if (isAP)
  {
    // keeps LED on shorter time than off
    if (now - lastBlink >= (isLEDOn ? 100 : 1500))
    {
      isLEDOn = !isLEDOn;
      digitalWrite(LED_PIN, (isLEDOn ? HIGH : LOW));
      lastBlink = now;
    }
  }
  else
  {
    // keeps LED on same time as off
    if (now - lastBlink >= 250)
    {
      isLEDOn = !isLEDOn;
      digitalWrite(LED_PIN, (isLEDOn ? HIGH : LOW));
      lastBlink = now;
    }
  }
}


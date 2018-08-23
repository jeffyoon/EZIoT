/*
** EZIoT - IOT Device: Over the Air Update Service
**
** Copyright (c) 2017,18 P.C.Monteith, GPL-3.0 License terms and conditions.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/
#include "iot.h"
#include <MD5Builder.h>
#include <Update.h>

#define INT_BUFFER_SIZE 16

using namespace EZ;

void IOT::otauCredentials(const char* password, uint16_t port)
{
    if ((password) && *password != '\0')
    {
        MD5Builder passmd5;
        passmd5.begin();
        passmd5.add(password);
        passmd5.calculate();
        _otauPASS.value(passmd5.toString());
    }
    else
        _otauPASS.value(password);
    _otauPort.native(port);
}

void IOT::_otauStart(void)
{
    mdns_txt_item_t arduTxtData[4] = {{(char*)"board", (char*)EZ_VARIANT},
                                      {(char*)"tcp_check", (char*)"no"},
                                      {(char*)"ssh_upload", (char*)"no"},
                                      {(char*)"auth_upload", (char*)"yes"}};

    if (_otauState == OTAU_STOPPED)
    {
        uint16_t port = _otauPort.native();

        if (port > 0)
        {
            if (_otauUDP.listen(port))
            {
                console.printf(LOG::INFO1, "OtAU: Listening on port: %u", port);

                _otauTimeout = 1000;
                _otauState = OTAU_WAITING;
                _otauUDP.onPacket(std::bind(&IOT::_otauRequest, this, std::placeholders::_1));

                mdnsService("_arduino", "_tcp", port, NULL, arduTxtData, 4);

                console.printf(LOG::INFO1, "OtAU: Started.");
                Update.end();
            }
        }
        else
            console.printf(LOG::INFO1, "OtAU: Disabled.");
    }
}

void IOT::_otauStop(void)
{
    if (_otauState != OTAU_STOPPED)
    {
        _otauUDP.close();
        _otauState = OTAU_STOPPED;
        mdnsRemove("_arduino", "_tcp");
    }
}

void IOT::_otauUpdate(void)
{
    if (_otauState != OTAU_UPDATING)
        return;

    if (!Update.begin(_otauSize, _otauCmd))
    {
        Update.printError(Serial);
        _otauState = OTAU_WAITING;
        return;
    }

    Update.setMD5(_otauMD5.c_str());
    // Update.onProgress(std::bind(&IOT::_otauProgress, this, std::placeholders::_1, std::placeholders::_2));
    console.printf(LOG::INFO1, "OtAU: Connect to client(%s:%u).", _otauClientAddr.toString().c_str(), _otauClientPort);

    WiFiClient client;

    if (!client.connect(_otauClientAddr, _otauClientPort))
    {
        console.printf(LOG::ERROR, "OtAU: Failed, no client connection!");
        Update.abort();
        _otauState = OTAU_WAITING;
        _otauClientAddr = (uint32_t)0;
        _otauClientPort = 0;
        return;
    }

    if (!EZ_IOT_MUTEX_TAKE())
    {
        _otauState = OTAU_WAITING;
        return;
    }

    console.printf(LOG::TITLE, "** SYSTEM UPDATE STARTED **");
    uint32_t written = 0, totaul = 0, tried = 0;
    xEventGroupSetBits(_eventGroup, OTAU_BIT);

    while (!Update.isFinished() && client.connected() && _otauState == OTAU_UPDATING)
    {
        size_t waited = _otauTimeout;
        size_t available = client.available();

        while (!available && waited)
        {
            delay(1);
            waited -= 1;
            available = client.available();
        }

        if (!waited)
        {
            if (written && tried++ < 3)
            {
                ESP_LOGD(iotTag, "OtAU: Try[%u]: %u\n", tried, written);

                if (!client.printf("%u", written))
                {
                    console.printf(LOG::ERROR, "OtAU: Client failed to respond!");
                    _otauState = OTAU_WAITING;
                    break;
                }

                continue;
            }

            console.printf(LOG::ERROR, "OtAU: Receive failed!");
            _otauState = OTAU_WAITING;
            break;
        }

        if (!available)
        {
            ESP_LOGD(iotTag, "OtAU: No Data: %u\n", waited);
            _otauState = OTAU_WAITING;
            break;
        }

        tried = 0;
        static uint8_t buf[1460];

        if (available > 1460)
        {
            available = 1460;
        }

        size_t r = client.read(buf, available);

        if (r != available)
        {
            console.printf(LOG::WARNING, "OtAU: Didn't read enough! %u != %u", r, available);
        }

        written = Update.write(buf, r);

        if (written > 0)
        {
            if (written != r)
            {
                console.printf(LOG::WARNING, "OtAU: Didn't write enough! %u != %u", written, r);
            }

            if (!client.printf("%u", written))
            {
                console.printf(LOG::ERROR, "OtAU: Client failed to respond!");
            }

            totaul += written;
            _otauProgress(totaul, _otauSize);
        }
        else
        {
            Update.printError(Serial);
        }
    }

    if (_otauState != OTAU_UPDATING)
        Update.abort();

    if (Update.end())
    {
        console.printf(LOG::TITLE, "** SYSTEM UPDATE SUCCESS **");
        client.print("OK");
        client.stop();
        _restart();
    }
    else
    {
        console.printf(LOG::TITLE, "** SYSTEM UPDATE FAILED **");
        Update.printError(client);
        client.stop();
        delay(10);
        Update.printError(Serial);
        _otauState = OTAU_WAITING;
    }

    xEventGroupClearBits(_eventGroup, OTAU_BIT);
    EZ_IOT_MUTEX_GIVE();
}

void IOT::_otauProgress(size_t progress, size_t totaul)
{
    console.printf(LOG::INFO1, "OtAU: Progress - %d/%d", progress, totaul);
}

void IOT::_otauRequest(AsyncUDPPacket packet)
{
    ESP_LOGD(iotTag, "OtAU: %s(%s:%u) Packet (Local:%d)",
             packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast",
             packet.remoteIP().toString().c_str(), packet.remotePort(), packet.localPort());

    if ((packet.localPort() != _otauPort.native()) || packet.isBroadcast() || packet.isMulticast())
        return;

    if (_otauState == OTAU_UPDATING)
    {
        ESP_LOGW(iotTag, "OtAU: Update already in progres!");
        packet.flush();
        return;
    }
    else if (_otauState == OTAU_WAITING)
    {
        _otauCmd = _otauParseInt(packet);

        if (_otauCmd != U_FLASH && _otauCmd != U_SPIFFS)
        {
            packet.flush();
            return;
        }

        _otauClientAddr = packet.remoteIP();
        _otauClientPort = _otauParseInt(packet);
        _otauSize = _otauParseInt(packet);
        packet.read();
        _otauMD5 = _otauReadStringUntil(packet, '\n');
        _otauMD5.trim();

        console.printf(LOG::INFO1, "OtAU: Remote(%s:%u) - Update %s, requested.", _otauClientAddr.toString().c_str(),
                       _otauClientPort, _otauCmd == U_FLASH ? "Flash" : "SPIFFS");

        if (_otauMD5.length() != 32)
        {
            packet.flush();
            return;
        }

        ESP_LOGD(iotTag, "OtAU: MD5=%s", _otauMD5.c_str());

        if (_otauPASS.value().length())
        {
            MD5Builder nonce_md5;
            nonce_md5.begin();
            nonce_md5.add(String(micros()));
            nonce_md5.calculate();
            _otauOnce = nonce_md5.toString();
            _otauState = OTAU_WAITAUTH;
            packet.printf("AUTH %s", _otauOnce.c_str());
            console.printf(LOG::INFO1, "OtAU: Sent authorisation challenge");
        }
        else
        {
            _otauState = OTAU_UPDATING;
            packet.print("OK");
        }
    }
    else if (_otauState == OTAU_WAITAUTH)
    {
        int cmd = _otauParseInt(packet);

        if (cmd != U_AUTH)
        {
            _otauState = OTAU_WAITING;
            packet.flush();
            return;
        }

        packet.read();

        String cnonce = _otauReadStringUntil(packet, ' ');
        String response = _otauReadStringUntil(packet, '\n');

        if (cnonce.length() != 32 || response.length() != 32)
        {
            _otauState = OTAU_WAITING;
            packet.flush();
            return;
        }

        String challenge = _otauPASS.value() + ":" + String(_otauOnce) + ":" + cnonce;

        MD5Builder _challengemd5;
        _challengemd5.begin();
        _challengemd5.add(challenge);
        _challengemd5.calculate();
        String result = _challengemd5.toString();

        if (result.equals(response))
        {
            _otauState = OTAU_UPDATING;
            packet.print("OK");
            packet.flush();
        }
        else
        {
            static const char* authFail = "Authentication Failed";
            _otauState = OTAU_WAITING;
            packet.print(authFail);
            console.printf(LOG::ERROR, "OtAU: %s", authFail);
        }
    }

    // always flush, even zero length packets must be flushed.
    packet.flush();

    if (_otauState == OTAU_UPDATING)
    {
        _mdnsStop();
        _ssdpStop();

        _otauUpdate();

        _mdnsStart();
        _ssdpStart();
    }
}

int IOT::_otauParseInt(AsyncUDPPacket& packet)
{
    char data[INT_BUFFER_SIZE];
    uint8_t index = 0;
    char value;

    while (packet.peek() == ' ')
        packet.read();

    while (index < INT_BUFFER_SIZE - 1)
    {
        value = packet.peek();

        if (value < '0' || value > '9')
        {
            data[index++] = '\0';
            return atoi(data);
        }

        data[index++] = packet.read();
    }
    return 0;
}

String IOT::_otauReadStringUntil(AsyncUDPPacket& packet, char end)
{
    String res = "";
    int value;

    while (true)
    {
        value = packet.read();

        if (value <= 0 || value == end)
        {
            return res;
        }

        res += (char)value;
    }
    return res;
}
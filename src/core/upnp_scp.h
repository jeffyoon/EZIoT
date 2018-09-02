/*
** EZIoT - UPNP Service Control Protocol Class
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
#if !defined(_UPNP_SCP_H)
#define _UPNP_SCP_H
#include "ez_common.h"
#include "ez_device.h"
#include "ez_http.h"
#include "ez_service.h"
#include "ez_variable.h"
#include "tool/ez_uuid.h"
#include "upnp_action.h"

namespace EZ
{
    namespace UPNP
    {
        /*
         ** NOTIFY delivery path HTTP/1.0
         ** HOST: delivery host:delivery port
         ** CONTENT-TYPE: text/xml; charset="utf-8"
         ** NT: upnp:event
         ** NTS: upnp:propchange
         ** SID: uuid:subscription-UUID
         ** SEQ: event key
         ** CONTENT-LENGTH: bytes in body
         ** <?xml version="1.0"?>
         ** <e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
         **      <e:property>
         **          <variableName>new value</variableName>
         **      </e:property>
         **      Other variable names and values (if any) go here.
         ** </e:propertyset>
         */
        static const char _gena_event_header[] = "NOTIFY %s HTTP/1.0\r\n"
                                                 "HOST: %s:%d\r\n"
                                                 "NT: upnp:event\r\n"
                                                 "NTS: upnp:propchange\r\n"
                                                 "SID: uuid:%s\r\n"
                                                 "SEQ: %u\r\n"
                                                 "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
                                                 "CONTENT-LENGTH: %d\r\n\r\n%s";

        static const char _gena_event_proph[] = "<?xml version=\"1.0\"?>\r\n"
                                                "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n";

        static const char _gena_event_propf[] = "</e:propertyset>\r\n";

        class SCP : public SERVICE
        {
            friend class DEVICE;
            friend class IOT;

            typedef struct _subscription
            {
                IPAddress ip;
                uint port;
                uint32_t key;
                String url;
                time_t expires;
                UUID uuid;
                // SERVICE* service;

                _subscription() { erase(); }

                void erase(void)
                {
                    ip = (uint32_t)0;
                    port = 80;
                    key = 0;
                    url = "";
                    expires = 0;
                    uuid = "";
                    // service = nullptr;
                }
            } subscription_t;

            typedef struct
            {
                union {
                    event_t event_header;
                    struct
                    {
                        SCP* service;
                        VARIABLE* var;
                    };
                };

                uint32_t addr;
                uint16_t port;
                uint32_t key;
                char uuid[EZ_UUID_LENGTH + 1];
                char url[1]; // Don't change order of these members!
            } gena_event_t;

        public:
            SCP(const char* type, const char* id)
                : SERVICE(SERVICE::MODE::UPNP, id), _upnpVersionMajor(1), _upnpVersionMinor(0), _upnpConfigId(1),
                  _upnpXMLNS(nullptr), _upnpServiceType(type)
            {
                for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                    _subscriptions[s] = nullptr;
            }

            virtual ~SCP()
            {
                for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                {
                    if (_subscriptions[s] != nullptr)
                        delete _subscriptions[s];
                }
            }

            virtual bool ssdpMatch(String& st)
            {
                String _ut(upnpServiceType());
                String _st(st);

                _ut.toLowerCase();
                _st.toLowerCase();

                if (_ut.startsWith(_st))
                {
                    // ESP_LOGV(iotTag, "%s == %s", st.c_str(), upnpServiceType().c_str());
                    return true;
                }

                return false;
            }

            // Should these be from the _homeDevice??
            //
            virtual String upnpVersionMajor(void) { return String(_upnpVersionMajor); }
            virtual String upnpVersionMinor(void) { return String(_upnpVersionMinor); }

            virtual String upnpConfigId(void) { return String(_upnpConfigId); }

            virtual String upnpXMLNS(void)
            {
                if (_upnpXMLNS)
                    return _upnpXMLNS;
                return "urn:EZIoT:device-1-0";
            }

            virtual String upnpXML(void)
            {
                if (_mode == MODE::UPNP)
                {
                    ACTIVITY* activity = _headActivity;
                    String xml("<?xml version=\"1.0\"?>\n");

                    xml.concat("<scpd");
                    xml.concat(" xmlns=\"" + upnpXMLNS() + "\"");

                    if (_upnpVersionMajor >= 2 || (_upnpVersionMajor == 1 && _upnpVersionMinor > 0))
                        xml.concat(" configId=\"" + upnpConfigId() + "\"");

                    xml.concat(">\r\n");

                    xml.concat("<specVersion>\r\n");
                    xml.concat(xmlTag("major", upnpVersionMajor()));
                    xml.concat(xmlTag("minor", upnpVersionMinor()));
                    xml.concat("</specVersion>\r\n");

                    if (activity)
                    {
                        String action("");
                        String vars("");

                        do
                        {
                            if (activity->mode() == ACTIVITY::MODE::ACTION)
                            {
                                action.concat(activity->upnpXML());
                            }
                            else if (activity->mode() == ACTIVITY::MODE::VARIABLE)
                                vars.concat(activity->upnpXML());
                        } while ((activity = activity->nextActivity()));

                        if (action != "")
                            xml.concat(xmlTag("actionList", action));
                        if (vars != "")
                            xml.concat(xmlTag("serviceStateTable", vars));
                    }

                    xml.concat("</scpd>\r\n");

                    return xml;
                }

                return "";
            }

            virtual String upnpServiceId(void)
            {
                if ((_name) && strncasecmp("urn:", _name, 4) != 0)
                {
                    String id("urn:EZIoT:serviceId:");
                    id.concat(_name);
                    return id;
                }

                return _name;
            }

            virtual String upnpServiceType(void)
            {
                if ((_upnpServiceType) && strncasecmp("urn:", _upnpServiceType, 4) != 0)
                {
                    String type("urn:EZIoT:service:");
                    type.concat(_upnpServiceType);

                    return type;
                }

                return _upnpServiceType;
            }

            virtual String urlSchema(bool pathOnly = true) { return _upnpURL(pathOnly, "scpd.xml"); }

            virtual String urlControl(bool pathOnly = true) { return _upnpURL(pathOnly, "control"); }

            virtual String urlEvents(bool pathOnly = true) { return _upnpURL(pathOnly, "event"); }

        protected:
            uint16_t _upnpVersionMajor;
            uint16_t _upnpVersionMinor;
            int _upnpConfigId;
            const char* _upnpXMLNS;
            const char* _upnpServiceType;

            String _upnpURL(bool pathOnly, String suffix)
            {
                String url = pathOnly ? "" : urlBase();
                String uui = uuidDevice();
                uui.replace("-", "");
                uui.toLowerCase();

                return String(url + "/upnp/" + uui + "/" + parseURN(upnpServiceId()) + "/" + suffix);
            }

            /*
            ** Web Handlers
            */
            bool _httpAccept(HTTP::METHOD method, String uri)
            {
                if (_mode == MODE::UPNP)
                {
                    if (method == HTTP::POST && uri == urlControl(true))
                        return true;
                    if (method == HTTP::GET && uri == urlSchema(true))
                        return true;
                    if ((method == HTTP::SUBSCRIBE || method == HTTP::UNSUBSCRIBE) && uri == urlEvents(true))
                        return true;
                }

                return false;
            }

            bool _httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri)
            {
                if (method == HTTP::POST && uri == urlControl(true))
                    return _soapHandle(server);

                if (uri == urlEvents(true))
                {
                    if (method == HTTP::SUBSCRIBE)
                        return _genaSubscribe(server);
                    if (method == HTTP::UNSUBSCRIBE)
                        return _genaUnSubscribe(server);
                }

                if (method == HTTP::GET && uri == urlSchema(true))
                {
                    _sendCommonHeaders(server);
                    return server.send(200, MIME_TYPE_XML, upnpXML());
                }

                return server.send(501); // Not Implemented
            }

            /*
            ** GENA Subscription Handlers
            **
            ** Eventing is on a per service basis, not per state variable
            ** On first subscription, send ALL evented state variables to
            ** subscriber
            */
            bool _genaSubscribe(HTTP::SERVER& server)
            {
                int h, mh = server.headers();
                for (h = 0; h < mh; h++)
                {
                    ESP_LOGV(iotTag, "Header: %s = %s", server.headerName(h).c_str(), server.header(h).c_str());
                }

                String tmp;
                int i, to;
                time_t now;

                // TIMEOUT = Second-5400 (present for new and renew subscriptions)
                //
                // Should be greater than or equal to 1800 seconds (30 minutes).
                //
                time(&now);
                to = EZ_UPNP_SUBSCRIPTION_TIMEOUT;

                if ((i = (tmp = server.header("TIMEOUT")).lastIndexOf("Second-")) >= 0)
                {
                    if ((to = tmp.substring(7).toInt()) < EZ_UPNP_SUBSCRIPTION_TIMEOUT)
                        to = EZ_UPNP_SUBSCRIPTION_TIMEOUT;
                }

                // New subscriptions should have an NT (upnp:event) header
                //
                if (server.header("NT") == "upnp:event")
                {
                    if (server.header("SID") != "")
                        return server.send(400);

                    _subscription* sub = nullptr;

                    // Any free slots?
                    for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                    {
                        if (_subscriptions[s])
                        {
                            if (_subscriptions[s]->expires < now)
                            {
                                sub = _subscriptions[s];
                                sub->erase();
                                break;
                            }
                            else
                            {
                                // Quick and dirty callback check!
                                //
                                // Smartthings (and others may) send new subscription requests
                                // without checking if they already have an active one, so we need
                                // to check if the new request is really a new one!
                                //
                                String cbURL("<http://");
                                cbURL += _subscriptions[s]->ip.toString();
                                cbURL += ":";
                                cbURL += _subscriptions[s]->port;
                                cbURL += _subscriptions[s]->url;
                                cbURL += ">";

                                ESP_LOGV(iotTag, "Callback: %s = %s", server.header("CALLBACK").c_str(), cbURL.c_str());

                                if (server.header("CALLBACK") == cbURL)
                                {
                                    _subscriptions[s]->expires = now + to;
                                    return _genaSuccess(server, _subscriptions[s], to, false);
                                }
                            }
                        }
                        else
                        {
                            _subscriptions[s] = sub = new _subscription;
                            break;
                        }
                    }

                    if (!sub)
                        return server.send(500);

                    // Subscription expiry time
                    //
                    sub->expires = now + to;

                    // CALLBACK = <http://xxx.xxx.xxx.xxx:pppp/>
                    //
                    if ((tmp = server.header("CALLBACK")).startsWith("<http://"))
                    {
                        sub->port = 80;
                        if ((i = tmp.lastIndexOf(":")) > 0)
                        {
                            if ((sub->port = tmp.substring(i + 1).toInt()) == 0)
                                sub->port = 80;
                        }
                        else
                            i = tmp.indexOf("/", 8);

                        if ((sub->ip.fromString(tmp.substring(8, i))) != 0)
                        {
                            if ((i = tmp.indexOf(">")) > 0)
                            {
                                if ((sub->url = tmp.substring(tmp.indexOf("/", 8), i)) == "")
                                    sub->url = "/";

                                // If we are not reusing a subscription
                                sub->uuid.makeV4();

                                return _genaSuccess(server, sub, to, false);
                            }
                        }
                    }
                }
                else if (server.header("SID") != "")
                {
                    UUID uuid(server.header("SID"));

                    if (server.header("NT") != "" || server.header("CALLBACK") != "")
                        return server.send(400);

                    for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                    {
                        if ((_subscriptions[s]) && _subscriptions[s]->uuid == uuid)
                        {
                            _subscriptions[s]->expires = now + to;
                            return _genaSuccess(server, _subscriptions[s], to, true);
                        }
                    }
                }

                return server.send(412); // Precondition Failed (Missing/Invalid header)
            }

            bool _genaUnSubscribe(HTTP::SERVER& server)
            {
                /**
                int h, mh = server.headers();
                for (h = 0; h < mh; h++)
                {
                    ESP_LOGV(iotTag, "Header: %s = %s", server.headerName(h).c_str(), server.header(h).c_str());
                }
                **/

                // If SID header field and one of NT or CALLBACK header fields are present, error 400
                //
                if (server.header("SID") != "")
                {
                    UUID uuid(server.header("SID"));

                    if (server.header("NT") != "" || server.header("CALLBACK") != "")
                        return server.send(400);

                    for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                    {
                        if ((_subscriptions[s]) && _subscriptions[s]->uuid == uuid)
                        {
                            _subscriptions[s]->erase();
                            return server.send(200);
                        }
                    }
                }

                return server.send(412);
            }

            /*
            ** HTTP/1.1 200 OK
            ** DATE: when response was generated
            ** SERVER: OS/version UPnP/2.0 product/version
            ** SID: uuid:subscription-UUID
            ** CONTENT-LENGTH: 0
            ** TIMEOUT: Second-actual subscription duration
            ** ACCEPTED-STATEVAR: CSV of state variables
            */
            bool _genaSuccess(HTTP::SERVER& server, subscription_t* sub, int to, bool renewal)
            {
                String timeout(to);
                struct tm tm;
                localtime_r(&sub->expires, &tm);

                ESP_LOGV(iotTag, "Subscription: %s (%s:%d) %s, expires: %s", sub->uuid.toString().c_str(),
                         sub->ip.toString().c_str(), sub->port, renewal ? "Renewed" : "Created",
                         dateRFC1123(&tm).c_str());

                _sendCommonHeaders(server);
                server.sendHeader("TIMEOUT", "Second-" + timeout);
                server.sendHeader("SID", sub->uuid.toString(true));

                // TODO: ACCEPTED-STATEVAR
                //

                (void)server.send(200, MIME_TYPE_TEXT);

                if (!renewal)
                    (void)_genaEvent(sub, NULL);

                return true;
            }

            void _genaEvent(subscription_t* sub, VARIABLE* var)
            {
                if (sub)
                {
                    size_t s = (sub) ? sub->url.length() : 0;
                    gena_event_t* e = (gena_event_t*)malloc(sizeof(gena_event_t) + s);

                    if (e)
                    {
                        e->service = this;
                        e->var = var;
                        e->addr = sub->ip;
                        e->port = sub->port;
                        e->key = sub->key++;
                        strncpy(e->uuid, sub->uuid.toString().c_str(), EZ_UUID_LENGTH);
                        e->uuid[EZ_UUID_LENGTH] = '\0';
                        strncpy(e->url, sub->url.c_str(), s);
                        e->url[s] = '\0';

                        _eventQueue(reinterpret_cast<event_t*>(e));
                    }
                    else
                    {
                        ESP_LOGE(iotTag, "Event: No Memory.");
                    }
                }
            }

            void registerEvent(ACTIVITY* activity)
            {
                time_t now;

                if ((!activity) || activity->mode() != ACTIVITY::MODE::VARIABLE)
                    return;

                for (int s = 0; s < EZ_UPNP_MAX_SUBSCRIPTIONS; s++)
                {
                    time(&now);

                    if ((_subscriptions[s]) && _subscriptions[s]->expires > now)
                    {
                        ESP_LOGV(iotTag, "Event (%d): %s (%d:%d)", s, _subscriptions[s]->uuid.toString().c_str(),
                                 _subscriptions[s]->expires, now);

                        _genaEvent(_subscriptions[s], reinterpret_cast<VARIABLE*>(activity));
                    }
                }
            }

            void processEvent(event_t* event)
            {
                gena_event_t* e = reinterpret_cast<gena_event_t*>(event);
                WiFiClient sub;

                if (!e)
                    return;

                if (sub.connect(e->addr, e->port))
                {
                    ESP_LOGV(iotTag, "Event: Connected %s:%d", sub.remoteIP().toString().c_str(), sub.remotePort());

                    ACTIVITY* activity = e->service->headActivity();

                    if (activity)
                    {
                        String props = _gena_event_proph;

                        do
                        {
                            if (activity->mode() == ACTIVITY::MODE::VARIABLE)
                            {
                                VARIABLE* var = static_cast<VARIABLE*>(activity);

                                if (((!e->var) || e->var == var) && var->upnpEventable())
                                {
                                    props += "<e:property>\r\n";
                                    props += var->upnpXML(true, false);
                                    props += "</e:property>\r\n";
                                }
                            }
                        } while ((activity = activity->nextActivity()));

                        props += _gena_event_propf;

                        sub.printf(_gena_event_header, e->url, sub.remoteIP().toString().c_str(), sub.remotePort(),
                                   e->uuid, e->key, props.length(), props.c_str());

                        sub.flush();

                        // Only for testing/debugging - need to get Required
                        if (1)
                        {
                            char buf[1000];

                            snprintf(buf, sizeof(buf), _gena_event_header, e->url, sub.remoteIP().toString().c_str(),
                                     sub.remotePort(), e->uuid, e->key, props.length(), props.c_str());

                            ESP_LOGV(iotTag, "\n%s", buf);
                        }
                    }

                    ESP_LOGV(iotTag, "Client Done");
                    sub.stop();
                }
            }

            /*
            ** SOAP Handlers
            */
            bool _soapHandle(HTTP::SERVER& server)
            {
                String sa, urn, action;

                ESP_LOGV(iotTag, "SOAP : %s", server.arg("plain").c_str());

                if (!server.header("Content-Type").startsWith(MIME_TYPE_XML))
                    return server.send(415); // Unsupported Media Type

                if ((sa = server.header("SOAPAction")) == "")
                    return server.send(400); // Bad Request

                sa.replace("\"", "");
                urn = sa.substring(0, sa.indexOf('#'));
                action = sa.substring(sa.indexOf('#') + 1);

                ESP_LOGV(iotTag, "Action : %s (%s)", action.c_str(), urn.c_str());

                if (upnpServiceType() == urn)
                {
                    ACTIVITY* activity = _headActivity;

                    if (activity)
                    {
                        do
                        {
                            if ((activity->mode() == ACTIVITY::MODE::ACTION) && activity->name() == action)
                                return _soapAction(server, static_cast<ACTION*>(activity));
                        } while ((activity = activity->nextActivity()));
                    }

                    ESP_LOGE(iotTag, "Cannot find matching activity");

                    return _soapFault(server, EZ_SOAP_ERROR_INVALID_ACTION, action);
                }

                ESP_LOGE(iotTag, "Cannot find matching service URN");

                return _soapFault(server, EZ_SOAP_ERROR_INVALID_ACTION, urn);
            }

            /*
            **      <u:actionName xmlns:u="urn:schemas-upnp-org:service:serviceType:v">
            **          <argumentName>in arg value</argumentName>
            **          <!-- other in args and their values go here, if any -->
            **      </u:actionName>
            */
            bool _soapAction(HTTP::SERVER& server, ACTION* action)
            {
                // Build argument (in and out) list and populate 'in' values from soap request
                //
                ACTION::arg_pairs_t argList[EZ_UPNP_MAX_ARGS] = {0};
                ACTION::action_arg_t* arg;
                int length = server.arg("plain").length();
                int index = 0, ti, si;
                int argc = 0;

                ti = server.arg("plain").indexOf(action->name());
                si = server.arg("plain").indexOf('>', ti);

                // Make sure the incoming envelope contains a service type match
                if (((index = server.arg("plain").indexOf(upnpServiceType().c_str())) < 0) || ti < 0 || si < 0)
                    return _soapFault(server, EZ_SOAP_ERROR_INVALID_ACTION, upnpServiceType());

                // Any argument tags passed?
                if (server.arg("plain")[si - 1] != '/')
                {
                    // Move to first argument (if any) entry
                    index = server.arg("plain").indexOf('>', index);
                    index = server.arg("plain").indexOf('<', index);

                    do
                    {
                        String tmp, val;

                        // Get opening tag name
                        index = server.arg("plain").indexOf('<', index) + 1;
                        ti = server.arg("plain").indexOf('>', index);
                        tmp = server.arg("plain").substring(index, ti);

                        // Remove any attributes
                        if ((si = tmp.indexOf(' ')) > 0)
                            tmp = tmp.substring(0, si);

                        // Have we found the end of the argument list?
                        if (tmp.startsWith("/"))
                        {
                            ESP_LOGE(iotTag, "End Tag? %s", tmp.c_str());

                            index = server.arg("plain").indexOf(':', index) + 1;
                            if (!server.arg("plain").substring(index).startsWith(action->name()))
                                return _soapFault(server, EZ_SOAP_ERROR_INVALID_ARGS, tmp);
                            break;
                        }

                        // Make sure we have a matching closing tag
                        index = ti + 1;
                        ti = server.arg("plain").indexOf("</", index);

                        if ((ti < 0) || !server.arg("plain").substring(ti + 2).startsWith(tmp))
                        {
                            return _soapFault(server, EZ_SOAP_ERROR_INVALID_ARGS, tmp);
                        }

                        // Extract raw value
                        val = server.arg("plain").substring(index, ti);

                        // Move to end of tag, ready for next check
                        index = server.arg("plain").indexOf(">", ti) + 1;

                        // See if we are expecting this argument, in this position/order
                        arg = action->upnpArgument(argc);

                        if ((arg) && arg->argName != nullptr && arg->relState != nullptr)
                        {
                            if (!arg->dirOut)
                            {
                                if (tmp != arg->argName)
                                    return _soapFault(server, EZ_SOAP_ERROR_INVALID_ARGS, tmp);

                                // Decode Value?

                                // Check the value provided is acceptable (and within ranges)
                                int error = arg->relState->validate(val);
                                if (error != EZ_SOAP_ERROR_NONE)
                                    return _soapFault(server, error, tmp);

                                // All good, so store ready for action!
                                argList[argc].value = val;
                                argList[argc].arg = arg;
                                argc++;
                            }
                            else
                                break;
                        }
                        else
                            break;
                    } while (index < length && argc < EZ_UPNP_MAX_ARGS);
                }

                // Make sure we collected all the 'in' argument values
                for (; argc < EZ_UPNP_MAX_ARGS; argc++)
                {
                    arg = action->upnpArgument(argc);
                    if ((arg) && arg->argName != nullptr && arg->relState != nullptr && !arg->dirOut)
                        return _soapFault(server, EZ_SOAP_ERROR_INVALID_ARGS, "Not enough 'in' arguments");

                    if (arg->dirOut)
                    {
                        argList[argc].value = arg->relState->value();
                        argList[argc].arg = arg;
                    }
                }

                // Call the Action
                int error = action->upnpCall(argList);
                if (error != EZ_SOAP_ERROR_NONE)
                    return _soapFault(server, error, "Error in Action");

                // Prepare response/result of the action and send it on its way
                String response("");
                response += "<u:" + action->name() + "Response xmlns:u=\"" + upnpServiceType() + "\">\r\n";

                for (argc = 0; argc < EZ_UPNP_MAX_ARGS; argc++)
                {
                    if ((arg = argList[argc].arg))
                    {
                        if (arg->dirOut || arg->retVal)
                        {
                            // Encode Value?
                            response += xmlTag(arg->argName, argList[argc].value, true);
                        }
                    }
                }

                response += "</u:" + action->name() + "Response>\r\n";
                return _soapEnvelope(server, 200, response);
            }

            /*
            ** <s:Fault>
            **  <faultcode>s:Client</faultcode>
            **  <faultstring>UPnPError</faultstring>
            **  <detail>
            **      <UPnPError xmlns="urn:schemas-upnp-org:control-1-0">
            **          <errorCode>error code</errorCode>
            **          <errorDescription>error string</errorDescription>
            **      </UPnPError>
            **  </detail>
            ** </s:Fault>
            */
            bool _soapFault(HTTP::SERVER& server, int code, String text)
            {
                String fault("");
                String error;

                switch (code)
                {
                    case EZ_SOAP_ERROR_INVALID_ACTION:
                        error = F("Invalid Action");
                        break;
                    case EZ_SOAP_ERROR_INVALID_ARGS:
                        error = F("Invalid Args");
                        break;
                    case EZ_SOAP_ERROR_ACTION_FAILED:
                        error = F("Action Failed");
                        break;
                    case EZ_SOAP_ERROR_INVALID_VALUE:
                        error = F("Argument Value Invalid");
                        break;
                    case EZ_SOAP_ERROR_OUT_OF_RANGE:
                        error = F("Argument Value Out of Range");
                        break;
                    case EZ_SOAP_ERROR_NOT_IMPLEMENTED:
                        error = F("Optional Action Not Implemented");
                        break;
                    case EZ_SOAP_ERROR_OUT_OF_MEMORY:
                        error = F("Out of Memory");
                        break;
                    case EZ_SOAP_ERROR_INTERVENTION:
                        error = F("Human Intervention Required");
                        break;
                    case EZ_SOAP_ERROR_STRING_LENGTH:
                        error = F("String Argument Too Long");
                        break;
                    default:
                        error = F("Unknown Error:");
                        error += String(code);
                        break;
                }

                if (text != "")
                    error += " (" + text + ")";
                fault += "<s:Fault>\r\n";
                fault += "<faultcode>s:Client</faultcode>\r\n";
                fault += "<faultstring>UPnPError</faultstring>\r\n";
                fault += "<detail>\r\n";
                fault += "<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">\r\n";
                fault += xmlTag("errorCode", String(code), true);
                fault += xmlTag("errorDescription", error, true);
                fault += "</UPnPError>\r\n";
                fault += "</detail>\r\n";
                fault += "</s:Fault>\r\n";

                return _soapEnvelope(server, 500, fault);
            }

            /*
            ** <s:Envelope
            ** xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
            ** s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
            **   <s:Body>
            **      <!-- Body Goes Here -->
            **   </s:Body>
            ** </s:Envelope>
            */
            bool _soapEnvelope(HTTP::SERVER& server, int httpCode, String content)
            {
                String envelope("");
                envelope += "<?xml version=\"1.0\"?>\r\n";
                envelope += "<s:Envelope ";
                envelope += "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" ";
                envelope += "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n";
                envelope += "<s:Body>\r\n";
                envelope += content;
                envelope += "</s:Body>\r\n";
                envelope += "</s:Envelope>\r\n";
                envelope += "\r\n";

                _sendCommonHeaders(server);
                String ct("text/xml; charset=\"utf-8\"");
                return server.send(httpCode, ct, envelope);

                // return server.send(httpCode, MIME_TYPE_XML, envelope);
            }

        private:
            subscription_t* _subscriptions[EZ_UPNP_MAX_SUBSCRIPTIONS];
        };
    } // namespace UPNP
} // namespace EZ
#endif // _UPNP_SERVICE_H

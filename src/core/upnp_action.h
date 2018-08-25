/*
** EZIoT - Action Base Class
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
#if !defined(_UPNP_ACTION_H)
#define _UPNP_ACTION_H
#include "ez_activity.h"
#include "ez_common.h"
#include "ez_variable.h"
#include "upnp_scp.h"

namespace EZ
{
    namespace UPNP
    {
        class ACTION : public ACTIVITY
        {
            typedef struct
            {
                VARIABLE* relState;
                const char* argName;
                bool dirOut;
                bool retVal;
            } action_arg_t;

            typedef struct
            {
                action_arg_t* arg;
                String value;
            } arg_pairs_t;

        public:
            friend class SERVICE;
            friend class SCP;

            virtual ~ACTION() {}
            ACTION(const char* actName) : ACTION(actName, nullptr) {}
            ACTION(const char* actName, VARIABLE* relState, const char* argName = nullptr, bool out = false,
                   bool ret = false)
                : ACTIVITY(actName, MODE::ACTION)
            {
                memset(&_args, 0, sizeof(_args));
                _retval = ret;

                if (relState)
                {
                    _args[0].relState = relState;
                    _args[0].argName = argName;
                    _args[0].dirOut = out;
                    _args[0].retVal = ret;
                }
            }

            String upnpXML(bool valueTag = false, bool emptyTag = false)
            {
                String xml("<action>\r\n");

                xml.concat(xmlTag("name", name(), true));

                bool closeTag = false;

                for (int a = 0; a < EZ_UPNP_MAX_ARGS; a++)
                {
                    if (_args[a].relState != nullptr)
                    {
                        if (!closeTag)
                        {
                            xml.concat("<argumentList>\r\n");
                            closeTag = true;
                        }

                        xml.concat("<argument>\r\n");

                        String an = _args[a].argName;

                        if (an == "")
                        {
                            an = (!_args[a].dirOut ? "new" : "ret");
                            an.concat(_args[a].relState->name());
                        }

                        xml.concat(xmlTag("name", an, true));
                        xml.concat(xmlTag("direction", (_args[a].dirOut ? "out" : "in"), true));
                        xml.concat(xmlTag("relatedStateVariable", _args[a].relState->name(), true));

                        if (a == 0 && _retval)
                            xml.concat("<retVal/>");

                        xml.concat("</argument>\r\n");
                    }
                }

                if (closeTag)
                    xml.concat("</argumentList>\r\n");
                xml.concat("</action>\r\n");

                return xml;
            }

            bool addArgument(VARIABLE* relState, const char* argName, bool out = false, bool ret = false)
            {
                for (int argc = 0; argc < EZ_UPNP_MAX_ARGS; argc++)
                {
                    if (!_args[argc].relState)
                    {
                        _args[argc].relState = relState;
                        _args[argc].argName = argName;
                        _args[argc].dirOut = out;
                        _args[argc].retVal = argc == 0 ? ret : false;

                        return true;
                    }
                    else if (_args[argc].relState == relState)
                        break;
                }
                return false;
            }

            action_arg_t* upnpArgument(int index)
            {
                if (index >= 0 && index < EZ_UPNP_MAX_ARGS)
                    return &_args[index];
                return nullptr;
            }

            int upnpCall(arg_pairs_t* args)
            {
                (void)_postCallback(SERVICE::CALLBACK::PRE_ACTION);

                for (int argc = 0; argc < EZ_UPNP_MAX_ARGS; argc++)
                {
                    if (args[argc].arg)
                    {
                        if (args[argc].arg->relState)
                        {
                            if (!args[argc].arg->dirOut)
                                args[argc].arg->relState->value(args[argc].value);
                            if (args[argc].arg->dirOut || args[argc].arg->retVal)
                                args[argc].value = args[argc].arg->relState->value();
                        }
                    }
                }

                (void)_postCallback(SERVICE::CALLBACK::POST_ACTION);

                return EZ_SOAP_ERROR_NONE;
            }

        private:
            bool _retval;
            action_arg_t _args[EZ_UPNP_MAX_ARGS];
        };
    } // namespace UPNP
} // namespace EZ
#endif // _UPNP_ACTION_H

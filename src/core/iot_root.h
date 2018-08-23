/*
** EZIoT - IOT Root Device Class
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
#if !defined(_IOT_ROOT_H)
#define _IOT_ROOT_H
#include "ez_device.h"
#include "ez_http.h"

namespace EZ
{
    class ROOT : public DEVICE, HTTP::SERVER
    {
        friend class IOT;

    public:
        String upnpUUID(void);
        uint16_t httpPort(void);

    protected:
        ROOT();
        bool _httpAccept(HTTP::METHOD method, String uri);
        bool _httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri);
        bool _httpPresentation(HTTP::SERVER& server, HTTP::METHOD method, String uri);

    private:
        ROOT(ROOT const& copy);            // Not Implemented
        ROOT& operator=(ROOT const& copy); // Not Implemented
    };
} // namespace EZ
#endif // _IOT_ROOT_H
/******************************************************************************/

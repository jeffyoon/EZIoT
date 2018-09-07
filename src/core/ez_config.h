/*
** EZIoT - Configuration Service Class
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
#if !defined(_EZ_CONFIG_H)
#define _EZ_CONFIG_H
#include "ez_common.h"
#include "ez_service.h"

namespace EZ
{
    class CONFIG : public SERVICE
    {
    public:
        ~CONFIG() {};
        CONFIG() : SERVICE(SERVICE::MODE::CONFIG, "_config_") {}

    private:
        void _initialise(void) {}
        bool _httpAccept(HTTP::METHOD method, String uri) { return false; }
        bool _httpHandle(HTTP::SERVER& server, HTTP::METHOD method, String uri) { return false; }
    };
} // namespace EZ
#endif // _EZ_CONFIG_H

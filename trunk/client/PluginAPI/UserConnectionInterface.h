#ifndef RSXPLUSPLUS_USER_CONNECTION_INTERFACE
#define RSXPLUSPLUS_USER_CONNECTION_INTERFACE

namespace dcpp {

class iUserConnection {
public:
	virtual void __cdecl p_disconnect(bool graceless = false) = 0;
	virtual rString __cdecl p_getRemoteIp() const = 0;
	virtual void __cdecl p_send(const rString& data) = 0;
	virtual bool __cdecl p_isAdc() const = 0;

};
} // namespace dcpp
#endif // RSXPLUSPLUS_USER_CONNECTION_INTERFACE